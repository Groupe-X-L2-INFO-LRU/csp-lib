#include "heuristics.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "csp.inc"

/**
 * @file heuristics.c
 * @brief Implementation of heuristics for improving CSP solver performance.
 *
 * This file implements various heuristics used in constraint satisfaction problem solving:
 *
 * 1. Minimum Remaining Values (MRV): For variable selection
 * 2. Least Constraining Value (LCV): For value ordering
 * 3. Domain pruning mechanisms for forward checking
 * 4. Restoration of pruned domains during backtracking
 *
 * These heuristics significantly improve the efficiency of the solving process
 * by making intelligent choices that reduce the search space.
 *
 * @author Ch. Demko (original)
 * @date 2024-2025
 * @version 1.0
 */

/**
 * @brief Implements the Minimum Remaining Values (MRV) heuristic for variable selection.
 *
 * This function examines all unassigned variables and selects the one with
 * the fewest remaining legal values in its domain. This heuristic aims to
 * identify the most constrained variable, which is most likely to cause a failure
 * if left for later (the "fail-first" principle).
 *
 * If multiple variables have the same minimum number of remaining values,
 * the first one encountered is returned.
 *
 * @param ctx The forward checking context containing domain information.
 * @return The index of the unassigned variable with the fewest remaining legal values.
 */
size_t select_unassigned_var(const CSPForwardCheckContext *ctx) {
    // Initialize with maximum values to ensure any variable will be better
    size_t best = SIZE_MAX;        // Tracks the best variable found so far
    size_t best_count = SIZE_MAX;  // Tracks the domain size of the best variable

    // Iterate through all variables in the CSP
    for (size_t i = 0; i < ctx->num_domains; ++i) {
        // Skip variables that have already been assigned
        if (ctx->assigned[i]) continue;

        // Count how many values are still available in this variable's domain
        size_t count = 0;
        for (size_t v = 0; v < ctx->original_domain_sizes[i]; ++v) {
            // If this value is still valid in the domain, increment the counter
            if (ctx->current_domains[i][v]) ++count;
        }

        // If this variable has fewer remaining values than the current best,
        // update our choice (this is the core of the MRV heuristic)
        if (count < best_count) {
            best_count = count;  // Update the smallest domain size found
            best = i;            // Remember this variable as our current best choice
        }
    }

    // Return the variable with the fewest remaining values (most constrained)
    return best;
}

/**
 * @brief Implements the Least Constraining Value (LCV) heuristic for value ordering.
 *
 * This function orders the values in a variable's domain by how constraining they are
 * on the domains of neighboring variables (those connected by constraints).
 *
 * For each possible value in the domain:
 * 1. It counts how many values would become invalid in neighboring variables if this value is
 * chosen
 * 2. It sorts the values from least constraining (fewest conflicts) to most constraining
 *
 * This heuristic increases the likelihood of finding a valid assignment without backtracking
 * by trying the values that leave the most flexibility for future assignments first.
 *
 * The implementation:
 * - Only considers binary constraints (between exactly 2 variables)
 * - Uses a simple insertion sort to order the values
 * - Temporarily modifies the values array for checking (but restores it)
 *
 * @param csp The CSP problem.
 * @param ctx The forward checking context.
 * @param values The current assignment of values to variables.
 * @param data User-provided data for constraint checking.
 * @param var The variable for which to order values.
 * @param ordered Output array where ordered values will be stored.
 * @param n_vals Output parameter that will be set to the number of valid values.
 */
