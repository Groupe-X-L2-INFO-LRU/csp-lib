/**
 * @file forward-checking
 * @brief Forward checking algorithm with MRV and LCV heuristics implementation.
 *
    * This module implements a CSP solver that uses forward checking to prune domains
 *
 * @author Quentin Sautière
 * @date 2025
 * @version 1.0
 */

#include "forward-checking.h"

#include <assert.h>
#include <signal.h>  // For sig_atomic_t
#include <stdlib.h>
#include <string.h>

#include "csp_internal.h"
#include "heuristics.h"

// Declare the global timeout flag that can be set externally
volatile sig_atomic_t timeout_occurred = 0;

/**
 * @brief Checks consistency of the current assignment under forward checking.
 *
 * Only constraints whose all variables have already been assigned (according to ctx->assigned)
 * are tested. Unassigned variables are ignored, avoiding false conflicts on default values.
 */
static bool fc_is_consistent(const CSPProblem *csp, const size_t *values, const void *data,
                             const CSPForwardCheckContext *ctx) {
    size_t ncons = csp_problem_get_num_constraints(csp);
    for (size_t ci = 0; ci < ncons; ++ci) {
        CSPConstraint *con = csp_problem_get_constraint(csp, ci);
        size_t ar = csp_constraint_get_arity(con);
        bool ready = true;
        // Check if all vars in this constraint have been assigned
        for (size_t i = 0; i < ar; ++i) {
            size_t var = csp_constraint_get_variable(con, i);
            if (!ctx->assigned[var]) {
                ready = false;
                break;
            }
        }
        if (!ready) continue;
        // All variables assigned -> test constraint
        CSPChecker *check_func = csp_constraint_get_check(con);
        if (!check_func(con, values, data)) return false;
    }
    return true;
}

/**
 * @file forward-checking.c
 * @brief Forward checking algorithm with MRV and LCV heuristics implementation.
 *
 * This module implements a CSP solver that uses forward checking to prune domains
 * as soon as a variable is assigned, combined with two heuristics:
 * - Minimum Remaining Values (MRV) for variable ordering
 * - Least Constraining Value (LCV) for value ordering
 *
 * Forward checking reduces search space by removing inconsistent values early,
 * and the heuristics guide the search toward a solution efficiently.
 */

/**
 * @brief Recursive backtracking function with forward checking.
 *
 * This function implements the core of the forward checking algorithm:
 * 1. Check if all variables are assigned - if so, return success
 * 2. Select the next unassigned variable using the MRV heuristic
 * 3. Order the values for this variable using the LCV heuristic
 * 4. Try each value in order:
 *    a. Assign the value to the variable
 *    b. Check if the assignment is consistent
 *    c. Prune inconsistent values from future variables
 *    d. Recursively try to assign the remaining variables
 *    e. If successful, return the solution
 *    f. If unsuccessful, restore pruned values and try the next value
 *
 * @param csp The CSP problem to solve.
 * @param values Array to store the assignments of values to variables.
 * @param data User-provided data for constraint checking.
 * @param ctx The forward checking context.
 * @return true if a solution is found, false otherwise.
 */
// External flag that can be set by a signal handler to indicate timeout
extern volatile sig_atomic_t timeout_occurred;

