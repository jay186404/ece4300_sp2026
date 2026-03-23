#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>

#ifndef NSAMPLES
#define NSAMPLES 100000000
#endif

#ifndef NTAPS
#define NTAPS 8
#endif

#ifndef NITER
#define NITER 10   // number of benchmark iterations
#endif

static double get_time_sec(void) {
    static LARGE_INTEGER freq;
    static int initialized = 0;
    LARGE_INTEGER counter;

    if (!initialized) {
        QueryPerformanceFrequency(&freq);
        initialized = 1;
    }

    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)freq.QuadPart;
}

int main(void) {
    static const int16_t h[NTAPS] = {1, 2, 3, 4, 4, 3, 2, 1};

    int16_t *x = (int16_t *)calloc(NSAMPLES, sizeof(int16_t));
    int64_t *y = (int64_t *)calloc(NSAMPLES, sizeof(int64_t));

    if (x == NULL || y == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(x);
        free(y);
        return 1;
    }

    for (int i = 0; i < NSAMPLES; i++) {
        x[i] = (int16_t)(i & 0xFF);
    }


    // WARM-UP RUN 

    for (int n = 0; n < NSAMPLES; n++) {
        int64_t acc = 0;
        for (int k = 0; k < NTAPS; k++) {
            if (n >= k) {
                acc += (int64_t)h[k] * (int64_t)x[n - k];
            }
        }
        y[n] = acc;
    }


    // TIMED RUNS

    double t0 = get_time_sec();

    for (int iter = 0; iter < NITER; iter++) {
        for (int n = 0; n < NSAMPLES; n++) {
            int64_t acc = 0;
            for (int k = 0; k < NTAPS; k++) {
                if (n >= k) {
                    acc += (int64_t)h[k] * (int64_t)x[n - k];
                }
            }
            y[n] = acc;
        }
    }

    double t1 = get_time_sec();

    // METRICS

    double elapsed_sec = t1 - t0;
    double total_samples = (double)NSAMPLES * NITER;
    double throughput = total_samples / elapsed_sec;

    printf("NSAMPLES           : %d\n", NSAMPLES);
    printf("NITER              : %d\n", NITER);
    printf("Total Samples      : %.0f\n", total_samples);
    printf("Elapsed time (s)   : %.9f\n", elapsed_sec);
    printf("Throughput (S/s)   : %.3f\n", throughput);
    printf("Throughput (MS/s)  : %.6f\n", throughput / 1e6);
    printf("Last output sample : %lld\n", (long long)y[NSAMPLES - 1]);

    free(x);
    free(y);
    return 0;
}