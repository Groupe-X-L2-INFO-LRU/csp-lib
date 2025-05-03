#include "heuristics.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "csp.inc"

size_t select_unassigned_var(const CSPForwardCheckContext *ctx) {
    size_t best = SIZE_MAX, best_count = SIZE_MAX;
    for (size_t i = 0; i < ctx->num_domains; ++i) {
        if (ctx->assigned[i]) continue;
        size_t count = 0;
        for (size_t v = 0; v < ctx->original_domain_sizes[i]; ++v) {
            if (ctx->current_domains[i][v]) ++count;
        }
        if (count < best_count) {
            best_count = count;
            best = i;
        }
    }
    return best;
}

void order_values_lcv(const CSPProblem *csp, CSPForwardCheckContext *ctx, const size_t *values,
                      const void *data, size_t var, size_t *ordered, size_t *n_vals) {
    size_t dsize = ctx->original_domain_sizes[var];
    typedef struct {
        size_t val;
        size_t conflicts;
    } pair_t;
    pair_t *pairs = malloc(dsize * sizeof(pair_t));
    size_t n = 0;
    size_t num_constraints = csp_problem_get_num_constraints(csp);
    // Cast away const for temporary assignments
    size_t *vals = (size_t *)values;
    for (size_t val = 0; val < dsize; ++val) {
        if (!ctx->current_domains[var][val]) continue;
        pairs[n].val = val;
        size_t conflicts = 0;
        vals[var] = val;
        for (size_t ci = 0; ci < num_constraints; ++ci) {
            CSPConstraint *con = csp_problem_get_constraint(csp, ci);
            if (con->arity != 2) continue;
            size_t a = csp_constraint_get_variable(con, 0);
            size_t b = csp_constraint_get_variable(con, 1);
            size_t other = SIZE_MAX;
            if (a == var)
                other = b;
            else if (b == var)
                other = a;
            if (other == SIZE_MAX || ctx->assigned[other]) continue;
            for (size_t o_val = 0; o_val < ctx->original_domain_sizes[other]; ++o_val) {
                if (!ctx->current_domains[other][o_val]) continue;
                vals[other] = o_val;
                if (!con->check(con, vals, data)) {
                    ++conflicts;
                }
            }
        }
        pairs[n].conflicts = conflicts;
        ++n;
    }
    // Sort by conflicts (insertion sort)
    for (size_t i = 1; i < n; ++i) {
        pair_t key = pairs[i];
        size_t j = i;
        while (j > 0 && pairs[j - 1].conflicts > key.conflicts) {
            pairs[j] = pairs[j - 1];
            --j;
        }
        pairs[j] = key;
    }
    for (size_t i = 0; i < n; ++i) ordered[i] = pairs[i].val;
    *n_vals = n;
    free(pairs);
}

void prune_neighbors(const CSPProblem *csp, const size_t *values, const void *data,
                     CSPForwardCheckContext *ctx, size_t var, size_t *counts, size_t **pruned) {
    size_t num_constraints = csp_problem_get_num_constraints(csp);
    size_t *vals = (size_t *)values;
    for (size_t ci = 0; ci < num_constraints; ++ci) {
        CSPConstraint *con = csp_problem_get_constraint(csp, ci);
        if (con->arity != 2) continue;
        size_t a = csp_constraint_get_variable(con, 0);
        size_t b = csp_constraint_get_variable(con, 1);
        size_t other = (a == var ? b : (b == var ? a : SIZE_MAX));
        if (other == SIZE_MAX || ctx->assigned[other]) continue;
        if (!pruned[other]) {
            pruned[other] = malloc(ctx->original_domain_sizes[other] * sizeof(size_t));
        }
        // var's value is already in vals[var]
        for (size_t o_val = 0; o_val < ctx->original_domain_sizes[other]; ++o_val) {
            if (!ctx->current_domains[other][o_val]) continue;
            vals[other] = o_val;
            if (!con->check(con, vals, data)) {
                ctx->current_domains[other][o_val] = false;
                pruned[other][counts[other]++] = o_val;
            }
        }
    }
}

void restore_pruned(CSPForwardCheckContext *ctx, const size_t *counts, size_t **pruned) {
    for (size_t i = 0; i < ctx->num_domains; ++i) {
        for (size_t j = 0; j < counts[i]; ++j) {
            ctx->current_domains[i][pruned[i][j]] = true;
        }
        free(pruned[i]);
    }
}
