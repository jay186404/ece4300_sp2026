#ifndef SUDOKUSOLVER_H
#define SUDOKUSOLVER_H

// Structure to hold coordinates of empty cells (Optimization to reduce redundant scanning for empty cells) 
struct Cell {
    int row;
    int col;
};

// Function prototypes
bool isValid(int board[9][9], int row, int col, int digit);
bool solve(int board[9][9], Cell emptyCells[81], int emptyCount, int index, long long& ops);
void printBoard(int board[9][9]);
//bool loadBoardFromFile(const std::string& filename, int board[9][9]);

// To aid in benchmarking purposes
extern int global_max_depth;
extern volatile unsigned long long dbg_ops;
extern volatile int dbg_last_depth;

#endif