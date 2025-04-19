# Product Requirements Document (PRD): Ultra‑Low‑Latency Trading Workstation & Simulator

## 1. Problem Statement
Real‑world HFT firms achieve sub‑100 µs end‑to‑end latencies using colocated hardware and kernel‑bypass techniques. This project must simulate the same pipeline on a single host, leveraging DPDK (or Netmap) and SPDK to bypass the kernel for both network and storage.

## 2. Objectives & Success Metrics
- [ ] **Prototype end‑to‑end trading pipeline**: market data → OMS → SOR → matching → fills.
- [ ] **End‑to‑end RTT < 100 µs** between feed ingestion and fill ack.
- [ ] **SPDK write latency < 10 µs** for event logging.
- [ ] **Throughput ≥ 1 Mpps** packet processing.

## 3. Functional Specifications
1. **Market Data Generator**: simulated ITCH/OUCH over raw UDP.
2. **OMS/SOR**: parent‑order slicing (TWAP, VWAP), venue scoring.
3. **Matching Engine**: lock‑free in‑memory limit order book.
4. **Risk Control**: per‑order circuit breakers, global kill switch.
5. **Logging & Telemetry**: SPDK‑backed persistence; eBPF/perf tracing; Grafana.

## 4. Non‑Functional Requirements
- **Reliability**: recoverable SPDK snapshots.
- **Modularity**: discrete services/threads per component.
- **Observability**: µs‑resolution metrics via Prometheus.

## 5. System Requirements
### 5.1 Hardware
- VM or bare‑metal Linux (≥ 4 vCPU, 8 GB RAM).
- NVMe SSD (e.g. Optane) for SPDK.
- DPDK/Netmap‑compatible NIC.

### 5.2 Software
- Ubuntu 20.04 LTS (kernel 5.4+).
- DPDK 20.11+ or Netmap.
- SPDK 20.10+.
- Rust 1.56+ or GCC 9+ (C++17).
- Python 3.8+.

## 6. Phases & Task Checklist
### Phase 1: Environment & Dependencies
- [X] Install Docker privileged container.
- [X] Allocate/mount hugepages (`vm.nr_hugepages=1024`).
- [X] Build and bind DPDK with vfio-pci.
- [X] Build and configure SPDK NVMe driver.
- [X] Install Rust/C++ toolchain & Python deps.

### Phase 2: Core Component Build
- [X] Scaffold repo: `md-gen`, `oms-sor`, `matcher`, `logger`, `telemetry`.
- [X] Build DPDK hello‑world TX/RX demo.
- [X] Build SPDK I/O latency sampler.

### Phase 3: Pipeline Integration
- [X] `md-gen`: UDP feed → DPDK ring.
- [X] `oms-sor`: ring → slice orders → DPDK send.
- [X] `matcher`: DPDK queue → LOB → fills.
- [X] `logger`: SPDK writes for orders/fills.

### Phase 4: Telemetry & Dashboard
- [X] Add eBPF/perf probes at each hop.
- [X] Export metrics to Prometheus.
- [X] Build Grafana/Python dashboard.

### Phase 5: Perf Testing & Tuning
- [X] Measure RTT and HFT‑style latency histogram.
- [X] Tune DPDK burst sizes & core pinning.
- [X] Tune SPDK queue depths & CPU affinities.
- [X] Document bottlenecks and optimizations.

## 7. Setup & Run From Scratch
```bash
# 1. Core tools
sudo apt update && sudo apt install -y build-essential git python3-pip
# hugepages
sudo sysctl -w vm.nr_hugepages=1024

# 2. DPDK
git clone https://github.com/DPDK/dpdk.git && cd dpdk
meson build && ninja -C build
sudo modprobe vfio-pci
sudo dpdk-devbind.py --bind=vfio-pci <PCI_ADDR>

# 3. SPDK
git clone https://github.com/spdk/spdk.git && cd spdk
./scripts/pkgdep.sh && ./configure --with-vfio && make -j

# 4. Project
cd ~/Desktop/Project
# Rust
cargo build --release
# or C++
make all

# 5. Run each component
# Terminal 1: market data
./md-gen/target/release/md-gen --iface dpdk0
# Terminal 2: OMS/SOR
./oms-sor/target/release/oms-sor
# Terminal 3: Matcher
./matcher/target/release/matcher
# Terminal 4: Logger
./logger/target/release/logger --ssd /dev/nvme0n1
# Terminal 5: Dashboard
python3 telemetry/dashboard.py
```
