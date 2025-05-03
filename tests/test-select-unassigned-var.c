#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/forward-checking.h"
#include "../src/heuristics.h"

// Mock CSPForwardCheckContext for select_unassigned_var
static CSPForwardCheckContext* make_ctx(size_t n, const size_t *domains, const bool **mask, const bool *assigned) {
    CSPForwardCheckContext *ctx = malloc(sizeof(*ctx));
    ctx->num_domains = n;
    ctx->original_domain_sizes = malloc(n * sizeof(size_t));
    ctx->current_domains = malloc(n * sizeof(bool*));
    ctx->assigned = malloc(n * sizeof(bool));
    for (size_t i = 0; i < n; ++i) {
        ctx->original_domain_sizes[i] = domains[i];
        ctx->current_domains[i] = malloc(domains[i] * sizeof(bool));
        for (size_t v = 0; v < domains[i]; ++v)
            ctx->current_domains[i][v] = mask[i][v];
        ctx->assigned[i] = assigned[i];
    }
    return ctx;
}

static void destroy_ctx(CSPForwardCheckContext *ctx) {
    for (size_t i = 0; i < ctx->num_domains; ++i)
        free(ctx->current_domains[i]);
    free(ctx->current_domains);
    free(ctx->original_domain_sizes);
    free(ctx->assigned);
    free(ctx);
}

void test_mrv_simple(void) {
    // 3 variables: domains sizes [2,3,1]
    size_t domains[3] = {2,3,1};
    bool m0[] = {true,true};    // var0 has 2 vals
    bool m1[] = {true,false,true}; // var1 has 2 vals
    bool m2[] = {true};         // var2 has 1 val (smallest)
    const bool *mask[3] = {m0, m1, m2};
    bool assigned[3] = {false,false,false};

    CSPForwardCheckContext *ctx = make_ctx(3, domains, mask, assigned);
    size_t sel = select_unassigned_var(ctx);
    assert(sel == 2);
    printf("test_mrv_simple passed\n");
    destroy_ctx(ctx);
}

void test_mrv_skip_assigned(void) {
    size_t domains[3] = {4,2,3};
    bool m0[] = {true,true,true,true};
    bool m1[] = {true,false};
    bool m2[] = {true,true,true};
    const bool *mask[3] = {m0, m1, m2};
    bool assigned[3] = {true,false,false};

    CSPForwardCheckContext *ctx = make_ctx(3, domains, mask, assigned);
    // var0 is assigned ⇒ skip. Next smallest is var1 (2 values).
    size_t sel = select_unassigned_var(ctx);
    assert(sel == 1);
    printf("test_mrv_skip_assigned passed\n");
    destroy_ctx(ctx);
}

int main(void) {
    test_mrv_simple();
    test_mrv_skip_assigned();
    printf("All select_unassigned_var tests passed\n");
    return 0;
}
