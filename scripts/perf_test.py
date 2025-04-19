#!/usr/bin/env python3
"""
Quick performance testing: fetch Prometheus histograms and print latency buckets.
"""
from urllib.request import urlopen
import time

METRICS_URL = 'http://localhost:8000/metrics'


def fetch_buckets(name):
    text = urlopen(METRICS_URL).read().decode()
    buckets = []
    for line in text.splitlines():
        if line.startswith(f"{name}_bucket"):
            parts = line.split()
            labels = line.split('{')[1].split('}')[0]
            bound = float([kv for kv in labels.split(',') if 'le=' in kv][0].split('=')[1])
            count = int(parts[1])
            buckets.append((bound, count))
    return buckets


if __name__ == '__main__':
    print("Waiting for metrics...\n")
    time.sleep(2)
    metrics = ['md_gen_enqueue_latency_us', 'oms_sor_latency_us', 'matcher_latency_us', 'logger_latency_us']
    for m in metrics:
        print(f"== {m} ==")
        for bound, count in fetch_buckets(m):
            print(f"<= {bound} µs: {count}")
        print()
