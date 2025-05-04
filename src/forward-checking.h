#ifndef FORWARD_CHECKING_H_
#define FORWARD_CHECKING_H_

#include "csp.h"

/**
 * @file forward-checking.h
 * @brief Forward checking algorithm for constraint satisfaction problems.
 * @ingroup algorithms
 *
 * This file provides an implementation of the forward checking algorithm for solving
 * constraint satisfaction problems. Forward checking is an improvement over basic
 * backtracking that prunes inconsistent values from variable domains as soon as
 * a variable assignment is made.
 *
 * @section fc_algorithm The Forward Checking Algorithm
 * 
 * Forward checking works by:
 * 1. Assigning a value to a variable
 * 2. Immediately checking all constraints involving that variable and unassigned variables
 * 3. Temporarily removing inconsistent values from the domains of unassigned variables
 * 4. If any domain becomes empty, backtracking immediately
 * 5. When backtracking, restoring previously pruned domain values
 *
 * This approach helps detect failures earlier than naive backtracking, avoiding unnecessary
 * exploration of dead-end paths in the search space.
 *
 * @section fc_heuristics Integrated Heuristics
 *
 * The implementation incorporates two key heuristics:
 * - **Minimum Remaining Values (MRV)**: Prioritizes variables with the fewest valid values
 *   in their domain, focusing search on the most constrained variables first.
 * - **Least Constraining Value (LCV)**: Prioritizes values that eliminate the fewest options
 *   for neighboring variables, maximizing flexibility for future assignments.
 *
 * @section fc_example Example Usage
 * 
 * ```c
 * // Create a CSP problem
 * CSPProblem *problem = create_my_csp_problem();
 * 
 * // Array to store the solution
 * size_t solution[NUM_VARS];
 * 
 * // Solve using forward checking
 * bool solved = solve_with_forward_checking(problem, solution, NULL, NULL);
 * 
 * if (solved) {
 *     // Process solution
 * } else {
 *     // No solution exists
 * }
 * ```
 *
 * @author Ch. Demko (original)
 * @date 2024-2025
 * @version 1.0
 */

/**
 * @brief Context for forward checking with heuristics.
 *
 * This structure maintains the state required for forward checking algorithm
 * with the MRV (Minimum Remaining Values) and LCV (Least Constraining Value) heuristics.
 * It keeps track of:
 * - The original domain sizes for each variable
 * - The current available values in each domain (after pruning)
 * - Which variables have already been assigned
 *
 * @var num_domains The total number of variables (domains) in the CSP
 * @var original_domain_sizes Array storing the original size of each variable's domain
 * @var current_domains 2D array where current_domains[i][j] is true if value j is still valid for
 * variable i
 * @var assigned Array indicating which variables have been assigned values
 */
typedef struct _CSPForwardCheckContext {
    size_t num_domains;            /* Number of variables in the CSP */
    size_t *original_domain_sizes; /* Original size of each variable's domain */
    bool **current_domains;        /* Available values for each variable (after pruning) */
    bool *assigned;                /* Indicates which variables have been assigned */
} CSPForwardCheckContext;

/**
 * @brief Creates a forward checking context for the given CSP problem.
 *
 * This function initializes a new context for the forward checking algorithm
 * based on the provided CSP problem. It allocates memory for tracking:
 * - Original domain sizes
 * - Current available domain values
 * - Variable assignment status
 *
 * Initially, all domain values are marked as available, and no variables are assigned.
 *
 * @param csp The CSP problem for which to create a forward checking context.
 * @return A new forward checking context, or NULL if memory allocation failed.
 * @pre The CSP library must be initialized via csp_init().
 * @pre csp must not be NULL.
 * @post If successful, returns a fully initialized context with all domains marked as fully
 * available.
 * @post If memory allocation fails at any stage, all already allocated memory is freed and NULL is
 * returned.
 */
extern CSPForwardCheckContext *csp_forward_check_context_create(const CSPProblem *csp);

/**
 * @brief Destroys a forward checking context and frees all associated resources.
 *
 * This function deallocates all memory used by the forward checking context,
 * including domain availability arrays and other internal state.
 *
 * @param context The forward checking context to destroy.
 * @pre The CSP library must be initialized via csp_init().
 * @pre context can be NULL (in which case the function does nothing).
 * @post All memory associated with the context is freed.
 */
extern void csp_forward_check_context_destroy(CSPForwardCheckContext *context);

/**
 * @brief Solves a CSP problem using the forward checking algorithm with heuristics.
 *
 * This function implements the forward checking algorithm, which is an improvement over
 * basic backtracking. Forward checking works by immediately pruning from the domains of
 * future variables any values that would violate a constraint with the current assignment.
 *
 * This implementation also incorporates two powerful heuristics:
 *
 * 1. Minimum Remaining Values (MRV): Selects the variable with the fewest remaining
 *    valid values in its domain, which helps identify bottlenecks early.
 *
 * 2. Least Constraining Value (LCV): Orders values for a variable by how many options
 *    they eliminate from the domains of neighboring variables, trying values that
 *    constrain neighbors the least first.
 *
 * @param csp The CSP problem to solve.
 * @param values Array to store the assignments of values to variables.
 * @param data User-provided data to pass to constraint check functions.
 * @return true if a solution is found, false if no solution exists.
 * @pre The CSP library must be initialized via csp_init().
 * @pre csp must not be NULL.
 * @pre values must not be NULL and must be large enough to hold assignments for all variables.
 * @post If a solution is found, values contains the solution assignments.
 */
extern bool csp_problem_solve_forward_checking(const CSPProblem *csp, size_t *values,
                                               const void *data);

#endif  // FORWARD_CHECKING_H_