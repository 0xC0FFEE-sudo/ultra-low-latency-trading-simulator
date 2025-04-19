#!/usr/bin/env bash
# Tune SPDK performance using SPDK bdevperf example for NVMe logging

SPDK_ROOT=/root/spdk
BDEVPERF=$SPDK_ROOT/build/examples/bdevperf
CORE_MASK=0x10         # use core 4 (bitmask)
QDEPTHS=(1 2 4 8 16 32 64)

if [[ ! -x $BDEVPERF ]]; then
  echo "Error: bdevperf not found at $BDEVPERF"
  exit 1
fi

echo "Tuning SPDK queue depths"
for q in "${QDEPTHS[@]}"; do
  echo "== queue depth: $q =="
  sudo $BDEVPERF -q "$q" -c "$CORE_MASK" -o 4096 -w randread -t 1
  echo
  sleep 1
done

echo "SPDK tuning complete."
