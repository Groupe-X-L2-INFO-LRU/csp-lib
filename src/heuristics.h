#ifndef HEURISTICS_H_
#define HEURISTICS_H_

#include <stddef.h>
#include "forward-checking.h"

/**
 * @brief Select the next unassigned variable (MRV heuristic).
 */
extern size_t select_unassigned_var(const CSPForwardCheckContext *ctx);

/**
 * @brief Order values for a given variable (LCV heuristic).
 */
extern void order_values_lcv(const CSPProblem *csp,
                             CSPForwardCheckContext *ctx,
                             const size_t *values,
                             const void *data,
                             size_t var,
                             size_t *ordered,
                             size_t *n_vals);

/**
 * @brief Prune inconsistent neighbor values for a given assignment.
 */
extern void prune_neighbors(const CSPProblem *csp,
                            const size_t *values,
                            const void *data,
                            CSPForwardCheckContext *ctx,
                            size_t var,
                            size_t *counts,
                            size_t **pruned);

/**
 * @brief Restore pruned values after backtracking.
 */
extern void restore_pruned(CSPForwardCheckContext *ctx,
                           const size_t *counts,
                           size_t **pruned);

#endif  // HEURISTICS_H_

