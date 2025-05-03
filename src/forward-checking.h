#ifndef FORWARD_CHECKING_H_
#define FORWARD_CHECKING_H_

#include "csp.h"

/**
 * @brief Context for forward checking with heuristics.
 */
typedef struct _CSPForwardCheckContext {
    size_t num_domains;
    size_t *original_domain_sizes;
    bool **current_domains;
    bool *assigned;
} CSPForwardCheckContext;

/**
 * @brief Create a forward checking context for the given CSP problem.
 * @param csp The CSP problem.
 * @return A new forward checking context, or NULL on failure.
 * @pre csp != NULL
 */
extern CSPForwardCheckContext *csp_forward_check_context_create(const CSPProblem *csp);

/**
 * @brief Destroy the forward checking context.
 * @param context The context to destroy.
 * @pre context != NULL
 */
extern void csp_forward_check_context_destroy(CSPForwardCheckContext *context);

/**
 * @brief Solve the CSP problem using forward checking with MRV and LCV heuristics.
 * @param csp The CSP problem to solve.
 * @param values Array to store variable assignments.
 * @param data User-provided data for constraint checks.
 * @return true if a solution is found, false otherwise.
 * @pre csp != NULL && values != NULL
 */
extern bool csp_problem_solve_forward_checking(const CSPProblem *csp,
                                               size_t *values,
                                               const void *data);

#endif  // FORWARD_CHECKING_H_