void order_values_lcv(const CSPProblem *csp, CSPForwardCheckContext *ctx, const size_t *values,
                      const void *data, size_t var, size_t *ordered, size_t *n_vals) {
    // Get the domain size for the variable we're considering
    size_t dsize = ctx->original_domain_sizes[var];

    // Define a structure to track each value and its conflict count
    typedef struct {
        size_t val;        // The actual value from the domain
        size_t conflicts;  // How many conflicts this value causes in other variables
    } pair_t;

    // Allocate array to store all potential values and their conflict counts
    pair_t *pairs = malloc(dsize * sizeof(pair_t));

    // Counter for the number of valid values we find
    size_t n = 0;

    // Get the total number of constraints in the CSP
    size_t num_constraints = csp_problem_get_num_constraints(csp);

    // Cast away const for temporary assignments (we'll modify values temporarily)
    size_t *vals = (size_t *)values;

    // For each possible value in this variable's domain
    for (size_t val = 0; val < dsize; ++val) {
        // Skip values that have been pruned from the domain
        if (!ctx->current_domains[var][val]) continue;

        // Store this value in our pairs array
        pairs[n].val = val;

        // Initialize conflict counter for this value
        size_t conflicts = 0;

        // Temporarily assign this value to the variable
        vals[var] = val;
        // For each constraint in the CSP
        for (size_t ci = 0; ci < num_constraints; ++ci) {
            // Get the constraint
            CSPConstraint *con = csp_problem_get_constraint(csp, ci);

            // We only consider binary constraints (between 2 variables)
            if (csp_constraint_get_arity(con) != 2) continue;

            // Get the two variables involved in this constraint
            size_t a = csp_constraint_get_variable(con, 0);
            size_t b = csp_constraint_get_variable(con, 1);

            // Determine which variable in the constraint is the "other" variable
            // (not the one we're currently assigning)
            size_t other = SIZE_MAX;  // Initialize to an impossible value
            if (a == var)
                other = b;
            else if (b == var)
                other = a;

            // Skip if this constraint doesn't involve our variable or if the other
            // variable is already assigned (we only care about future conflicts)
            if (other == SIZE_MAX || ctx->assigned[other]) continue;

            // For each value in the domain of the other variable
            for (size_t o_val = 0; o_val < ctx->original_domain_sizes[other]; ++o_val) {
                // Skip values that have been pruned from the other variable's domain
                if (!ctx->current_domains[other][o_val]) continue;

                // Temporarily assign this value to the other variable
                vals[other] = o_val;

                // Check if this assignment violates the constraint
                // If it does, increment the conflict counter
                if (!con->check(con, vals, data)) {
                    ++conflicts;  // This value for our var conflicts with a value in other var
                }
            }
        }
        // Store the total conflict count for this value
        pairs[n].conflicts = conflicts;

        // Move to the next value
        ++n;
    }

    // Sort values by conflict count using insertion sort
    // (values with fewer conflicts should come first - these are less constraining)
    for (size_t i = 1; i < n; ++i) {
        // Store the current element we need to insert in the right position
        pair_t key = pairs[i];
        size_t j = i;

        // Move elements that have more conflicts than key one position ahead
        while (j > 0 && pairs[j - 1].conflicts > key.conflicts) {
            pairs[j] = pairs[j - 1];
            --j;
        }

        // Place the key element in its correct position
        pairs[j] = key;
    }

    // Copy the ordered values to the output array
    for (size_t i = 0; i < n; ++i) ordered[i] = pairs[i].val;

    // Set the number of valid values found
    *n_vals = n;

    // Free the pairs array we allocated
    free(pairs);
}

/**
 * @brief Prunes inconsistent values from neighboring variables during forward checking.
 *
 * This function is the core of the forward checking algorithm. After a variable is assigned a
 * value, this function examines all neighboring variables (those connected by constraints) and
 * removes any values from their domains that would violate a constraint, given the current
 * assignment.
 *
 * For each binary constraint involving the assigned variable:
 * 1. It identifies the other variable in the constraint
 * 2. It checks each value in that variable's domain
 * 3. If a value would violate the constraint with the current assignment, it's marked as
 * unavailable
 * 4. The pruned value is recorded so it can be restored if we need to backtrack
 *
 * The pruned values are tracked in two data structures:
 * - counts[v] records how many values were pruned from variable v
 * - pruned[v] stores which values were pruned from variable v
 *
 * @param csp The CSP problem.
 * @param values The current assignment of values to variables.
 * @param data User-provided data for constraint checking.
 * @param ctx The forward checking context to update.
 * @param var The variable that was just assigned.
 * @param counts Array to record the number of pruned values for each variable.
 * @param pruned 2D array to record which values were pruned from each variable.
 */
