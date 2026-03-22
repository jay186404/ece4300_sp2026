#include "SudokuSolver.h"
#include "xil_printf.h"

using namespace std;

int global_max_depth = 0; // To aid in benchmarking purposes

volatile unsigned long long dbg_ops = 0;
volatile int dbg_last_depth = 0;
static const unsigned long long REPORT_FREQ = 100000;

// Check if placing 'digit' at board[row][col] is valid
bool isValid(int board[9][9], int row, int col, int digit) {
    // Check row and column
    for (int i = 0; i < 9; i++) {
        if (board[row][i] == digit || board[i][col] == digit) {
            return false;
        }
    }

    // Check 3x3 sub-grid
    int startRow = row - row % 3;
    int startCol = col - col % 3;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i + startRow][j + startCol] == digit) {
                return false;
            }
        }
    }
    return true;
}

// The core Backtracking algorithm using missing_digits list
bool solve(int board[9][9], Cell emptyCells[81], int emptyCount, int index, long long& ops) {
    // Benchmark: Track the furthest we've reached in the puzzle
    if (index > global_max_depth) {
        global_max_depth = index;
    }

    // Base Case: If we've filled all empty cells, we are done
    if (index == emptyCount) {
        return true;
    }

    int row = emptyCells[index].row;
    int col = emptyCells[index].col;

    // Try digits 1-9
    for (int digit = 1; digit <= 9; digit++) {
        ops++; // Increment operation counter for benchmarking

        dbg_ops++;
        if ((dbg_ops % REPORT_FREQ) == 0) {
            xil_printf("Progress: ops=%llu, depth=%d\r\n",
               (unsigned long long)dbg_ops, global_max_depth);
        }


        if (isValid(board, row, col, digit)) {
            board[row][col] = digit; // Place digit

            // Move to the next mapped empty cell
            if (solve(board, emptyCells, emptyCount, index + 1, ops)) {
                return true;
            }

            // BACKTRACK: Reset the cell if the path fails
            board[row][col] = 0;
        }
    }
    return false;
}

void printBoard(int board[9][9]) {
    for (int i = 0; i < 9; i++) {
        // Print horizontal dividers every 3 rows
        if (i % 3 == 0 && i != 0) {
            xil_printf("------+-------+------\r\n");
        }

        for (int j = 0; j < 9; j++) {
            // Print vertical dividers every 3 columns
            if (j % 3 == 0 && j != 0) {
                xil_printf("| ");
            }

            // If value is 0, print '.', otherwise print the number
            // Added a space after each for clarity
            if (board[i][j] == 0) {
                xil_printf(". ");
            }
            else {
                xil_printf("%d ", board[i][j]);
            }
        }
        xil_printf("\r\n");
    }
}
/*
bool loadBoardFromFile(const string& filename, int board[9][9]) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return false;
    }

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (!(file >> board[i][j])) {
                cerr << "Error: Invalid data in " << filename << endl;
                file.close();
                return false;
            }
        }
    }
    file.close();
    return true;
}
*/