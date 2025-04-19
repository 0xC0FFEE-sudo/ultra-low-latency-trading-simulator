#!/usr/bin/env bash
# Tune DPDK burst sizes and core pinning for DPDK-based components
# Usage: ./tune_dpdk.sh <app> (md-gen | oms-sor | matcher)

set -euo pipefail
APP=${1:-md-gen}
CORE_LIST="0,1,2,3"          # CPU cores for EAL (-l)
BURST_SIZES=(32 64 128 256 512)
BIN="./${APP}/main"

if [[ ! -x $BIN ]]; then
  echo "Binary $BIN not found or not executable"
  exit 1
fi

echo "Tuning $APP with cores $CORE_LIST"
for burst in "${BURST_SIZES[@]}"; do
  echo "== burst size: $burst =="
  taskset -c $CORE_LIST $BIN -l $CORE_LIST -- --burst=$burst &
  PID=$!
  sleep 2
  kill $PID
  echo
  sleep 1
done

echo "DPDK tuning complete."
