# Project Overview: Ultra‑Low‑Latency Trading Workstation & Simulator

## 1 Introduction
This project delivers a fully simulated micro‑latency trading pipeline on a single Linux host (VM or Docker on MacBook). It covers market data feed generation, order management, smart order routing, matching engine, risk control, logging, and telemetry. The system leverages kernel‑bypass techniques for both network (DPDK or Netmap) and storage (SPDK) to achieve sub‑100 µs latencies end to end.

## 2 Goals
- Prototype end‑to‑end trading simulation
- Explore user‑space NIC bypass (DPDK or Netmap)
- Explore user‑space NVMe bypass (SPDK)
- Build lock‑free limit order book and TWAP/VWAP engines
- Instrument components with microsecond‑level metrics

## 3 System Components

### 3.1 Environment Setup
- Host OS: Ubuntu 20.04 LTS in VM or Docker
- Kernel configuration: hugepages, real‑time patch optional
- DPDK or Netmap libraries bound to NIC
- SPDK NVMe driver for SSD bypass
- Language toolchain: Rust or C++17, Python3 for orchestration

### 3.2 Networking Stack
- DPDK poll‑mode driver or Netmap for direct NIC access
- Raw UDP messaging for market data (ITCH/OUCH simulator)
- TX/RX ring buffers for inter‑component communication

### 3.3 Storage Stack
- SPDK to bypass kernel I/O stack for low‑latency NVMe access
- Asynchronous IO queues for event logging and state snapshots
- Durable order/fill logs persisted on SSD

### 3.4 Core Trading Pipeline
1. Market Data Generator: high‑speed UDP feed simulator
2. Strategy Engine & OMS: slices parent orders into child orders
3. Smart Order Router: venue scoring and routing logic
4. Matching Engine: lock‑free in‑memory limit order book
5. Risk Control: real‑time circuit breakers and kill switches
6. Logger: SPDK‑backed event log writer

### 3.5 Telemetry and Monitoring
- eBPF or perf for timestamping and tracing
- Python or Grafana scripts to plot latency histograms
- Automated alerts on performance or risk breaches

## 4 Architecture Diagram (textual)
```text
[MD Gen] --DPDK RX--> [OMS/SOR] --DPDK TX--> [Matcher] --SPDK--> [Logger]
                              |--eBPF--> [Metrics Collector]
```

## 5 Performance Targets
- End‑to‑end RTT (feed in to ack out) < 100 µs
- SPDK log write latency < 10 µs
- Packet processing throughput > 1 Mpps

## 6 Future Extensions
- Migrate from VM to bare‑metal Linux
- Add PTP hardware time sync (IEEE 1588)
- Integrate FPGA path for sub‑10 µs decision logic
- Extend OMS with real‑market integration via FIX/native protocols
