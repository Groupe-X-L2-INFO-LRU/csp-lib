#ifndef HEURISTICS_H_
#define HEURISTICS_H_

#include <stddef.h>
#include "forward-checking.h"

/**
 * @file heuristics.h
 * @brief Heuristics for improving CSP solver performance.
 *
 * This file provides implementations of common heuristics used to improve the efficiency
 * of constraint satisfaction problem (CSP) solvers. These heuristics help guide the search
 * process by making intelligent choices about:
 * 
 * 1. Which variable to assign next (variable ordering)
 * 2. Which value to try first for a variable (value ordering)
 * 3. How to prune impossible values during forward checking
 * 4. How to restore pruned values during backtracking
 *
 * The main heuristics implemented are:
 * - Minimum Remaining Values (MRV): Choose the variable with the fewest remaining
 *   legal values in its domain
 * - Least Constraining Value (LCV): Choose the value that rules out the fewest choices
 *   for neighboring variables
 *
 * @author Ch. Demko (original)
 * @date 2024-2025
 * @version 1.0
 */

/**
 * @brief Selects the next unassigned variable using the MRV (Minimum Remaining Values) heuristic.
 *
 * The MRV heuristic chooses the variable with the fewest remaining legal values in its domain.
 * This is often referred to as the "most constrained variable" or "fail-first" heuristic,
 * as it tends to identify variables that are most likely to cause a failure quickly,
 * helping to prune the search tree more effectively.
 *
 * @param ctx The forward checking context containing domain and assignment information.
 * @return The index of the unassigned variable with the fewest remaining legal values.
 * @pre ctx must not be NULL.
 * @pre ctx must contain at least one unassigned variable.
 */
extern size_t select_unassigned_var(const CSPForwardCheckContext *ctx);

/**
 * @brief Orders values for a given variable using the LCV (Least Constraining Value) heuristic.
 *
 * The LCV heuristic chooses values in order of how many options they rule out for neighboring
 * variables. The values that eliminate the fewest options for neighbors are tried first.
 * This increases the likelihood of finding a valid assignment without having to backtrack.
 * 
 * This function counts "conflicts" - the number of values in neighboring variables that
 * would be ruled out by each possible value assignment - and sorts values from least
 * constraining (fewest conflicts) to most constraining (most conflicts).
 *
 * @param csp The CSP problem instance.
 * @param ctx The forward checking context.
 * @param values The current assignment of values to variables.
 * @param data User-provided data for constraint checking.
 * @param var The variable for which to order values.
 * @param ordered Output array where ordered values will be stored.
 * @param n_vals Output parameter that will be set to the number of valid values.
 * @pre csp, ctx, values, ordered, and n_vals must not be NULL.
 * @pre var must be a valid variable index in the CSP.
 * @pre ordered must have enough space to hold all possible values for the variable.
 * @post ordered[0..(*n_vals-1)] contains the values in the domain of var ordered by the LCV heuristic.
 * @post *n_vals is set to the number of valid values in the domain of var.
 */
extern void order_values_lcv(const CSPProblem *csp,
                             CSPForwardCheckContext *ctx,
                             const size_t *values,
                             const void *data,
                             size_t var,
                             size_t *ordered,
                             size_t *n_vals);

/**
 * @brief Prunes inconsistent values from neighboring variables after an assignment.
 *
 * This function is a key component of the forward checking algorithm. After assigning
 * a value to a variable, this function checks all neighboring variables (those that
 * share a constraint with the assigned variable) and removes any values that would
 * violate a constraint with the current assignment.
 *
 * The pruned values are recorded so they can be restored if backtracking becomes necessary.
 *
 * @param csp The CSP problem instance.
 * @param values The current assignment of values to variables.
 * @param data User-provided data for constraint checking.
 * @param ctx The forward checking context to update.
 * @param var The variable that was just assigned.
 * @param counts Array to record the number of pruned values for each variable.
 * @param pruned 2D array to record which values were pruned from each variable.
 * @pre csp, values, ctx, counts, and pruned must not be NULL.
 * @pre var must be a valid variable index in the CSP.
 * @pre counts must be an array of size ctx->num_domains, initialized to zeros.
 * @pre pruned must be an array of size ctx->num_domains, initialized to NULL.
 * @post Domain values that would violate constraints with the current assignment are marked as unavailable.
 * @post counts[i] records the number of values pruned from variable i.
 * @post pruned[i] records which values were pruned from variable i.
 */
extern void prune_neighbors(const CSPProblem *csp,
                            const size_t *values,
                            const void *data,
                            CSPForwardCheckContext *ctx,
                            size_t var,
                            size_t *counts,
                            size_t **pruned);

/**
 * @brief Restores previously pruned values when backtracking.
 *
 * When backtracking occurs (due to encountering a dead end in the search),
 * this function restores all the domain values that were previously pruned
 * during forward checking. This ensures that the search can continue correctly
 * with different assignments.
 *
 * @param ctx The forward checking context to update.
 * @param counts Array indicating how many values were pruned for each variable.
 * @param pruned 2D array indicating which values were pruned from each variable.
 * @pre ctx, counts, and pruned must not be NULL.
 * @pre pruned[i] must either be NULL or point to an array recording which values were pruned from variable i.
 * @post All pruned values are restored (marked as available again).
 * @post Memory allocated for the pruned arrays is freed.
 */
extern void restore_pruned(CSPForwardCheckContext *ctx,
                           const size_t *counts,
                           size_t **pruned);

#endif  // HEURISTICS_H_

