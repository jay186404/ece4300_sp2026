# AES Performance Benchmarking on Softcore Processor Implementations

**Project Team:** Edwin Estrada, Nicholas Johnson, Samuel Regan, Kenneth Wang, David Jessup

---

## Overview

This project benchmarks the performance of the Advanced Encryption Standard (AES) algorithm across multiple softcore processor implementations. By targeting the same cryptographic workload on different softcore architectures, we aim to produce a fair, apples-to-apples comparison of each platform's throughput, latency, and resource utilization when executing AES encryption and decryption.

---

## Motivation

AES is one of the most widely deployed symmetric encryption standards in the world, used in everything from TLS/HTTPS to disk encryption and embedded security modules. Understanding how different softcore processors handle AES workloads is valuable for:

- Selecting the right FPGA-based processor for security-sensitive embedded applications
- Identifying bottlenecks in instruction pipelines when executing compute-heavy cryptographic loops
- Evaluating the trade-off between hardware resource usage (LUTs, BRAMs, DSPs) and encryption throughput

---

## Softcore Implementations Under Test

The following softcore processors are included in our benchmark suite:

| Softcore | Architecture | Target FPGA | Notes |
|----------|-------------|-------------|-------|
| MicroBlaze | 32-bit RISC | Xilinx/AMD | Xilinx proprietary softcore |
| LEON3 | 32-bit SPARC V8 | Vendor-agnostic (Xilinx tested) | Open-source, FrontGrade Gaisler |
| PicoRV32 | 32-bit RISC-V | Vendor-agnostic | Open-source, YosysHQ |
| VexRiscv | 32-bit RISC-V | Vendor-agnostic | Configurable pipeline depth |