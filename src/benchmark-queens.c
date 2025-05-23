#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "csp.h"
#include "forward-checking.h"

/** @author : Gwendal Henry */

// Vérifie la compatibilité entre deux reines
bool queen_compatibles(CSPConstraint *constraint, const size_t *values, unsigned int *data) {
    size_t x0 = csp_constraint_get_variable(constraint, 0);
    size_t x1 = csp_constraint_get_variable(constraint, 1);
    size_t y0 = values[x0];
    size_t y1 = values[x1];
    return y0 != y1 && x0 + y1 != x1 + y0 && x0 + y0 != x1 + y1;
}

static double timespec_to_ms(struct timespec *start, struct timespec *end) {
    return (end->tv_sec - start->tv_sec) * 1000.0 + (end->tv_nsec - start->tv_nsec) / 1e6;
}

bool benchmark_solve_methods(unsigned int number, double *timeFC, double *timeBasic) {
    size_t *queens = calloc(number, sizeof(size_t));
    if (!queens) return false;

    // Création du problème CSP
    size_t index;
    CSPProblem *problem = csp_problem_create(number, number * (number - 1) / 2);
    for (size_t i = 0; i < number; i++) {
        csp_problem_set_domain(problem, i, number);
    }
    index = 0;
    for (size_t i = 0; i < number - 1; i++) {
        for (size_t j = i + 1; j < number; j++) {
            csp_problem_set_constraint(problem, index, csp_constraint_create(2, (CSPChecker *)queen_compatibles));
            csp_constraint_set_variable(csp_problem_get_constraint(problem, index), 0, i);
            csp_constraint_set_variable(csp_problem_get_constraint(problem, index), 1, j);
            index++;
        }
    }

    struct timespec startFC, endFC;
    clock_gettime(CLOCK_MONOTONIC, &startFC);
    bool resultFC = csp_problem_solve_forward_checking(problem, queens, NULL);
    clock_gettime(CLOCK_MONOTONIC, &endFC);
    *timeFC = timespec_to_ms(&startFC, &endFC);

    // Remise à zéro du tableau pour le second solveur
    for (unsigned int i = 0; i < number; i++) queens[i] = 0;

    struct timespec startBasic, endBasic;
    clock_gettime(CLOCK_MONOTONIC, &startBasic);
    bool resultBasic = csp_problem_solve(problem, queens, NULL);
    clock_gettime(CLOCK_MONOTONIC, &endBasic);
    *timeBasic = timespec_to_ms(&startBasic, &endBasic);

    csp_problem_destroy(problem);
    free(queens);

    return resultFC && resultBasic;
}
int main(void) {
    unsigned int sizes[] = {4,8,16};
    size_t count = sizeof(sizes) / sizeof(sizes[0]);

    csp_init();

    FILE *f = fopen("resultats.csv", "w");
    if (!f) {
        perror("Erreur ouverture fichier CSV");
        csp_finish();
        return EXIT_FAILURE;
    }

    // Écrire l'en-tête CSV
    fprintf(f, "N,ForwardChecking(ms),BasicBacktracking(ms)\n");

    for (size_t i = 0; i < count; i++) {
        double timeFC = 0.0, timeBasic = 0.0;
        unsigned int n = sizes[i];
        printf("Benchmark pour N = %u\n", n);

        bool success = benchmark_solve_methods(n, &timeFC, &timeBasic);

        if (success) {
            printf("Solution trouvée pour %u reines\n", n);
            printf("Forward Checking : %.2f ms\n", timeFC);
            printf("Backtracking basique : %.2f ms\n", timeBasic);

            fprintf(f, "%u,%.2f,%.2f\n", n, timeFC, timeBasic);
        } else {
            printf("Aucune solution trouvée pour %u\n", n);
            fprintf(f, "%u,NA,NA\n", n);
        }
        printf("\n");
    }

    fclose(f);

    csp_finish();

    printf("Résultats enregistrés dans resultats.csv\n");

    return EXIT_SUCCESS;
}
