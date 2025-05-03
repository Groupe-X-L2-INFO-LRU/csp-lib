#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#include "../src/csp.h"
#include "../src/forward-checking.h"
#include "../src/heuristics.h"

// Simple CSP with one binary constraint: sum ≤ max
static bool sumLeq(CSPConstraint const *c, const size_t *v, const void *d) {
    size_t a = csp_constraint_get_variable(c, 0);
    size_t b = csp_constraint_get_variable(c, 1);
    return v[a] + v[b] <= *(const size_t *)d;
}

void test_lcv_basic(void) {
    // Initialize the CSP library
    csp_init();

    // 2 variables both domain size 3: {0,1,2}
    CSPProblem *csp = csp_problem_create(2, 1);
    csp_problem_set_domain(csp, 0, 3);
    csp_problem_set_domain(csp, 1, 3);
    CSPConstraint *con = csp_constraint_create(2, sumLeq);
    csp_constraint_set_variable(con, 0, 0);
    csp_constraint_set_variable(con, 1, 1);
    csp_problem_set_constraint(csp, 0, con);

    // Prepare context
    CSPForwardCheckContext *ctx = csp_forward_check_context_create(csp);
    size_t values[2] = {SIZE_MAX, SIZE_MAX};
    size_t max = 2;

    // Order values for var0: those causing fewest prunings on var1 (sumLeq ≤2)
    size_t ordered[3];
    size_t n_vals = 0;
    order_values_lcv(csp, ctx, values, &max, 0, ordered, &n_vals);
    assert(n_vals == 3);
    // For val=0 ⇒ var1 candidates {0,1,2} ⇒ 0 prunings
    // For val=1 ⇒ var1 {0,1} ⇒ prunes 2 ⇒ 1 pruning
    // For val=2 ⇒ var1 {0} ⇒ prunes {1,2} ⇒ 2 prunings
    assert(ordered[0] == 0);
    assert(ordered[1] == 1);
    assert(ordered[2] == 2);

    printf("test_lcv_basic order: [%zu, %zu, %zu]\n", ordered[0], ordered[1], ordered[2]);

    // Cleanup
    csp_constraint_destroy(con);
    csp_problem_destroy(csp);
    csp_forward_check_context_destroy(ctx);
    // Finalize the CSP library
    csp_finish();
    printf("All order_values_lcv tests passed\n");
}

int main(void) {
    test_lcv_basic();
    return 0;
}
