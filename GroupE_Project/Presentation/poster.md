# The Security-Efficiency Trade-Off
​
## The Critical Challenge:​
The Conflict: IoT devices require 10+ years of battery life, but "Harvest-Now-
Decrypt-Later" threats push designers toward AES-256.​

The Missing Link: Until now, the exact computational and energy "tax" of this
migration on softcore processors was largely anecdotal.​

## The Project’s Impact​
Established a Controlled Baseline: By holding hardware constant (2,742 LUTs),
we isolated the algorithmic penalty, proving that security scaling is not "free"
even on the same chip.​

## Quantified the Trade-offs:​
Latency: A 38.5% increase in execution time when moving from 128 to 256-bit.​

Energy: A 158.82 nJ/bit increase in the "battery tax" per bit of data.​

Defined the "Efficiency Floor": Demonstrated that AES-128 remains the
optimal "Sweet Spot" for current edge nodes, maintaining a 27.5% higher
throughput-to-area efficiency than AES-256.​

​
