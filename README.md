# Ultra-Low-Latency Trading Simulator

This project implements an ultra-low-latency trading workstation and simulator using [DPDK](https://www.dpdk.org/) for packet processing and a modular ring-based architecture to simulate market data generation, order management, matching, and logging (stubbed).

## Features

- **Market Data Generator** (`md-gen`): Publishes dummy market data into a DPDK ring on dedicated lcore.
- **Order Management/SOR** (`oms-sor`): Consumes market data, applies simple risk/PnL checks, and enqueues orders.
- **Matcher** (`matcher`): Processes orders and forwards as fills between rings with configurable worker threads.
- **Logger** (`logger`): Stubbed component (revert SPDK integration) for demonstration.
- **Telemetry Dashboard** (`telemetry/dashboard.py`): Exposes Prometheus metrics (dummy counter) and can be extended with eBPF probes.
- **Grafana Dashboard** (`telemetry/grafana/dashboard.json`): Minimal JSON for visualizing the dummy counter.

## Getting Started

1. **Build & Run** (inside [Docker](https://www.docker.com/) container `hft_demo`):
   ```bash
   make all
   md-gen/md-gen -i 1000 &
   oms-sor/oms-sor &
   matcher/matcher -t 2 &
   logger/logger &
   ```
2. **Telemetry**:
   ```bash
   cd telemetry
   ../telemetry-venv/bin/python dashboard.py
   ```
3. **Metrics**: Browse `http://localhost:8000/metrics` and import `telemetry/grafana/dashboard.json` into [Grafana](https://grafana.com/).

## Documentation

- **DPDK**: [Data Plane Development Kit](https://www.dpdk.org/) - An open-source set of libraries and drivers for high-performance packet processing.
- **SPDK**: [Storage Performance Development Kit](https://spdk.io/) - A set of tools and libraries for high-performance storage applications.
- **eBPF**: [Extended Berkeley Packet Filter](https://ebpf.io/) - Technology for running sandboxed programs in the Linux kernel for observability and networking.
- **Prometheus**: [Prometheus Monitoring](https://prometheus.io/) - An open-source monitoring and alerting toolkit.
- **Grafana**: [Grafana](https://grafana.com/) - An open-source platform for analytics and interactive visualization.
- **Docker**: [Docker](https://www.docker.com/) - A platform for developing, shipping, and running containerized applications.

## License

This project is licensed under the MIT License - see `LICENSE` for details.
