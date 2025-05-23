/**
    * solve-sudoku.c
    *
    * @file solve-sudoku.c
    * @brief Sudoku solver using CSP with forward checking and heuristics.
    * @author Quentin Sautière
    * @date 2025
    This program models the Sudoku problem as a Constraint Satisfaction Problem (CSP) using the CSP
library. It uses forward checking with MRV and LCV heuristics to efficiently solve the puzzle. The
Sudoku grid is represented as 81 variables with domains 1..9 and all-different constraints on rows,
columns, and 3x3 blocks.
    *
*/

//
// This program models the Sudoku problem as a Constraint Satisfaction Problem (CSP)
// using the CSP library. It uses forward checking with MRV and LCV heuristics
// to efficiently solve the puzzle. The Sudoku grid is represented as 81 variables
// with domains 1..9 and all-different constraints on rows, columns, and 3x3 blocks.

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "csp.h"
#include "forward-checking.h"

// Size definitions for Sudoku
#define SUDOKU_SIZE 9    // Number of rows and columns
#define SUDOKU_CELLS 81  // Total number of cells

// Holds the initial puzzle values: 0 for blank, 1..9 for pre-filled cells
static int initial_grid[SUDOKU_CELLS];

/**
 * @brief Checker to enforce a cell's pre-assigned value.
 *
 * For a unary constraint, ensures that the assigned value matches the initial puzzle.
 * If the cell was blank in the puzzle (initial_grid[var] == 0), this constraint always passes.
 *
 * @param constraint The unary constraint object.
 * @param values Current assignment to all variables.
 * @param data Unused (NULL).
 * @return true if cell value matches the pre-assigned value or if blank, false otherwise.
 */
bool sudoku_preassigned(const CSPConstraint *constraint, const size_t *values, const void *data) {
    // Determine which variable this constraint applies to
    size_t var = csp_constraint_get_variable(constraint, 0);
    int fixed = initial_grid[var];
    if (fixed == 0) {
        // No pre-assigned value, always satisfied
        return true;
    }
    // Puzzle digits are 1..9, solver values are 0..8, so compare with fixed-1
    return values[var] == (size_t)(fixed - 1);
}

/**
 * @brief Checker for binary all-different constraint.
 *
 * Ensures that two variables do not have the same value.
 *
 * @param constraint The binary constraint object.
 * @param values Current assignment to all variables.
 * @param data Unused (NULL).
 * @return true if values differ, false if equal.
 */
bool sudoku_not_equal(const CSPConstraint *constraint, const size_t *values, const void *data) {
    size_t a = csp_constraint_get_variable(constraint, 0);
    size_t b = csp_constraint_get_variable(constraint, 1);
    return values[a] != values[b];
}

/**
 * @brief Prints the solved Sudoku grid in a human-readable format.
 */
void print_sudoku(const size_t *solution) {
    for (size_t i = 0; i < SUDOKU_SIZE; i++) {
        if (i % 3 == 0) printf("+-------+-------+-------+\n");
        for (size_t j = 0; j < SUDOKU_SIZE; j++) {
            if (j % 3 == 0) printf("| ");
            // solver values are 0..8 => print 1..9
            printf("%zu ", solution[i * SUDOKU_SIZE + j] + 1);
        }
        printf("|\n");
    }
    printf("+-------+-------+-------+\n");
}

/**
 * @brief Reads a Sudoku puzzle from stdin or file: 9 lines of 9 digits or dots.
 *
 * Dot ('.') or '0' indicates blank. Digits '1'..'9' indicate pre-filled cells.
 */
bool read_puzzle(FILE *in) {
    char line[32];
    for (size_t i = 0; i < SUDOKU_SIZE; i++) {
        if (!fgets(line, sizeof(line), in)) return false;
        for (size_t j = 0; j < SUDOKU_SIZE; j++) {
            char c = line[j];
            if (c >= '1' && c <= '9') {
                initial_grid[i * SUDOKU_SIZE + j] = c - '0';
            } else {
                // blank
                initial_grid[i * SUDOKU_SIZE + j] = 0;
            }
        }
    }
    return true;
}

