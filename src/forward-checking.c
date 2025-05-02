
#include "forward-checking.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "csp.h"
#include "csp.inc"

/**
 * @brief Internal backtrack function with forward checking.
 */
static bool backtrack_fc(const CSPProblem *csp,
                         size_t *values,
                         const void *data,
                         CSPForwardCheckContext *ctx,
                         size_t index) {
    assert(csp_initialised());
    size_t num_vars = ctx->num_domains;

    /* Base case: all variables assigned */
    if (index == num_vars) {
        return true;
    }

    /* Try each value in the current domain */
    for (size_t val = 0; val < ctx->original_domain_sizes[index]; ++val) {
        if (!ctx->current_domains[index][val]) {
            continue;
        }
        values[index] = val;

        /* Check consistency with already assigned variables */
        if (!csp_problem_is_consistent(csp, values, data, index + 1)) {
            continue;
        }

        /* Record pruned values to restore on backtrack */
        size_t num_constraints = csp_problem_get_num_constraints(csp);
        size_t *pruned_counts = calloc(num_vars, sizeof(size_t));
        size_t **pruned_values = calloc(num_vars, sizeof(size_t *));
        if (!pruned_counts || !pruned_values) {
            free(pruned_counts);
            free(pruned_values);
            return false;
        }

        /* Forward-check: prune inconsistent values for unassigned vars */
        for (size_t var = index + 1; var < num_vars; ++var) {
            pruned_values[var] = malloc(ctx->original_domain_sizes[var] * sizeof(size_t));
            for (size_t candidate = 0; candidate < ctx->original_domain_sizes[var]; ++candidate) {
                if (!ctx->current_domains[var][candidate]) {
                    continue;
                }
                /* Temporarily assign and test each binary constraint */
                values[var] = candidate;
                bool consistent = true;
                for (size_t ci = 0; ci < num_constraints; ++ci) {
                    CSPConstraint *constraint = csp_problem_get_constraint(csp, ci);
                    /* Identify binary constraint between 'index' and 'var' */
                    if (constraint->arity == 2) {
                        size_t a = constraint->variables[0];
                        size_t b = constraint->variables[1];
                        if ((a == index && b == var) || (a == var && b == index)) {
                            if (!constraint->check(constraint, values, data)) {
                                consistent = false;
                                break;
                            }
                        }
                    }
                }

                if (!consistent) {
                    /* Prune this candidate */
                    ctx->current_domains[var][candidate] = false;
                    pruned_values[var][pruned_counts[var]++] = candidate;
                }
            }
        }

        /* Recurse */
        if (backtrack_fc(csp, values, data, ctx, index + 1)) {
            /* Cleanup pruned memory */
            for (size_t v = index + 1; v < num_vars; ++v) {
                free(pruned_values[v]);
            }
            free(pruned_values);
            free(pruned_counts);
            return true;
        }

        /* Restore pruned values if backtracking */
        for (size_t v = index + 1; v < num_vars; ++v) {
            for (size_t i = 0; i < pruned_counts[v]; ++i) {
                ctx->current_domains[v][pruned_values[v][i]] = true;
            }
            free(pruned_values[v]);
        }
        free(pruned_values);
        free(pruned_counts);
    }

    return false;
}

CSPForwardCheckContext *csp_forward_check_context_create(const CSPProblem *csp) {
    assert(csp_initialised());
    size_t num_vars = csp_problem_get_num_domains(csp);
    CSPForwardCheckContext *ctx = malloc(sizeof(*ctx));
    if (!ctx) {
        return NULL;
    }
    ctx->num_domains = num_vars;
    ctx->original_domain_sizes = malloc(sizeof(size_t) * num_vars);
    ctx->current_domains = malloc(sizeof(bool *) * num_vars);
    if (!ctx->original_domain_sizes || !ctx->current_domains) {
        free(ctx->original_domain_sizes);
        free(ctx->current_domains);
        free(ctx);
        return NULL;
    }

    for (size_t i = 0; i < num_vars; ++i) {
        ctx->original_domain_sizes[i] = csp_problem_get_domain(csp, i);
        size_t size = ctx->original_domain_sizes[i];
        ctx->current_domains[i] = calloc(size, sizeof(bool));
        if (!ctx->current_domains[i]) {
            for (size_t j = 0; j < i; ++j) {
                free(ctx->current_domains[j]);
            }
            free(ctx->original_domain_sizes);
            free(ctx->current_domains);
            free(ctx);
            return NULL;
        }
        for (size_t v = 0; v < size; ++v) {
            ctx->current_domains[i][v] = true;
        }
    }
    return ctx;
}