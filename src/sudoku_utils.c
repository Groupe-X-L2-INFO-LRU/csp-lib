#include "sudoku_utils.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/**
 * Checker function for Sudoku unary constraints (pre-assigned values)
 *
 * This function is used to ensure that values pre-assigned in the original
 * Sudoku grid are respected in the solution. When the `data` pointer is NULL,
 * we simply return `true` since there are no pre-assigned values to enforce.
 */
static bool sudoku_preassigned_checker(const CSPConstraint *constraint, const size_t *values,
                                       const void *data) {
    // If no grid data is provided, we don't have any constraints to enforce
    if (!data) {
        return true;
    }

    // The grid is passed as data parameter
    const int *initial_grid = (const int *)data;

    // Get the variable this constraint applies to
    size_t var = csp_constraint_get_variable(constraint, 0);
    int fixed = initial_grid[var];

    // No pre-assigned value (blank)
    if (fixed == 0) {
        return true;
    }

    // Check if the assigned value matches the pre-assigned value
    // Note: puzzle digits are 1-9, solver values are 0-8
    return values[var] == (size_t)(fixed - 1);
}

/**
 * Checker function for Sudoku not-equal constraints (binary constraints)
 */
static bool sudoku_not_equal_checker(const CSPConstraint *constraint, const size_t *values,
                                     const void *data) {
    size_t var1 = csp_constraint_get_variable(constraint, 0);
    size_t var2 = csp_constraint_get_variable(constraint, 1);
    return values[var1] != values[var2];
}

bool read_sudoku_puzzle(FILE *in, int initial_grid[SUDOKU_CELLS]) {
    if (!in || !initial_grid) return false;

    char line[SUDOKU_SIZE * 2];                           // Extra space for newlines, etc.
    memset(initial_grid, 0, SUDOKU_CELLS * sizeof(int));  // Initialize to all 0s (blank)

    // Read 9 rows
    for (size_t row = 0; row < SUDOKU_SIZE; row++) {
        if (!fgets(line, sizeof(line), in)) {
            return false;  // Failed to read line
        }

        // Process each character in the line
        for (size_t col = 0; col < SUDOKU_SIZE; col++) {
            char c = line[col];
            if (c >= '1' && c <= '9') {
                // Convert digit character to integer (1..9)
                initial_grid[row * SUDOKU_SIZE + col] = c - '0';
            } else if (c == '.' || c == '0') {
                // Blank cell - already initialized to 0
                continue;
            } else if (!isspace(c)) {
                // Invalid character
                return false;
            }
        }
    }

    return true;
}

void print_sudoku_solution(const size_t solution[SUDOKU_CELLS]) {
    if (!solution) return;

    // Print the formatted Sudoku grid with dividers
    for (size_t row = 0; row < SUDOKU_SIZE; row++) {
        if (row % 3 == 0) {
            printf("+-------+-------+-------+\n");
        }

        for (size_t col = 0; col < SUDOKU_SIZE; col++) {
            if (col % 3 == 0) {
                printf("| ");
            }

            // Convert 0-based values to 1-based digits for display
            size_t value = solution[row * SUDOKU_SIZE + col];
            printf("%zu ", value + 1);
        }

        printf("|\n");
    }

    printf("+-------+-------+-------+\n");
}

