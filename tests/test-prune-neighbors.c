#include <assert.h>
#include <stdio.h>

#include "../src/csp.h"
#include "../src/forward-checking.h"
#include "../src/heuristics.h"

static bool diff(CSPConstraint const *c, const size_t *v, const void *data) {
    (void)data;  // unused
    size_t a = csp_constraint_get_variable(c, 0);
    size_t b = csp_constraint_get_variable(c, 1);
    return v[a] != v[b];
}

void test_prune_neighbors(void) {
    csp_init();
    CSPProblem *csp = csp_problem_create(2, 1);
    csp_problem_set_domain(csp, 0, 2);
    csp_problem_set_domain(csp, 1, 2);
    CSPConstraint *con = csp_constraint_create(2, diff);
    csp_constraint_set_variable(con, 0, 0);
    csp_constraint_set_variable(con, 1, 1);
    csp_problem_set_constraint(csp, 0, con);

    CSPForwardCheckContext *ctx = csp_forward_check_context_create(csp);
    size_t values[2] = {0, 0};
    size_t counts[2] = {0, 0};
    size_t *pruned[2] = {NULL, NULL};

    prune_neighbors(csp, values, NULL, ctx, 0, counts, pruned);
    assert(counts[1] == 1);
    assert(pruned[1][0] == 0);

    /* Pas de prune sur var0 */
    assert(counts[0] == 0);

    /* Cleanup */
    restore_pruned(ctx, counts, pruned);
    csp_forward_check_context_destroy(ctx);
    // Don't manually destroy constraints - let csp_problem_destroy handle it
    csp_problem_destroy(csp);
    csp_finish();
    printf("test_prune_neighbors passed\n");
}

int main(void) {
    test_prune_neighbors();
    return 0;
}
