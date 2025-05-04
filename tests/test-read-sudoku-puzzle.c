#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/sudoku_utils.h"

/**
 * Tests for the read_sudoku_puzzle function
 */
int main() {
    printf("Testing read_sudoku_puzzle function...\n");

    // Test 1: Valid Sudoku puzzle
    const char *valid_puzzle =
        "53..7....\n"
        "6..195...\n"
        ".98....6.\n"
        "8...6...3\n"
        "4..8.3..1\n"
        "7...2...6\n"
        ".6....28.\n"
        "...419..5\n"
        "....8..79\n";

    FILE *valid_file = tmpfile();
    assert(valid_file && "Failed to create temporary file for valid puzzle");
    fputs(valid_puzzle, valid_file);
    rewind(valid_file);

    int grid[SUDOKU_CELLS] = {0};
    bool success = read_sudoku_puzzle(valid_file, grid);

    // Check if reading was successful
    assert(success && "Reading valid puzzle should succeed");

    // Verify specific values
    assert(grid[0] == 5 && "First cell should be 5");
    assert(grid[1] == 3 && "Second cell should be 3");
    assert(grid[4] == 7 && "Fifth cell should be 7");
    assert(grid[9] == 6 && "First cell of second row should be 6");
    assert(grid[80] == 9 && "Last cell should be 9");

    fclose(valid_file);
    printf("Valid puzzle test: PASSED\n");

    // Test 2: Invalid character in puzzle
    const char *invalid_puzzle =
        "53..7....\n"
        "6..195...\n"
        ".98....6.\n"
        "8...6...3\n"
        "4..8.3..1\n"
        "7...2...6\n"
        ".6....28.\n"
        "...419..5\n"
        "....8..X9\n";  // 'X' is invalid

    FILE *invalid_file = tmpfile();
    assert(invalid_file && "Failed to create temporary file for invalid puzzle");
    fputs(invalid_puzzle, invalid_file);
    rewind(invalid_file);

    memset(grid, 0, sizeof(grid));
    success = read_sudoku_puzzle(invalid_file, grid);

    assert(!success && "Reading invalid puzzle should fail");
    fclose(invalid_file);
    printf("Invalid puzzle test: PASSED\n");

    // Test 3: Incomplete puzzle (too few rows)
    const char *incomplete_puzzle =
        "53..7....\n"
        "6..195...\n"
        ".98....6.\n";  // only 3 rows

    FILE *incomplete_file = tmpfile();
    assert(incomplete_file && "Failed to create temporary file for incomplete puzzle");
    fputs(incomplete_puzzle, incomplete_file);
    rewind(incomplete_file);

    memset(grid, 0, sizeof(grid));
    success = read_sudoku_puzzle(incomplete_file, grid);

    assert(!success && "Reading incomplete puzzle should fail");
    fclose(incomplete_file);
    printf("Incomplete puzzle test: PASSED\n");

    // Test 4: NULL file pointer
    memset(grid, 0, sizeof(grid));
    success = read_sudoku_puzzle(NULL, grid);

    assert(!success && "Reading from NULL file should fail");
    printf("NULL file test: PASSED\n");

    // Test 5: NULL grid pointer
    FILE *test_file = tmpfile();
    if (test_file) {
        fputs(valid_puzzle, test_file);
        rewind(test_file);
    }

    success = read_sudoku_puzzle(test_file, NULL);

    assert(!success && "Reading into NULL grid should fail");
    if (test_file) {
        fclose(test_file);
    }
    printf("NULL grid test: PASSED\n");

    printf("All read_sudoku_puzzle tests PASSED!\n");
    return 0;
}
