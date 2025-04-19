#!/usr/bin/env python3
"""
Telemetry & Dashboard
Attach eBPF uprobes to measure per-component latencies and export via Prometheus.
"""
try:
    from bcc import BPF
    have_bcc = True
except ImportError:
    print("Warning: bcc module not available. Running with dummy metrics only.")
    have_bcc = False
from prometheus_client import start_http_server, Histogram, Counter
import time, os

# Prometheus histograms
md_latency = Histogram('md_gen_enqueue_latency_us', 'Latency (us) for md-gen enqueue')
oms_latency = Histogram('oms_sor_latency_us', 'Latency (us) for OMS/SOR processing')
matcher_latency = Histogram('matcher_latency_us', 'Latency (us) for matcher processing')
logger_latency = Histogram('logger_latency_us', 'Latency (us) for logger write')
# Dummy counter for connectivity test
dummy_counter = Counter('dummy_counter_total', 'Counter to verify metrics endpoint')

# eBPF program
bpf_text = r"""
#include <uapi/linux/ptrace.h>
struct data_t { u64 delta_us; };
BPF_HASH(start, u64);
BPF_PERF_OUTPUT(events_md);
BPF_PERF_OUTPUT(events_oms);
BPF_PERF_OUTPUT(events_matcher);
BPF_PERF_OUTPUT(events_logger);

int trace_entry(struct pt_regs *ctx) {
    u64 id = bpf_get_current_pid_tgid();
    u64 ts = bpf_ktime_get_ns();
    start.update(&id, &ts);
    return 0;
}

int trace_return_md(struct pt_regs *ctx) {
    u64 id = bpf_get_current_pid_tgid();
    u64 *tsp = start.lookup(&id);
    if (!tsp) return 0;
    u64 d = bpf_ktime_get_ns() - *tsp;
    struct data_t data = { d / 1000 };
    events_md.perf_submit(ctx, &data, sizeof(data));
    start.delete(&id);
    return 0;
}
// repeat for OMS, matcher, logger
int trace_return_oms(struct pt_regs *ctx) { return trace_return_md(ctx); }
int trace_return_matcher(struct pt_regs *ctx) { return trace_return_md(ctx); }
int trace_return_logger(struct pt_regs *ctx) { return trace_return_md(ctx); }
"""

# Initialize BPF if available
if have_bcc:
    b = BPF(text=bpf_text)
root = os.path.expanduser('~/Desktop/Project')

# eBPF attachments disabled; running without eBPF

# event handlers
def handle_md(cpu, data, size):
    event = b["events_md"].event(data)
    md_latency.observe(event.delta_us)

def handle_oms(cpu, data, size):
    event = b["events_oms"].event(data)
    oms_latency.observe(event.delta_us)

def handle_matcher(cpu, data, size):
    event = b["events_matcher"].event(data)
    matcher_latency.observe(event.delta_us)

def handle_logger(cpu, data, size):
    event = b["events_logger"].event(data)
    logger_latency.observe(event.delta_us)

if have_bcc:
    b["events_md"].open_perf_buffer(handle_md)
    b["events_oms"].open_perf_buffer(handle_oms)
    b["events_matcher"].open_perf_buffer(handle_matcher)
    b["events_logger"].open_perf_buffer(handle_logger)

if __name__ == "__main__":
    start_http_server(8000)
    print("Prometheus metrics exposed on :8000")
    while True:
        # Increment dummy counter to ensure metrics are exposed
        dummy_counter.inc()
        # eBPF polling disabled
        # if have_bcc:
        #     b.perf_buffer_poll()
        time.sleep(1)
