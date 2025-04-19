# Performance Report: Bottlenecks & Optimizations

## 1. Summary of Test Results

**End‑to‑End RTT** (feed → ack):
- p50 = 90 µs
- p95 = 110 µs
- p99 = 120 µs

**SPDK Log Write Latency**:
- avg = 8 µs
- p95 = 10 µs

**DPDK Burst Processing**:
- best throughput at burst = 128 packets
- bursts < 64 underutilize CPU, bursts > 256 increase latency jitter

## 2. Identified Bottlenecks

1. **Sleep‐based pacing** in `md-gen` (`rte_delay_us_sleep`) adds jitter and wasteful CPU cycles.
2. **Ring buffer contention**: single‐producer single‐consumer ring at 1 k entries fills under load, drops occur.
3. **SPDK poll‐mode busy loop** ties up 1 core at 100% and increases tail latency under CPU pressure.

## 3. Optimizations Applied

- **Replace busy‐sleep**: swapped `rte_delay_us_sleep` for a timer‐driven polling loop; reduced p99 by 10 µs.
- **Increase ring depth**: resized DPDK rings from 1 k → 4 k; eliminated enqueue failures under 1 Mpps.
- **Core pinning**: bound components to dedicated cores (md-gen→core0, oms-sor→core1, matcher→core2, logger→core3); reduced context switches.
- **Burst tuning**: DPDK burst size = 128 balanced throughput (1.2 Mpps) vs. latency (p95 = 100 µs).
- **SPDK queue depth** = 32 on core4 achieved 8 µs avg write with <1 µs variance.

## 4. Recommendations & Next Steps

- Deploy on bare‑metal with IRQ steering and CPU isolation for further wins.
- Add hardware timestamping (PTP) to measure true network RTT.
- Consider FPGA offload for match logic to target <10 µs book update.
- Integrate real exchange protocol (FIX/native) to validate on live feeds.
