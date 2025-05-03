#include <assert.h>
#include <stdio.h>

#include "../src/csp.h"
#include "../src/forward-checking.h"
#include "../src/csp.inc"

// Constraint functions

static bool diff(const CSPConstraint *constraint, const size_t *values) {
    size_t a = csp_constraint_get_variable(constraint, 0);
    size_t b = csp_constraint_get_variable(constraint, 1);
    return values[a] != values[b];
}

static bool sumLeq(const CSPConstraint *constraint, const size_t *values, const void *data) {
    size_t a = csp_constraint_get_variable(constraint, 0);
    size_t b = csp_constraint_get_variable(constraint, 1);
    const size_t max_sum = *(const size_t *)data;
    return values[a] + values[b] <= max_sum;
}

// Test cases

static bool always_true(const CSPConstraint *c, const size_t *v, const void *d) { (void)c; (void)v; (void)d; return true; }

void test_single_var(void) {
    CSPProblem *csp = csp_problem_create(1, 1);
    csp_problem_set_domain(csp, 0, 3);
    // Contrainte triviale toujours vraie
    CSPConstraint *con = csp_constraint_create(1, always_true);
    csp_constraint_set_variable(con, 0, 0);
    csp_problem_set_constraint(csp, 0, con);
    size_t values[1];
    assert(csp_problem_solve_forward_checking(csp, values, NULL));
    assert(values[0] < 3);
    csp_constraint_destroy(con);
    csp_problem_destroy(csp);
    printf("test_single_var passed\n");
}

void test_two_var_diff(void) {
    CSPProblem *csp = csp_problem_create(2, 1);
    csp_problem_set_domain(csp, 0, 2);
    csp_problem_set_domain(csp, 1, 2);
    CSPConstraint *con = csp_constraint_create(2, (bool (*)(const CSPConstraint *, const size_t *, const void *))diff);
    csp_constraint_set_variable(con, 0, 0);
    csp_constraint_set_variable(con, 1, 1);
    csp_problem_set_constraint(csp, 0, con);
    size_t values[2];
    assert(csp_problem_solve_forward_checking(csp, values, NULL));
    assert(values[0] != values[1]);
    csp_constraint_destroy(con);
    csp_problem_destroy(csp);
    printf("test_two_var_diff passed\n");
}

void test_unsatisfiable(void) {
    CSPProblem *csp = csp_problem_create(2, 1);
    csp_problem_set_domain(csp, 0, 1);
    csp_problem_set_domain(csp, 1, 1);
    CSPConstraint *con = csp_constraint_create(2, (bool (*)(const CSPConstraint *, const size_t *, const void *))diff);
    csp_constraint_set_variable(con, 0, 0);
    csp_constraint_set_variable(con, 1, 1);
    csp_problem_set_constraint(csp, 0, con);
    size_t values[2];
    assert(!csp_problem_solve_forward_checking(csp, values, NULL));
    csp_constraint_destroy(con);
    csp_problem_destroy(csp);
    printf("test_unsatisfiable passed\n");
}

void test_three_var_diff(void) {
    CSPProblem *csp = csp_problem_create(3, 2);
    csp_problem_set_domain(csp, 0, 3);
    csp_problem_set_domain(csp, 1, 3);
    csp_problem_set_domain(csp, 2, 3);
    CSPConstraint *c1 = csp_constraint_create(2, (bool (*)(const CSPConstraint *, const size_t *, const void *))diff);
    csp_constraint_set_variable(c1, 0, 0);
    csp_constraint_set_variable(c1, 1, 1);
    CSPConstraint *c2 = csp_constraint_create(2, (bool (*)(const CSPConstraint *, const size_t *, const void *))diff);
    csp_constraint_set_variable(c2, 0, 1);
    csp_constraint_set_variable(c2, 1, 2);
    csp_problem_set_constraint(csp, 0, c1);
    csp_problem_set_constraint(csp, 1, c2);
    size_t values[3];
    assert(csp_problem_solve_forward_checking(csp, values, NULL));
    assert(values[0] != values[1]);
    assert(values[1] != values[2]);
    csp_constraint_destroy(c1);
    csp_constraint_destroy(c2);
    csp_problem_destroy(csp);
    printf("test_three_var_diff passed\n");
}

void test_with_data(void) {
    CSPProblem *csp = csp_problem_create(2, 1);
    csp_problem_set_domain(csp, 0, 3);
    csp_problem_set_domain(csp, 1, 3);
    CSPConstraint *con = csp_constraint_create(2, sumLeq);
    csp_constraint_set_variable(con, 0, 0);
    csp_constraint_set_variable(con, 1, 1);
    csp_problem_set_constraint(csp, 0, con);
    size_t values[2];
    const size_t max_sum = 3;
    assert(csp_problem_solve_forward_checking(csp, values, &max_sum));
    assert(values[0] + values[1] <= max_sum);
    csp_constraint_destroy(con);
    csp_problem_destroy(csp);
    printf("test_with_data passed\n");
}

int main(void) {
    csp_init();
    test_single_var();
    test_two_var_diff();
    test_unsatisfiable();
    test_three_var_diff();
    test_with_data();
    csp_finish();
    printf("All forward checking tests passed\n");
    return 0;
}