int main(int argc, char *argv[]) {
    FILE *in = stdin;
    bool using_file = false;

    if (argc == 2) {
        in = fopen(argv[1], "r");
        if (!in) {
            // Using snprintf for safer string formatting
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "Cannot open file: %s\n", argv[1]);
            fputs(error_msg, stderr);
            return EXIT_FAILURE;
        }
        using_file = true;
    }

    // Read the puzzle
    bool read_success = read_puzzle(in);

    // Close file if we opened one
    if (using_file) {
        fclose(in);
    }

    if (!read_success) {
        fputs("Failed to read puzzle\n", stderr);
        return EXIT_FAILURE;
    }

    // Initialize CSP library
    csp_init();

    // Pre-allocate an array for the solution values (0..8)
    // and set initial assignments for pre-filled cells
    size_t solution[SUDOKU_CELLS] = {0};
    for (size_t v = 0; v < SUDOKU_CELLS; ++v) {
        if (initial_grid[v] != 0) {
            // Fixed value in puzzle: store zero-based index
            solution[v] = (size_t)(initial_grid[v] - 1);
        }
    }

    // Compute the total number of binary constraints for rows, cols, blocks
    size_t pairs_per_unit = (SUDOKU_SIZE * (SUDOKU_SIZE - 1)) / 2;
    size_t total_binary = pairs_per_unit * (3 * SUDOKU_SIZE);
    // Add unary constraints for each cell (pre-assignment)
    size_t total_constraints = total_binary + SUDOKU_CELLS;

    // Create CSP problem: 81 variables, total_constraints constraints
    CSPProblem *problem = csp_problem_create(SUDOKU_CELLS, total_constraints);

    // Set domains: each cell can take values 0..8 (i.e. digits 1..9)
    for (size_t v = 0; v < SUDOKU_CELLS; v++) {
        csp_problem_set_domain(problem, v, SUDOKU_SIZE);
    }

    size_t ci = 0;
    // 1) Unary constraints for pre-assigned cells
    for (size_t v = 0; v < SUDOKU_CELLS; v++) {
        CSPConstraint *c = csp_constraint_create(1, (CSPChecker *)sudoku_preassigned);
        csp_constraint_set_variable(c, 0, v);
        csp_problem_set_constraint(problem, ci++, c);
    }

    // 2) All-different constraints for rows
    for (size_t r = 0; r < SUDOKU_SIZE; r++) {
        for (size_t a = 0; a < SUDOKU_SIZE - 1; a++) {
            for (size_t b = a + 1; b < SUDOKU_SIZE; b++) {
                size_t v1 = r * SUDOKU_SIZE + a;
                size_t v2 = r * SUDOKU_SIZE + b;
                CSPConstraint *c = csp_constraint_create(2, (CSPChecker *)sudoku_not_equal);
                csp_constraint_set_variable(c, 0, v1);
                csp_constraint_set_variable(c, 1, v2);
                csp_problem_set_constraint(problem, ci++, c);
            }
        }
    }
    // 3) All-different constraints for columns
    for (size_t cidx = 0; cidx < SUDOKU_SIZE; cidx++) {
        for (size_t a = 0; a < SUDOKU_SIZE - 1; a++) {
            for (size_t b = a + 1; b < SUDOKU_SIZE; b++) {
                size_t v1 = a * SUDOKU_SIZE + cidx;
                size_t v2 = b * SUDOKU_SIZE + cidx;
                CSPConstraint *c = csp_constraint_create(2, (CSPChecker *)sudoku_not_equal);
                csp_constraint_set_variable(c, 0, v1);
                csp_constraint_set_variable(c, 1, v2);
                csp_problem_set_constraint(problem, ci++, c);
            }
        }
    }
    // 4) All-different constraints for 3x3 blocks
    for (size_t block = 0; block < SUDOKU_SIZE; block++) {
        size_t br = (block / 3) * 3;
        size_t bc = (block % 3) * 3;
        // Collect cell indices in this block
        size_t idxs[9];
        size_t k = 0;
        for (size_t dr = 0; dr < 3; dr++) {
            for (size_t dc = 0; dc < 3; dc++) {
                idxs[k++] = (br + dr) * SUDOKU_SIZE + (bc + dc);
            }
        }
        // Add pairwise constraints within block
        for (size_t a = 0; a < 9 - 1; a++) {
            for (size_t b = a + 1; b < 9; b++) {
                CSPConstraint *c = csp_constraint_create(2, (CSPChecker *)sudoku_not_equal);
                csp_constraint_set_variable(c, 0, idxs[a]);
                csp_constraint_set_variable(c, 1, idxs[b]);
                csp_problem_set_constraint(problem, ci++, c);
            }
        }
    }

    // Solve the Sudoku with forward checking and heuristics
    bool solved = csp_problem_solve_forward_checking(problem, solution, NULL);

    // Cleanup: no need to manually destroy constraints - csp_problem_destroy will handle that
    csp_problem_destroy(problem);

    // Print result
    if (solved) {
        print_sudoku(solution);
    } else {
        printf("No solution found\n");
    }

    csp_finish();
    return EXIT_SUCCESS;
}
