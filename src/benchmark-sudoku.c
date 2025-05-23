//
// Created by gwenh on 23/05/2025.
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "csp.h"
#include "forward-checking.h"

/**  @author Gwendal Henry */

// Vérifie que deux cases du Sudoku n'ont pas la même valeur (différent)
// La contrainte est binaire sur 2 variables (cases)
bool sudoku_diff_constraint(CSPConstraint *constraint, const size_t *values, unsigned int *data) {
    size_t v1 = csp_constraint_get_variable(constraint, 0);
    size_t v2 = csp_constraint_get_variable(constraint, 1);
    size_t val1 = values[v1];
    size_t val2 = values[v2];

    // Si une case n'est pas encore assignée (val=0), la contrainte est vraie
    if (val1 == 0 || val2 == 0) return true;

    return val1 != val2;
}

// Retourne l'indice de la case dans la grille 9x9
static inline size_t cell_index(size_t row, size_t col) {
    return row * 9 + col;
}

// Ajoute toutes les contraintes entre cases qui doivent être différentes
void add_sudoku_constraints(CSPProblem *problem) {
    size_t constraint_index = 0;

    for (size_t i = 0; i < 81; i++) {
        size_t row_i = i / 9;
        size_t col_i = i % 9;
        size_t block_i = (row_i / 3) * 3 + (col_i / 3);

        for (size_t j = i + 1; j < 81; j++) {
            size_t row_j = j / 9;
            size_t col_j = j % 9;
            size_t block_j = (row_j / 3) * 3 + (col_j / 3);

            // Même ligne ou même colonne ou même bloc 3x3 => contrainte différence
            if (row_i == row_j || col_i == col_j || block_i == block_j) {
                CSPConstraint *constraint = csp_constraint_create(2, (CSPChecker *)sudoku_diff_constraint);
                csp_constraint_set_variable(constraint, 0, i);
                csp_constraint_set_variable(constraint, 1, j);
                csp_problem_set_constraint(problem, constraint_index, constraint);
                constraint_index++;
            }
        }
    }
}

static double timespec_to_ms(const struct timespec *start, const struct timespec *end) {
    return (end->tv_sec - start->tv_sec) * 1000.0 + (end->tv_nsec - start->tv_nsec) / 1e6;
}

// Remplit le domaine des variables : 1..9
void init_domains(CSPProblem *problem) {
    for (size_t i = 0; i < 81; i++) {
        csp_problem_set_domain(problem, i, 9);
    }
}

// Initialise la solution partielle avec les valeurs initiales du sudoku (0=vide)
void init_solution(const size_t *initial_grid, size_t *solution) {
    for (size_t i = 0; i < 81; i++) {
        solution[i] = initial_grid[i];
    }
}

bool benchmark_sudoku_solver(const size_t *initial_grid, double *timeFC, double *timeBasic) {
    size_t *solution = calloc(81, sizeof(size_t));
    if (!solution) return false;

    const size_t nb_constraints = 810; // Nombre approximatif de contraintes pour Sudoku

    CSPProblem *problem = csp_problem_create(81, nb_constraints);
    if (!problem) {
        free(solution);
        return false;
    }

    init_domains(problem);
    add_sudoku_constraints(problem);
    init_solution(initial_grid, solution);

    struct timespec startFC, endFC;
    clock_gettime(CLOCK_MONOTONIC, &startFC);
    bool solvedFC = csp_problem_solve_forward_checking(problem, solution, NULL);
    clock_gettime(CLOCK_MONOTONIC, &endFC);
    *timeFC = timespec_to_ms(&startFC, &endFC);

    // Reset solution partielle pour le backtracking basique
    init_solution(initial_grid, solution);

    struct timespec startBasic, endBasic;
    clock_gettime(CLOCK_MONOTONIC, &startBasic);
    bool solvedBasic = csp_problem_solve(problem, solution, NULL);
    clock_gettime(CLOCK_MONOTONIC, &endBasic);
    *timeBasic = timespec_to_ms(&startBasic, &endBasic);

    csp_problem_destroy(problem);
    free(solution);

    return solvedFC && solvedBasic;
}

void print_grid(const size_t *grid) {
    for (size_t r = 0; r < 9; r++) {
        for (size_t c = 0; c < 9; c++) {
            printf("%zu ", grid[cell_index(r, c)]);
        }
        printf("\n");
    }
}

int main(void) {
    size_t sudoku_grid[81];

    csp_init();

    const char *fichiers[] = { "../puzzles/easy.txt","../puzzles/medium.txt", "../puzzles/hard.txt" };
    const char *resultats_file = "resultats_sudoku.csv";

    // Ecrire l'entête CSV (en mode écriture, écrase le fichier s'il existe)
    FILE *f = fopen(resultats_file, "w");
    if (!f) {
        perror("Erreur ouverture fichier CSV");
        return 1;
    }
    fprintf(f, "Probleme,Resolu,Temps_ForwardChecking_ms,Temps_Backtracking_ms\n");
    fclose(f);

    for (int i = 0; i < 3; i++) {

        double timeFC = 0.0, timeBasic = 0.0;
        bool success = benchmark_sudoku_solver(sudoku_grid, &timeFC, &timeBasic);

        if (success) {
            printf("Solution trouvée pour %s.\n", fichiers[i]);
            printf("Forward Checking : %.2f ms\n", timeFC);
            printf("Backtracking basique : %.2f ms\n", timeBasic);
        } else {
            printf("Aucune solution trouvée pour %s.\n", fichiers[i]);
        }
        printf("\n");

        // Ouvre le fichier CSV en mode ajout
        f = fopen(resultats_file, "a");
        if (!f) {
            perror("Erreur ouverture fichier CSV");
            return 1;
        }
        fprintf(f, "%s,%s,%.2f,%.2f\n",
                fichiers[i],
                success ? "Oui" : "Non",
                timeFC,
                timeBasic);
        fclose(f);
    }

    csp_finish();

    return 0;
}
