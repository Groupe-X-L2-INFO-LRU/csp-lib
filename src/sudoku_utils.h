// sudoku_utils.h
#ifndef SUDOKU_UTILS_H_
#define SUDOKU_UTILS_H_

#include <stdio.h>
#include <stdbool.h>
#include "csp.h"

#define SUDOKU_SIZE 9       // Rows and columns
#define SUDOKU_CELLS 81     // Total cells

/**
 * Reads a Sudoku puzzle from the given input stream.
 * Expect 9 lines of 9 characters: '1'..'9' or '.' for blank.
 * Stores digits 1..9 in grid, and 0 for blanks.
 * Returns true on success, false on I/O errors or invalid format.
 */
bool read_sudoku_puzzle(FILE *in, int initial_grid[SUDOKU_CELLS]);

/**
 * Prints a solved Sudoku grid in human-readable format.
 * solution values are 0..8 corresponding to digits 1..9.
 */
void print_sudoku_solution(const size_t solution[SUDOKU_CELLS]);

/**
 * Creates a CSPProblem modeling the Sudoku puzzle with given initial grid.
 * Returns a new CSPProblem pointer or NULL on allocation failure.
 * The caller is responsible for destroying the problem and its constraints.
 */
CSPProblem *create_sudoku_problem(const int initial_grid[SUDOKU_CELLS]);

#endif  // SUDOKU_UTILS_H_