CSPProblem *create_sudoku_problem(const int initial_grid[SUDOKU_CELLS]) {
    if (!initial_grid) return NULL;

    // Compute the total number of constraints for a Sudoku grid:
    // 1) Unary constraints for pre-assigned cells: SUDOKU_CELLS (81)
    // 2) Binary constraints (not-equal) for rows, columns, and 3x3 blocks:
    //    Each unit (row, column, block) has SUDOKU_SIZE*(SUDOKU_SIZE-1)/2 pairs
    //    There are 3*SUDOKU_SIZE units (9 rows, 9 columns, 9 blocks)
    size_t pairs_per_unit = (SUDOKU_SIZE * (SUDOKU_SIZE - 1)) / 2;  // 36 pairs per unit
    size_t total_binary = pairs_per_unit * (3 * SUDOKU_SIZE);  // 36 * 27 = 972 binary constraints
    size_t total_constraints = total_binary + SUDOKU_CELLS;    // 972 + 81 = 1053 constraints

    // Create CSP problem with 81 variables and all constraints
    CSPProblem *problem = csp_problem_create(SUDOKU_CELLS, total_constraints);
    if (!problem) return NULL;

    // Set domains for all variables (cells): values 0..8 (representing digits 1..9)
    for (size_t var = 0; var < SUDOKU_CELLS; var++) {
        csp_problem_set_domain(problem, var, SUDOKU_SIZE);
    }

    size_t constraint_index = 0;

    // 1) Create unary constraints for pre-assigned values
    for (size_t var = 0; var < SUDOKU_CELLS; var++) {
        CSPConstraint *constraint = csp_constraint_create(1, sudoku_preassigned_checker);
        if (!constraint) {
            // We no longer need to manually free each constraint
            // since we've modified csp_problem_destroy to handle that
            csp_problem_destroy(problem);
            return NULL;
        }

        csp_constraint_set_variable(constraint, 0, var);
        csp_problem_set_constraint(problem, constraint_index++, constraint);
    }

    // 2) Create binary constraints for rows (all cells in same row must have different values)
    for (size_t row = 0; row < SUDOKU_SIZE; row++) {
        for (size_t col1 = 0; col1 < SUDOKU_SIZE - 1; col1++) {
            for (size_t col2 = col1 + 1; col2 < SUDOKU_SIZE; col2++) {
                size_t var1 = row * SUDOKU_SIZE + col1;
                size_t var2 = row * SUDOKU_SIZE + col2;

                CSPConstraint *constraint = csp_constraint_create(2, sudoku_not_equal_checker);
                if (!constraint) {
                    csp_problem_destroy(problem);
                    return NULL;
                }

                csp_constraint_set_variable(constraint, 0, var1);
                csp_constraint_set_variable(constraint, 1, var2);
                csp_problem_set_constraint(problem, constraint_index++, constraint);
            }
        }
    }

    // 3) Create binary constraints for columns (all cells in same column must have different
    // values)
    for (size_t col = 0; col < SUDOKU_SIZE; col++) {
        for (size_t row1 = 0; row1 < SUDOKU_SIZE - 1; row1++) {
            for (size_t row2 = row1 + 1; row2 < SUDOKU_SIZE; row2++) {
                size_t var1 = row1 * SUDOKU_SIZE + col;
                size_t var2 = row2 * SUDOKU_SIZE + col;

                CSPConstraint *constraint = csp_constraint_create(2, sudoku_not_equal_checker);
                if (!constraint) {
                    csp_problem_destroy(problem);
                    return NULL;
                }

                csp_constraint_set_variable(constraint, 0, var1);
                csp_constraint_set_variable(constraint, 1, var2);
                csp_problem_set_constraint(problem, constraint_index++, constraint);
            }
        }
    }

    // 4) Create binary constraints for 3x3 blocks (all cells in same block must have different
    // values)
    for (size_t block_row = 0; block_row < 3; block_row++) {
        for (size_t block_col = 0; block_col < 3; block_col++) {
            size_t block_vars[SUDOKU_SIZE];
            size_t index = 0;

            // Collect all cell indices in this 3x3 block
            for (size_t r = 0; r < 3; r++) {
                for (size_t c = 0; c < 3; c++) {
                    size_t row = block_row * 3 + r;
                    size_t col = block_col * 3 + c;
                    block_vars[index++] = row * SUDOKU_SIZE + col;
                }
            }

            // Create constraints between all pairs of cells in this block
            for (size_t i = 0; i < SUDOKU_SIZE - 1; i++) {
                for (size_t j = i + 1; j < SUDOKU_SIZE; j++) {
                    CSPConstraint *constraint = csp_constraint_create(2, sudoku_not_equal_checker);
                    if (!constraint) {
                        // We no longer need to manually free each constraint
                        // since we've modified csp_problem_destroy to handle that
                        csp_problem_destroy(problem);
                        return NULL;
                    }

                    csp_constraint_set_variable(constraint, 0, block_vars[i]);
                    csp_constraint_set_variable(constraint, 1, block_vars[j]);
                    csp_problem_set_constraint(problem, constraint_index++, constraint);
                }
            }
        }
    }

    return problem;
}