static bool backtrack_fc(const CSPProblem *csp, size_t *values, const void *data,
                         CSPForwardCheckContext *ctx) {
    assert(csp_initialised());

    // First check if we've received a timeout signal
    if (timeout_occurred) {
        return false;  // Exit early if timeout occurred
    }

    // Check if we've assigned all variables - if so, we've found a complete solution
    for (size_t v = 0; v < ctx->num_domains; ++v) {
        if (!ctx->assigned[v]) goto proceed;  // At least one variable is still unassigned
    }
    return true;  // All variables are assigned, solution found!

proceed:;
    // Step 1: Select the next variable to assign using the MRV heuristic
    // (chooses the variable with the fewest remaining values in its domain)
    size_t var = select_unassigned_var(ctx);

    // Step 2: Determine the maximum number of values in this variable's domain
    size_t max_vals = ctx->original_domain_sizes[var];

    // Allocate space for the ordered values buffer
    size_t *order = malloc(max_vals * sizeof(size_t));
    size_t n_vals = 0;  // Number of values after ordering

    // Step 3: Order the values for this variable using the LCV heuristic
    // (tries least constraining values first)
    order_values_lcv(csp, ctx, values, data, var, order, &n_vals);

    // Step 4: Try each possible value for this variable in the order determined by LCV
    for (size_t i = 0; i < n_vals; ++i) {
        // Get the current value to try
        size_t val = order[i];

        // Assign this value to the variable
        values[var] = val;

        // Mark this variable as assigned for forward checking
        ctx->assigned[var] = true;

        // Check consistency among already assigned variables
        if (!fc_is_consistent(csp, values, data, ctx)) {
            ctx->assigned[var] = false;
            continue;
        }

        // No need to check global consistency here since fc_is_consistent already
        // checks constraints among assigned variables only

        // Step 5: Forward checking - prune values from neighboring variables
        // that would conflict with ou
        // Step 5: Forward checking - prune values from neighboring variables
        // that would conflict with our current assignment

        // Allocate arrays for tracking pruned values:
        // - counts[v] will track how many values were pruned from variable v
        // - pruned[v] will store which values were pruned from variable v
        size_t *counts = calloc(ctx->num_domains, sizeof(size_t));
        size_t **pruned = calloc(ctx->num_domains, sizeof(size_t *));

        // Handle memory allocation failure
        if (!counts || !pruned) {
            free(counts);
            free(pruned);
            free(order);  // Free order memory too
            ctx->assigned[var] = false;
            return false;
        }
        // Perform the actual pruning of inconsistent values from future variables
        prune_neighbors(csp, values, data, ctx, var, counts, pruned);

        // Step 6: Recursively try to assign values to the remaining variables
        if (backtrack_fc(csp, values, data, ctx)) {
            // Solution found! Clean up and return success
            free(order);
            restore_pruned(ctx, counts, pruned);
            free(counts);
            free(pruned);
            return true;
        }

        // Step 7: If we reach here, this assignment didn't lead to a solution
        // Backtrack: restore the pruned values and try another value
        restore_pruned(ctx, counts, pruned);
        free(counts);
        free(pruned);

        // Mark this variable as unassigned again
        ctx->assigned[var] = false;
    }

    // If we've tried all values and none worked, backtrack further
    free(order);
    return false;
}

/**
 * @brief Creates and initializes a forward checking context.
 *
 * This function allocates and initializes all the data structures needed for
 * the forward checking algorithm:
 * 1. The original domain sizes for each variable
 * 2. The current available values for each variable (initially all values are available)
 * 3. The assignment status for each variable (initially all are unassigned)
 *
 * The context is crucial for implementing both the MRV and LCV heuristics as well as
 * for tracking domain pruning during forward checking.
 *
 * @param csp The CSP problem for which to create a context.
 * @return A newly created forward checking context, or NULL if memory allocation failed.
 */
