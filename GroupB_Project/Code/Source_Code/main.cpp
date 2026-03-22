#include "SudokuSolver.h"
#include "SudokuProblems.h"
#include <cstring>
#include <stdio.h>

#include "xparameters.h"
#include "xtmrctr.h"
#include "xil_printf.h"
#include "xgpio.h"

static XTmrCtr Timer;
XTmrCtr_Config *TimerConfigPtr;

static XGpio Buttons;
XGpio_Config *ButtonsConfigPtr;

#define BTN_START 0x1
#define CLOCK XPAR_XTMRCTR_0_CLOCK_FREQUENCY

static void copy_board(int dst[9][9], const int src[9][9]) {
    memcpy(dst, src, sizeof(int) * 9 * 9);
}

static void delay() {
    for (volatile int i = 0; i < 500000; i++) {}
}

static void debounce(u32 mask) {
    while ((XGpio_DiscreteRead(&Buttons, 1) & mask) == 0) {}
    delay();
    while (XGpio_DiscreteRead(&Buttons, 1) & mask) {}
    delay();
}

u64 get_64bits_cycles(XTmrCtr *Instance) {
    u32 high, low;
    high = XTmrCtr_GetValue(Instance, 1);
    low = XTmrCtr_GetValue(Instance, 0);
    return ((u64)high << 32 | (u64)low);
}

int main() {
    // GPIO Configuration
    ButtonsConfigPtr = XGpio_LookupConfig(XPAR_XGPIO_0_BASEADDR);
    if (ButtonsConfigPtr == NULL) {
        xil_printf("GPIO config failed\r\n");
        while (1) {}
    }

    if (XGpio_CfgInitialize(&Buttons, ButtonsConfigPtr,
                            ButtonsConfigPtr->BaseAddress) != XST_SUCCESS) {
        xil_printf("GPIO init failed\r\n");
        while (1) {}
    }

    XGpio_SetDataDirection(&Buttons, 1, 0x3);

    // Timer Configuration
    TimerConfigPtr = XTmrCtr_LookupConfig(XPAR_XTMRCTR_0_BASEADDR);
    if (TimerConfigPtr == NULL) {
        xil_printf("Timer config failed\r\n");
        while (1) {}
    }

    XTmrCtr_CfgInitialize(&Timer, TimerConfigPtr, TimerConfigPtr->BaseAddress);
    XTmrCtr_SetOptions(&Timer, 0, XTC_CASCADE_MODE_OPTION);

    xil_printf("Sudoku Benchmark Ready\r\n");
    xil_printf("Press BTN_U to start\r\n");

    while (1) {
        // Sets up Sudoku board
        const int (*puzzle)[9] = medium_problem;
        static int board[9][9];
        copy_board(board, puzzle);

        Cell emptyCells[81];
        int emptyCount = 0;
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                if (board[i][j] == 0) {
                    emptyCells[emptyCount].row = i;
                    emptyCells[emptyCount].col = j;
                    emptyCount++;
                }
            }
        }

        xil_printf("\r\nInitial Board Layout:\r\n");
        printBoard(board);

        xil_printf("\r\nWaiting for start button (BTN_U)...\r\n");
        debounce(BTN_START);

        xil_printf("Start button pressed. Algorithm started.\r\n");

        long long operations = 0;
        global_max_depth = 0;

        /* Timer starts
        XTmrCtr_Reset(&Timer, 0);
        XTmrCtr_Start(&Timer, 0);
        */

        XTmrCtr_Reset(&Timer, 0);
        XTmrCtr_Reset(&Timer, 1);
        XTmrCtr_Start(&Timer, 0);
        XTmrCtr_Start(&Timer, 1);

        //u32 start = XTmrCtr_GetValue(&Timer, 0);
        u64 start = get_64bits_cycles(&Timer);

        bool solvedBoard = solve(board, emptyCells, emptyCount, 0, operations);

        xil_printf("Finished solving. \r\n");

        //u32 end = XTmrCtr_GetValue(&Timer, 0);
        u64 end = get_64bits_cycles(&Timer);

        XTmrCtr_Stop(&Timer, 0);
        XTmrCtr_Stop(&Timer, 1);

        //u32 ticks = end - start;
        u64 cycles = end - start;

        double seconds = (double)cycles / CLOCK;
        double throughput = (seconds > 0.0) ? ((double)operations / seconds) : 0.0;


        // Prints results
        xil_printf("\r\n Benchmark Results \r\n");
        if (solvedBoard) {
            xil_printf("Algorithm finished successfully\r\n");
            xil_printf("Solved Board:\r\n");
            printBoard(board);
            xil_printf("Total Operations: %u\r\n", (u32)operations);
            xil_printf("Max Depth Reached: %d / 81\r\n", global_max_depth);
            xil_printf("Clock Cycles: %llu\r\n", (unsigned long long)cycles);
            xil_printf("Total Time: %d.%06d sec\r\n",
                       (int)seconds,
                       (int)((seconds - (int)seconds) * 1000000.0));
            xil_printf("Throughput ops/s: %d\r\n", (int)throughput);
        } else {
            xil_printf("Algorithm finished, but puzzle could not be solved\r\n");
            xil_printf("Total Operations: %llu\r\n", (unsigned long long)operations);
            xil_printf("Max Depth Reached: %d / 81\r\n", global_max_depth);
            xil_printf("Clock Cycles: %llu\r\n", (unsigned long long)cycles);
            xil_printf("Total Time: %d.%06d sec\r\n",
                       (int)seconds,
                       (int)((seconds - (int)seconds) * 1000000.0));
        }

        xil_printf("\r\nRun complete. Press BTN_U to run again.\r\n");
    }

    return 0;
}