#include <assert.h>
#include <stdio.h>
#include "../src/csp.h"
#include "../src/forward-checking.h"

void test_context_create_destroy(void) {
    csp_init();
    CSPProblem *csp = csp_problem_create(3, 0);
    csp_problem_set_domain(csp, 0, 2);
    csp_problem_set_domain(csp, 1, 3);
    csp_problem_set_domain(csp, 2, 4);

    CSPForwardCheckContext *ctx = csp_forward_check_context_create(csp);
    assert(ctx != NULL);
    assert(ctx->num_domains == 3);
    /* Domain sizes */
    assert(ctx->original_domain_sizes[0] == 2);
    assert(ctx->original_domain_sizes[1] == 3);
    assert(ctx->original_domain_sizes[2] == 4);
    /* Tous les états non assignés, et domaines initiaux à true */
    for (size_t i = 0; i < 3; ++i) {
        assert(ctx->assigned[i] == false);
        for (size_t v = 0; v < ctx->original_domain_sizes[i]; ++v)
            assert(ctx->current_domains[i][v] == true);
    }

    csp_forward_check_context_destroy(ctx);
    csp_problem_destroy(csp);
    csp_finish();
    printf("test_context_create_destroy passed\n");
}

int main(void) {
    test_context_create_destroy();
    return 0;
}