CSPForwardCheckContext *csp_forward_check_context_create(const CSPProblem *csp) {
    assert(csp_initialised());

    // Get the number of variables in the CSP problem
    size_t n = csp_problem_get_num_domains(csp);

    // Step 1: Allocate memory for the context structure
    CSPForwardCheckContext *ctx = malloc(sizeof(*ctx));
    if (!ctx) return NULL;  // Memory allocation failed

    // Store the number of variables
    ctx->num_domains = n;

    // Step 2: Allocate memory for tracking:
    // - Which variables have been assigned (initialized to false/0 using calloc)
    ctx->assigned = calloc(n, sizeof(bool));

    // - The original size of each variable's domain
    ctx->original_domain_sizes = malloc(n * sizeof(size_t));

    // - The current state of each domain (which values are still available)
    ctx->current_domains = malloc(n * sizeof(bool *));

    // Check if all memory allocations succeeded
    if (!ctx->assigned || !ctx->original_domain_sizes || !ctx->current_domains) {
        // Clean up on failure
        free(ctx->assigned);
        free(ctx->original_domain_sizes);
        free(ctx->current_domains);
        free(ctx);
        return NULL;
    }

    for (size_t i = 0; i < n; ++i) {
        ctx->original_domain_sizes[i] = csp_problem_get_domain(csp, i);
        size_t d = ctx->original_domain_sizes[i];
        ctx->current_domains[i] = calloc(d, sizeof(bool));
        if (!ctx->current_domains[i]) {
            // nettoyage en cas d’échec
            for (size_t j = 0; j < i; ++j) {
                free(ctx->current_domains[j]);
            }
            free(ctx->assigned);
            free(ctx->original_domain_sizes);
            free(ctx->current_domains);
            free(ctx);
            return NULL;
        }
        for (size_t v = 0; v < d; ++v) {
            ctx->current_domains[i][v] = true;
        }
    }
    // ---- Initial pruning for unary (pre-assigned) constraints ----
    // Iterate all constraints and prune domains for arity-1 constraints
    size_t total_cons = csp_problem_get_num_constraints(csp);
    // Temporary assignment array for testing
    size_t *tmp_vals = calloc(ctx->num_domains, sizeof(size_t));
    for (size_t ci = 0; ci < total_cons; ++ci) {
        CSPConstraint *con = csp_problem_get_constraint(csp, ci);
        if (csp_constraint_get_arity(con) != 1) continue;  // only unary
        size_t var = csp_constraint_get_variable(con, 0);
        size_t dsize = ctx->original_domain_sizes[var];
        CSPChecker *checker = csp_constraint_get_check(con);
        // Test each value in the variable's domain
        for (size_t d = 0; d < dsize; ++d) {
            if (!ctx->current_domains[var][d]) continue;
            tmp_vals[var] = d;
            // If the constraint fails, mark this value unavailable
            if (!checker(con, tmp_vals, NULL)) {
                ctx->current_domains[var][d] = false;
            }
        }
        // If exactly one value remains, consider variable assigned
        size_t count = 0;
        for (size_t d = 0; d < dsize; ++d) {
            if (ctx->current_domains[var][d]) ++count;
        }
        if (count == 1) ctx->assigned[var] = true;
    }
    free(tmp_vals);
    return ctx;
}

/**
 * @brief Destroys a forward checking context and frees all associated memory.
 *
 * This function properly deallocates all resources used by a forward checking context:
 * 1. The arrays tracking available values for each domain
 * 2. The array of original domain sizes
 * 3. The array tracking variable assignments
 * 4. The context structure itself
 *
 * The function safely handles NULL contexts by doing nothing.
 *
 * @param ctx The forward checking context to destroy.
 */
void csp_forward_check_context_destroy(CSPForwardCheckContext *ctx) {
    assert(csp_initialised());

    // Safety check - if context is NULL, nothing to free
    if (!ctx) return;

    // Step 1: Free each variable's domain availability array
    for (size_t i = 0; i < ctx->num_domains; ++i) {
        free(ctx->current_domains[i]);
    }

    // Step 2: Free all the main arrays in the context
    free(ctx->current_domains);        // Array of pointers to domain arrays
    free(ctx->original_domain_sizes);  // Array of original domain sizes
    free(ctx->assigned);               // Array tracking assigned variables

    // Step 3: Free the context structure itself
    free(ctx);
}

/**
 * @brief Solves a CSP problem using the forward checking algorithm with heuristics.
 *
 * This function serves as the main entry point for solving a CSP using forward checking.
 * It performs the following steps:
 * 1. Creates a forward checking context for the problem
 * 2. Calls the recursive backtracking function to find a solution
 * 3. Destroys the context after solving (regardless of whether a solution was found)
 * 4. Returns whether a solution was found
 *
 * The forward checking algorithm with MRV and LCV heuristics typically performs much better
 * than basic backtracking, especially for larger or more constrained problems.
 *
 * @param csp The CSP problem to solve.
 * @param values Array to store the assignments of values to variables.
 * @param data User-provided data for constraint checking.
 * @return true if a solution is found, false otherwise.
 */
bool csp_problem_solve_forward_checking(const CSPProblem *csp, size_t *values, const void *data) {
    assert(csp_initialised());

    // Step 1: Create a context for forward checking algorithm
    // This will track domain pruning and variable assignments
    CSPForwardCheckContext *ctx = csp_forward_check_context_create(csp);

    // Handle memory allocation failure
    if (!ctx) return false;

    // Step 2: Call the recursive backtracking function with forward checking
    // This is where the actual solving happens
    bool solved = backtrack_fc(csp, values, data, ctx);

    // Step 3: Clean up resources regardless of whether a solution was found
    csp_forward_check_context_destroy(ctx);

    // Return whether we found a solution or not
    return solved;
}
