// tests_restore_pruned.c
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/forward-checking.h"
#include "../src/heuristics.h"

void test_restore_pruned(void) {
    /* On monte un contexte factice de taille 2 */
    CSPForwardCheckContext ctx = {
        .num_domains = 2,
        .original_domain_sizes = (size_t[]){2,2},
        .current_domains = (bool*[]){ malloc(2*sizeof(bool)), malloc(2*sizeof(bool)) },
        .assigned = (bool[]){false,false}
    };
    /* Prétend que la var1 a perdu la valeur 1 */
    ctx.current_domains[1][0] = true;
    ctx.current_domains[1][1] = false;
    size_t counts[2] = {0,1};
    size_t *pruned[2];
    pruned[1] = malloc(sizeof(size_t));
    pruned[1][0] = 1;

    restore_pruned(&ctx, counts, pruned);
    /* Doit être rétabli à true */
    assert(ctx.current_domains[1][1] == true);

    free(ctx.current_domains[0]);
    free(ctx.current_domains[1]);
    printf("test_restore_pruned passed\n");
}

int main(void) {
    test_restore_pruned();
    return 0;
}
