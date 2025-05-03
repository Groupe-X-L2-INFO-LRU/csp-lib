#include "forward-checking.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "heuristics.h"

static bool backtrack_fc(const CSPProblem *csp, size_t *values, const void *data,
                         CSPForwardCheckContext *ctx) {
    assert(csp_initialised());
    // check completion
    for (size_t v = 0; v < ctx->num_domains; ++v) {
        if (!ctx->assigned[v]) goto proceed;
    }
    return true;
proceed:;
    // select variable and order values via heuristics module
    size_t var = select_unassigned_var(ctx);
    size_t max_vals = ctx->original_domain_sizes[var];
    size_t *order = malloc(max_vals * sizeof(size_t));
    size_t n_vals = 0;
    order_values_lcv(csp, ctx, values, data, var, order, &n_vals);

    for (size_t i = 0; i < n_vals; ++i) {
        size_t val = order[i];
        values[var] = val;
        if (!csp_problem_is_consistent(csp, values, data, ctx->num_domains)) continue;
        ctx->assigned[var] = true;
        // forward check prune
        size_t *counts = calloc(ctx->num_domains, sizeof(size_t));
        size_t **pruned = calloc(ctx->num_domains, sizeof(size_t *));
        if (!counts || !pruned) {
            free(counts);
            free(pruned);
            ctx->assigned[var] = false;
            return false;
        }
        prune_neighbors(csp, values, data, ctx, var, counts, pruned);
        if (backtrack_fc(csp, values, data, ctx)) {
            free(order);
            restore_pruned(ctx, counts, pruned);
            free(counts);
            return true;
        }
        restore_pruned(ctx, counts, pruned);
        ctx->assigned[var] = false;
    }
    free(order);
    return false;
}

CSPForwardCheckContext *csp_forward_check_context_create(const CSPProblem *csp) {
    assert(csp_initialised());
    size_t n = csp_problem_get_num_domains(csp);
    CSPForwardCheckContext *ctx = malloc(sizeof(*ctx));
    if (!ctx) return NULL;

    ctx->num_domains = n;
    ctx->assigned = calloc(n, sizeof(bool));
    ctx->original_domain_sizes = malloc(n * sizeof(size_t));
    ctx->current_domains = malloc(n * sizeof(bool *));
    if (!ctx->assigned || !ctx->original_domain_sizes || !ctx->current_domains) {
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

void csp_forward_check_context_destroy(CSPForwardCheckContext *ctx) {
    assert(csp_initialised());
    if (!ctx) return;
    for (size_t i = 0; i < ctx->num_domains; ++i) free(ctx->current_domains[i]);
    free(ctx->current_domains);
    free(ctx->original_domain_sizes);
    free(ctx->assigned);
    free(ctx);
}

bool csp_problem_solve_forward_checking(const CSPProblem *csp, size_t *values, const void *data) {
    assert(csp_initialised());
    CSPForwardCheckContext *ctx = csp_forward_check_context_create(csp);
    if (!ctx) return false;
    bool solved = backtrack_fc(csp, values, data, ctx);
    csp_forward_check_context_destroy(ctx);
    return solved;
}
