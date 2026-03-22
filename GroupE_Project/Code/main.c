#include <stdio.h>
#include "xil_printf.h"
#include "xtmrctr.h"     // AXI Timer Driver
#include "xparameters.h" // Hardware addresses
#include "aes.h"         // Your AES library

XTmrCtr TimerInstance;

int main() {
    // 1. Initialize AXI Timer
    XTmrCtr_Initialize(&TimerInstance, XPAR_AXI_TIMER_0_BASEADDR);
    XTmrCtr_SetOptions(&TimerInstance, 0, XTC_AUTO_RELOAD_OPTION);

    // 2. Setup AES Data
    struct AES_ctx ctx;
    // For AES-256
    uint8_t key[24] = { 
    0x8e, 0x73, 0xb0, 0xf7, 0xda, 0x0e, 0x64, 0x52, 
    0xc8, 0x10, 0xf3, 0x2b, 0x80, 0x90, 0x79, 0xe5, 
    0x62, 0xf8, 0xea, 0xd2, 0x52, 0x2c, 0x6b, 0x7b };
    uint8_t iv[16]  = { (uint8_t) 0x00, (uint8_t) 0x01, (uint8_t) 0x02, (uint8_t) 0x03, (uint8_t) 0x04, (uint8_t) 0x05, (uint8_t) 0x06, (uint8_t) 0x07, (uint8_t) 0x08, (uint8_t) 0x09, (uint8_t) 0x0a, (uint8_t) 0x0b, (uint8_t) 0x0c, (uint8_t) 0x0d, (uint8_t) 0x0e, (uint8_t) 0x0f };
    uint8_t in[16]  = { (uint8_t) 0x6b, (uint8_t) 0xc1, (uint8_t) 0xbe, (uint8_t) 0xe2, (uint8_t) 0x2e, (uint8_t) 0x40, (uint8_t) 0x9f, (uint8_t) 0x96, (uint8_t) 0xe9, (uint8_t) 0x3d, (uint8_t) 0x7e, (uint8_t) 0x11, (uint8_t) 0x73, (uint8_t) 0x93, (uint8_t) 0x17, (uint8_t) 0x2a };

    xil_printf("--- Starting AES-192 Benchmark ---\r\n");

    // 3. Start Timer and Benchmark
    XTmrCtr_Start(&TimerInstance, 0);
    uint32_t start_cycles = XTmrCtr_GetValue(&TimerInstance, 0);

    // Perform Encryption
    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_encrypt_buffer(&ctx, in, 16);

    uint32_t end_cycles = XTmrCtr_GetValue(&TimerInstance, 0);
    XTmrCtr_Stop(&TimerInstance, 0);

    // 4. Report Results
    uint32_t duration = end_cycles - start_cycles;
    xil_printf("Encryption Complete!\r\n");
    xil_printf("Clock Cycles: %u\r\n", duration);
    xil_printf("Note: At 100MHz, 1 cycle = 10ns\r\n");

    return 0;
}