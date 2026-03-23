# AES Performance Benchmarking on Softcore Processor Implementations

**Project Team:** Edwin Estrada, Nicholas Johnson, Samuel Regan, Kenneth Wang, David Jessup

---

## Overview

This project benchmarks the performance of the Advanced Encryption Standard (AES) algorithm with the MicroBlaze 32-bit RISC softcore processor implementation. By targeting the same cryptographic workload on this softcore architectures, we aim to produce a fair benchmark of the platform's throughput, latency, and resource utilization when executing AES encryption and decryption.

---

## Motivation

AES is one of the most widely deployed symmetric encryption standards in the world, used in everything from TLS/HTTPS to disk encryption and embedded security modules. Understanding how this softcore processor handle AES workloads is valuable for:

- Selecting the right FPGA-based processor for security-sensitive embedded applications
- Identifying bottlenecks in instruction pipelines when executing compute-heavy cryptographic loops
- Evaluating the trade-off between hardware resource usage (LUTs, BRAMs, DSPs) and encryption throughput

---

## Demo

Link: https://youtube.com/shorts/gfvgCmoDOPA?feature=share

## Presentation

Link: https://github.com/california-polytechnic-university/ECE4300_SP2026/blob/927e761a7aa1dab8a3db5e0c8dbe44f83d7643a2/GroupJ_Project/Presentation/ECE4300_Project_Slides.pdf

## Report

Link: https://github.com/california-polytechnic-university/ECE4300_SP2026/blob/cbff5cc4442c05bfd132a45a053e55d78f611513/GroupJ_Project/Document/ECE4300_Project_Article.pdf
