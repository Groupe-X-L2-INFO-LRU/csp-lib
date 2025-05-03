// sudoku_utils.h
#ifndef SUDOKU_UTILS_H_
#define SUDOKU_UTILS_H_

#include <stdbool.h>
#include <stdio.h>

#include "csp.h"

/**
 * @file sudoku_utils.h
 * @brief Utilities for modeling and solving Sudoku puzzles using the CSP framework
 * @ingroup applications
 *
 * This file provides utilities for handling Sudoku puzzles as constraint satisfaction problems.
 * It includes functions for reading puzzles from files, creating the corresponding CSP problems
 * with appropriate constraints, and printing solutions in a human-readable format.
 *
 * @section sudoku_modeling Sudoku as a CSP
 *
 * The Sudoku puzzle is modeled as a CSP with 81 variables (one for each cell), each with
 * domain {1,2,...,9}. The constraints ensure that:
 * 1. Each row contains all digits 1-9 without repetition
 * 2. Each column contains all digits 1-9 without repetition
 * 3. Each 3x3 block contains all digits 1-9 without repetition
 * 4. Pre-filled cells must keep their initial values
 *
 * @section sudoku_example Example Usage
 *
 * @code{.c}
 * #include "csp.h"
 * #include "forward-checking.h"
 * #include "sudoku_utils.h"
 * #include <stdio.h>
 * 
 * int main() {
 *     // Initialize the library
 *     csp_init();
 *     
 *     // Read a puzzle from a file
 *     FILE *file = fopen("puzzle.txt", "r");
 *     int grid[SUDOKU_SIZE * SUDOKU_SIZE] = {0};
 *     if (!read_sudoku_puzzle(file, grid)) {
 *         fprintf(stderr, "Failed to read puzzle\n");
 *         fclose(file);
 *         return 1;
 *     }
 *     fclose(file);
 *     
 *     // Create a CSP problem from the Sudoku puzzle
 *     CSPProblem *problem = create_sudoku_problem(grid);
 *     
 *     // Solve the puzzle
 *     size_t solution[SUDOKU_SIZE * SUDOKU_SIZE];
 *     bool solved = solve_with_forward_checking(problem, solution, NULL, NULL);
 *     
 *     // Print the solution
 *     if (solved) {
 *         print_sudoku_solution(solution);
 *     } else {
 *         printf("No solution found!\n");
 *     }
 *     
 *     // Clean up
 *     csp_problem_destroy(problem);
 *     csp_finish();
 *     
 *     return 0;
 * }
 * @endcode
 *
 * @author Quentin Sautiere
 * @date 3 mai 2025
 * @version 1.0
 */

/** @brief Number of rows and columns in a standard Sudoku grid */
#define SUDOKU_SIZE 9
/** @brief Total number of cells in a standard Sudoku grid */
#define SUDOKU_CELLS 81

/**
 * @brief Reads a Sudoku puzzle from the given input stream
 *
 * Parses a Sudoku puzzle from a text file where:
 * - Each row is on a separate line (9 lines total)
 * - Each cell is represented by a single character:
 *   - '1' through '9' for pre-filled cells
 *   - '.' or '0' for blank cells that need to be filled
 *
 * @param in File pointer to read from
 * @param initial_grid Array to store the parsed puzzle (0 for blank cells, 1-9 for filled)
 * @return true if parsing was successful, false on I/O error or invalid format
 *
 * @pre in != NULL
 * @pre initial_grid != NULL and can store at least SUDOKU_CELLS integers
 */
bool read_sudoku_puzzle(FILE *in, int initial_grid[SUDOKU_CELLS]);

/**
 * @brief Prints a solved Sudoku grid in a human-readable format
 *
 * The function prints the solved Sudoku grid to standard output using a
 * visually appealing format with:
 * - Horizontal and vertical separators between 3x3 blocks
 * - Cell values displayed as digits 1-9
 * - Clear row and column alignment
 *
 * Example output:
 * @verbatim
 * +-------+-------+-------+
 * | 5 3 4 | 6 7 8 | 9 1 2 |
 * | 6 7 2 | 1 9 5 | 3 4 8 |
 * | 1 9 8 | 3 4 2 | 5 6 7 |
 * +-------+-------+-------+
 * | 8 5 9 | 7 6 1 | 4 2 3 |
 * | 4 2 6 | 8 5 3 | 7 9 1 |
 * | 7 1 3 | 9 2 4 | 8 5 6 |
 * +-------+-------+-------+
 * | 9 6 1 | 5 3 7 | 2 8 4 |
 * | 2 8 7 | 4 1 9 | 6 3 5 |
 * | 3 4 5 | 2 8 6 | 1 7 9 |
 * +-------+-------+-------+
 * @endverbatim
 *
 * @param solution Array of assigned values (0-8 representing digits 1-9)
 * @note If solution is NULL, the function returns without doing anything
 */
void print_sudoku_solution(const size_t solution[SUDOKU_CELLS]);

/**
 * @brief Creates a CSP problem that models the Sudoku puzzle
 *
 * This function converts a Sudoku puzzle into a constraint satisfaction problem
 * by creating:
 * 1. 81 variables, one for each cell, with domains {0,1,...,8} (representing 1-9)
 * 2. 81 unary constraints for pre-filled cells (to enforce their initial values)
 * 3. 27 groups of binary constraints (one for each row, column, and 3x3 block)
 *    - Each group requires all cell values to be different
 *
 * The resulting CSP has 1053 constraints in total:
 * - 81 unary constraints for pre-filled cells
 * - 972 binary constraints for the all-different requirements
 *
 * @param initial_grid Array containing the initial puzzle (0 for blank cells, 1-9 for filled)
 * @return Pointer to a newly created CSP problem, or NULL on allocation failure
 *
 * @note The caller is responsible for destroying the problem with csp_problem_destroy()
 * @warning The function will return NULL if memory allocation fails
 */
CSPProblem *create_sudoku_problem(const int initial_grid[SUDOKU_CELLS]);

#endif  // SUDOKU_UTILS_H_
