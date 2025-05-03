#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/csp.h"
#include "../src/sudoku_utils.h"

/**
 * Tests for the create_sudoku_problem function
 */
int main() {
    printf("Testing create_sudoku_problem function...\n");

    // Initialize the CSP library
    assert(csp_init() && "CSP initialization failed");

    // Create a sample Sudoku grid with some fixed values
    int grid[SUDOKU_CELLS] = {0};

    // Set a few pre-assigned values (representing a partial Sudoku puzzle)
    grid[0] = 5;   // Row 0, Col 0 = 5
    grid[1] = 3;   // Row 0, Col 1 = 3
    grid[4] = 7;   // Row 0, Col 4 = 7
    grid[9] = 6;   // Row 1, Col 0 = 6
    grid[80] = 9;  // Row 8, Col 8 = 9

    // Test 1: Create a problem with the sample grid
    CSPProblem *problem = create_sudoku_problem(grid);

    // Check if problem was created successfully
    assert(problem != NULL && "Problem creation should succeed");

    // Verify problem dimensions
    size_t num_variables = csp_problem_get_num_domains(problem);
    size_t num_constraints = csp_problem_get_num_constraints(problem);

    assert(num_variables == SUDOKU_CELLS && "Problem should have 81 variables");

    // Expected number of constraints:
    // - 81 unary constraints (one per cell)
    // - 36 binary constraints per row × 9 rows = 324 constraints
    // - 36 binary constraints per column × 9 columns = 324 constraints
    // - 36 binary constraints per 3×3 block × 9 blocks = 324 constraints
    // Total: 81 + 324 + 324 + 324 = 1053 constraints
    assert(num_constraints == 1053 && "Problem should have 1053 constraints");

    // Verify that all variables have domain size 9
    for (size_t var = 0; var < num_variables; var++) {
        size_t domain_size = csp_problem_get_domain(problem, var);
        assert(domain_size == SUDOKU_SIZE && "Each variable should have domain size 9");
    }

    // Check a few constraints
    // For each pre-assigned cell, verify its corresponding unary constraint

    // Extract the first constraint (unary constraint for cell 0,0)
    CSPConstraint const *first_constraint = csp_problem_get_constraint(problem, 0);
    assert(csp_constraint_get_arity(first_constraint) == 1 && "First constraint should be unary");
    assert(csp_constraint_get_variable(first_constraint, 0) == 0 &&
           "First constraint should apply to first variable");

    // Clean up
    // Don't destroy individual constraints here since csp_problem_destroy will handle that
    csp_problem_destroy(problem);
    printf("Valid problem creation test: PASSED\n");

    // Test 2: NULL grid parameter
    problem = create_sudoku_problem(NULL);
    assert(problem == NULL && "Creating problem with NULL grid should fail");
    printf("NULL grid test: PASSED\n");

    csp_finish();
    printf("All create_sudoku_problem tests PASSED!\n");
    return 0;
}