void prune_neighbors(const CSPProblem *csp, const size_t *values, const void *data,
                     CSPForwardCheckContext *ctx, size_t var, size_t *counts, size_t **pruned) {
    // Get the total number of constraints in the CSP
    size_t num_constraints = csp_problem_get_num_constraints(csp);

    // Cast away const for temporary assignments
    size_t *vals = (size_t *)values;

    // For each constraint in the CSP
    for (size_t ci = 0; ci < num_constraints; ++ci) {
        // Get the constraint
        CSPConstraint *con = csp_problem_get_constraint(csp, ci);

        // We only consider binary constraints (between 2 variables)
        if (csp_constraint_get_arity(con) != 2) continue;

        // Get the two variables involved in this constraint
        size_t a = csp_constraint_get_variable(con, 0);
        size_t b = csp_constraint_get_variable(con, 1);

        // Find the other variable in this constraint (not the one we just assigned)
        // Using a conditional expression for conciseness
        size_t other = (a == var ? b : (b == var ? a : SIZE_MAX));

        // Skip if this constraint doesn't involve our variable or if the other
        // variable is already assigned (no need to prune already assigned vars)
        if (other == SIZE_MAX || ctx->assigned[other]) continue;

        // Allocate memory to track which values will be pruned from this variable
        // (only if we haven't already allocated it)
        if (!pruned[other]) {
            pruned[other] = malloc(ctx->original_domain_sizes[other] * sizeof(size_t));
        }
        // The variable 'var' already has its current value in vals[var]

        // For each value in the domain of the other variable
        for (size_t o_val = 0; o_val < ctx->original_domain_sizes[other]; ++o_val) {
            // Skip values that have already been pruned
            if (!ctx->current_domains[other][o_val]) continue;

            // Temporarily assign this value to the other variable
            vals[other] = o_val;

            // Check if this combination violates the constraint
            CSPChecker *check_func = csp_constraint_get_check(con);
            if (!check_func(con, vals, data)) {
                // This value is incompatible with our current assignment,
                // so prune it from the domain
                ctx->current_domains[other][o_val] = false;

                // Record this pruned value so we can restore it later if needed
                pruned[other][counts[other]++] = o_val;
            }
        }
    }
}

/**
 * @brief Restores values that were pruned during forward checking when backtracking occurs.
 *
 * When the solving algorithm needs to backtrack (because it reached a dead end),
 * this function restores all the domain values that were pruned as a result of
 * the assignment that's being undone. This ensures that the search can continue
 * with different value assignments.
 *
 * For each variable:
 * 1. It examines the list of values that were pruned from that variable
 * 2. It marks each of these values as available again in the current domains
 * 3. It frees the memory used to track pruned values
 *
 * This function is essential for the completeness of the backtracking algorithm,
 * ensuring that no potential solutions are missed.
 *
 * @param ctx The forward checking context to update.
 * @param counts Array indicating how many values were pruned for each variable.
 * @param pruned 2D array indicating which values were pruned from each variable.
 */
void restore_pruned(CSPForwardCheckContext *ctx, const size_t *counts, size_t **pruned) {
    // Iterate through all variables in the CSP
    for (size_t i = 0; i < ctx->num_domains; ++i) {
        // For each value that was pruned from this variable
        for (size_t j = 0; j < counts[i]; ++j) {
            // Restore the value to the domain (mark it as available again)
            ctx->current_domains[i][pruned[i][j]] = true;
        }

        // Free the memory used to track pruned values for this variable
        // (only if it was allocated - the check is implicit since free(NULL) is safe)
        free(pruned[i]);
    }
}
