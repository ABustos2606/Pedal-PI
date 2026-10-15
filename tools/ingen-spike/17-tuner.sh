#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
# Only run under WSL with no existing JACK/Ingen session.
set -euo pipefail
cd "$(dirname "$(readlink -f "$0")")/../.."
source tools/ingen-spike/env.sh
if pgrep -x jackd >/dev/null || pgrep -x ingen >/dev/null; then
    echo 'An audio test session is already running; close it before this isolated test.' >&2
    exit 1
fi
cmake -S . -B /root/peepo-build -G Ninja -DPEEPO_BUILD_UI=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo >/dev/null
cmake --build /root/peepo-build --target peepo_tuner_live -j 6
mkdir -p build/scratch
JACK_NO_AUDIO_RESERVATION=1 jackd -d dummy -r 48000 -p 128 >build/scratch/tuner-jack.log 2>&1 &
jack_pid=$!
engine_pid=''
cleanup() {
    if [[ -n "$engine_pid" ]]; then kill "$engine_pid" 2>/dev/null || true; fi
    kill "$jack_pid" 2>/dev/null || true
}
trap cleanup EXIT
for _ in $(seq 50); do jack_lsp >/dev/null 2>&1 && break; sleep 0.1; done
ingen -e -f -E 0 -S /tmp/peepo-tuner.sock >build/scratch/tuner-ingen.log 2>&1 &
engine_pid=$!
for _ in $(seq 50); do [[ -S /tmp/peepo-tuner.sock ]] && break; sleep 0.1; done
python3 tools/ingen-spike/17-tuner.py /root/peepo-build/peepo_tuner_live build/scratch/tuner-frequency config/catalogue/wsl-debian.json
