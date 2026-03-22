#include <stdio.h>
#include "xparameters.h"
#include "xtmrctr.h"
#include "uECC.h"
#include "xil_printf.h"

typedef unsigned long u32;

XTmrCtr Timer;

// Dummy Random Number Generator for Benchmarking
int dummy_rng(uint8_t *dest, unsigned size) {
    for (unsigned i = 0; i < size; ++i) {
        dest[i] = i; 
    }
    return 1; 
}

void run_benchmark(const struct uECC_Curve_t * curve, char* name) {
    uint8_t public[64], private[32], hash[32] = {1}, sig[128];
    u32 start, end;
    int status;

    xil_printf("\r\n--- Benchmarking %s ---\r\n", name);

    // 1. Key Generation
    XTmrCtr_Start(&Timer, 0);
    start = XTmrCtr_GetValue(&Timer, 0);
    status = uECC_make_key(public, private, curve);
    end = XTmrCtr_GetValue(&Timer, 0);
    xil_printf("  KeyGen: %lu cycles (Status: %d)\r\n", end - start, status);

    // 2. Signing
    start = XTmrCtr_GetValue(&Timer, 0);
    status = uECC_sign(private, hash, 32, sig, curve);
    end = XTmrCtr_GetValue(&Timer, 0);
    xil_printf("  Signing: %lu cycles (Status: %d)\r\n", end - start, status);

    // 3. Verification
    start = XTmrCtr_GetValue(&Timer, 0);
    status = uECC_verify(public, hash, 32, sig, curve);
    end = XTmrCtr_GetValue(&Timer, 0);
    xil_printf("  Verify: %lu cycles (Status: %d)\r\n", end - start, status);
}

int main() {
    XTmrCtr_Initialize(&Timer, XPAR_XTMRCTR_0_BASEADDR);
    XTmrCtr_SetOptions(&Timer, 0, XTC_AUTO_RELOAD_OPTION);
    
    uECC_set_rng(&dummy_rng);

    xil_printf("\r\n=== Nexys A7 ECC Full Suite Benchmark ===\r\n");

    run_benchmark(uECC_secp160r1(), "160-bit");
    run_benchmark(uECC_secp192r1(), "192-bit");
    run_benchmark(uECC_secp256r1(), "256-bit");

    xil_printf("\r\nDone!\r\n");
    return 0;
}