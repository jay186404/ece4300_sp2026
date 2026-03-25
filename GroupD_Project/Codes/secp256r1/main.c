#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xtmrctr.h"
#include "monocypher.h"
#include "xstatus.h"

/* Adjust if your macro name is different */
#define CPU_FREQ XPAR_CPU_CORE_CLOCK_FREQ_HZ

XTmrCtr Timer;

void print_cycles_time(char *label, u32 cycles, int status)
{
    u32 seconds = cycles / CPU_FREQ;
    u32 ms = (cycles % CPU_FREQ) * 1000 / CPU_FREQ;

    xil_printf(" %s: %lu cycles (%lu.%03lu s) (Status: %d)\r\n",
               label, cycles, seconds, ms, status);
}

void run_curve25519_benchmark(void)
{
    uint8_t alice_sk[32] = {0};
    uint8_t alice_pk[32];
    uint8_t bob_sk[32] = {0};
    uint8_t bob_pk[32];
    uint8_t shared1[32];
    uint8_t shared2[32];

    u32 start, end, cycles;
    int verify_status;

    /* Simple test keys */
    alice_sk[0] = 1;
    bob_sk[0]   = 2;

    xil_printf("\r\n--- Benchmarking Curve25519 ---\r\n");

    XTmrCtr_Start(&Timer, 0);

    /* KeyGen (Alice public key) */
    start = XTmrCtr_GetValue(&Timer, 0);
    crypto_x25519_public_key(alice_pk, alice_sk);
    end = XTmrCtr_GetValue(&Timer, 0);
    cycles = end - start;
    print_cycles_time("KeyGen", cycles, 1);

    /* Generate Bob public key */
    crypto_x25519_public_key(bob_pk, bob_sk);

    /* Signing (Alice shared secret) */
    start = XTmrCtr_GetValue(&Timer, 0);
    crypto_x25519(shared1, alice_sk, bob_pk);
    end = XTmrCtr_GetValue(&Timer, 0);
    cycles = end - start;
    print_cycles_time("Signing", cycles, 1);

    /* Verify (Bob shared secret) */
    start = XTmrCtr_GetValue(&Timer, 0);
    crypto_x25519(shared2, bob_sk, alice_pk);
    end = XTmrCtr_GetValue(&Timer, 0);
    cycles = end - start;

    verify_status = (memcmp(shared1, shared2, 32) == 0) ? 1 : 0;
    print_cycles_time("Verify", cycles, verify_status);
}

int main(void)
{
    int status;

    init_platform();

    status = XTmrCtr_Initialize(&Timer, XPAR_TMRCTR_0_DEVICE_ID);
    if (status != XST_SUCCESS) {
        xil_printf("Timer init failed!\r\n");
        cleanup_platform();
        return -1;
    }

    XTmrCtr_SetOptions(&Timer, 0, XTC_AUTO_RELOAD_OPTION);

    xil_printf("=== Nexys A7 ECC Full Suite Benchmark ===\r\n");

    run_curve25519_benchmark();

    xil_printf("\r\nDone!\r\n");

    cleanup_platform();
    return 0;
}
