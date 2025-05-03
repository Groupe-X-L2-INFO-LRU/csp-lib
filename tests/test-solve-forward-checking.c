#include <assert.h>
#include <stdio.h>
#include "../src/csp.h"
#include "../src/forward-checking.h"

/* Contrainte simple : deux vars diff */
static bool diff(CSPConstraint const *c, const size_t *v, const void *data) {
    (void)data; // unused
    size_t a = csp_constraint_get_variable(c, 0);
    size_t b = csp_constraint_get_variable(c, 1);
    return v[a] != v[b];
}

void test_solve_simple(void) {
    csp_init();
    CSPProblem *csp = csp_problem_create(2, 1);
    csp_problem_set_domain(csp, 0, 2);
    csp_problem_set_domain(csp, 1, 2);
    CSPConstraint *con = csp_constraint_create(2, diff);
    csp_constraint_set_variable(con, 0, 0);
    csp_constraint_set_variable(con, 1, 1);
    csp_problem_set_constraint(csp, 0, con);

    size_t vals[2];
    bool ok = csp_problem_solve_forward_checking(csp, vals, NULL);
    assert(ok);
    assert(vals[0] != vals[1]);

    csp_constraint_destroy(con);
    csp_problem_destroy(csp);
    csp_finish();
    printf("test_solve_simple passed\n");
}

int main(void) {
    test_solve_simple();
    return 0;
}
