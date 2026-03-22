# Nexys A7 ECC Benchmarking (MicroBlaze)

This project benchmarks the performance of Elliptic Curve Cryptography (ECC) on a 32-bit MicroBlaze soft-core processor.

The implementation uses the **micro-ecc** library to perform Key Generation, ECDSA Signing, and ECDSA Verification across standard NIST prime curves.

## Hardware Configuration
* **FPGA:** Nexys A7-100T
* **Processor:** MicroBlaze (32-bit)
* **Clock Frequency:** 100 MHz
* **Optimization Level:** `-O0` (Standard Debug)

## Benchmark Results
The following measurements were taken using an AXI Timer. Real-time values are calculated based on the 100 MHz system clock ($T = \text{Cycles} / 10^8$).

| Bit Size | Curve Name | Operation | Cycles | Time (Seconds) |
| :--- | :--- | :--- | :--- | :--- |
| **160-bit** | **secp160r1** | KeyGen | 60,126,979 | 0.60s |
| | | Signing | 62,511,860 | 0.62s |
| | | Verify | 70,205,010 | 0.70s |
| **192-bit** | **secp192r1** | KeyGen | 94,129,079 | 0.94s |
| | | Signing | 96,369,492 | 0.96s |
| | | Verify | 108,902,654 | 1.09s |
| **256-bit** | **secp256r1** | KeyGen | 234,176,849 | 2.34s |
| | | Signing | 238,078,119 | 2.38s |
| | | Verify | 267,640,482 | 2.68s |

## Summary of Operations
1. **KeyGen:** Generates a private/public key pair. In ECC, the public key is a point on the curve calculated by multiplying the base point by the private key.
2. **Signing:** Proves the identity of the sender and data integrity. Uses the private key to sign a hash of the data.
3. **Verification:** The most computationally expensive step. Uses the sender's public key to mathematically prove the signature matches the data.

## Library Reference
* **Original Library:** [kmackay/micro-ecc](https://github.com/kmackay/micro-ecc)
