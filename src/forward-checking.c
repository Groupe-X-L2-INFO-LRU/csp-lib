#include "forward-checking.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "heuristics.h"

/**
 * @file forward-checking.c
 * @brief Implementation    // Step 3: Initialize the domain information for each variable
    for (size_t i = 0; i < n; ++i) {
        // Get the domain size for this variable from the CSP problem
        ctx->original_domain_sizes[i] = csp_problem_get_domain(csp, i);
        size_t d = ctx->original_domain_sizes[i];

        // Allocate memory for tracking which values are available in this domain
        ctx->current_domains[i] = calloc(d, sizeof(bool));

        // Check if memory allocation succeeded
        if (!ctx->current_domains[i]) {
            // Clean up on failure: free all previously allocated domain arrays
            for (size_t j = 0; j < i; ++j) {
                free(ctx->current_domains[j]);
            }
            free(ctx->assigned);
            free(ctx->original_domain_sizes);
            free(ctx->current_domains);
            free(ctx);
            return NULL;
        }

        // Initialize all values in this domain as available (not pruned)
        for (size_t v = 0; v < d; ++v) {
            ctx->current_domains[i][v] = true;
        }
    }

    // Return the initialized context
    return ctx;king algorithm for CSP solving.
 *
 * This file provides the implementation of the forward checking algorithm with MRV
 * (Minimum Remaining Values) and LCV (Least Constraining Value) heuristics for solving
 * constraint satisfaction problems. Forward checking is an optimization over basic
 * backtracking that prunes inconsistent values as soon as a variable is assigned.
 *
 * @author Ch. Demko (original)
 * @date 2024-2025
 * @version 1.0
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
static bool backtrack_fc(const CSPProblem *csp, size_t *values, const void *data,
                         CSPForwardCheckContext *ctx) {
    assert(csp_initialised());

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

    // Allocate space for the ordered values
    size_t *order = malloc(max_vals * sizeof(size_t));
    size_t n_vals = 0;  // Will hold the actual number of valid values

    // Step 3: Order the values for this variable using the LCV heuristic
    // (order values by how constraining they are on neighboring variables)
    order_values_lcv(csp, ctx, values, data, var, order, &n_vals);

    // Step 4: Try each possible value for this variable in the order determined by LCV
    for (size_t i = 0; i < n_vals; ++i) {
        // Get the current value to try
        size_t val = order[i];

        // Assign this value to the variable
        values[var] = val;

        // Check if this assignment is consistent with all constraints
        // (only checks constraints where all variables are already assigned)
        if (!csp_problem_is_consistent(csp, values, data, ctx->num_domains)) continue;

        // Mark this variable as assigned
        ctx->assigned[var] = true;

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
            return true;
        }

        // Step 7: If we reach here, this assignment didn't lead to a solution
        // Backtrack: restore the pruned values and try another value
        restore_pruned(ctx, counts, pruned);

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
