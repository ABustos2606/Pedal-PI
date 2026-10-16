#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
# The general Output (engine patch 0008) end to end: the adapter sets a block's
# output level, and what comes out of the engine is measured after each step.
# Run as root inside Debian, after 01-build.sh:  bash 18-output-level.sh
D="$(dirname "$(readlink -f "$0")")"
. "$D/env.sh"
BUILD="${PEEPO_BUILD:-/tmp/peepo-adapter-build}"
CATALOGUE="${PEEPO_CATALOGUE:-$D/../../config/catalogue/wsl-debian.json}"
cmake -S "$D/../.." -B "$BUILD" -G Ninja -DPEEPO_BUILD_UI=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo >/dev/null || exit 1
cmake --build "$BUILD" --target peepo_output_level_live 2>&1 | grep -E "error|warning|FAILED"
python3 -c "import jack, numpy" 2>/dev/null || apt-get install -y -qq python3-jack-client python3-numpy >/dev/null
rm -rf /tmp/peepo-output-patches
start_engine || exit 1
failed=0
# A constant 0.25 into both inputs; the left goes through the module, the right
# straight to the output. Prints the left and right output levels.
measure() {
    python3 - "$1" "$2" <<'PY'
import sys, time, jack
LEVEL = 0.25
want, what = float(sys.argv[1]), sys.argv[2]
client = jack.Client("probe"); out = client.outports.register("dc"); caps = [client.inports.register(n) for n in ("l", "r")]; seen = []
def process(frames):
    out.get_array()[:] = LEVEL; seen.append([float(c.get_array()[-1]) for c in caps])
client.set_process_callback(process); client.activate()
for port in ("ingen:in_l", "ingen:in_r"): client.connect("probe:dc", port)
client.connect("ingen:out_l", "probe:l"); client.connect("ingen:out_r", "probe:r")
deadline = time.time() + 10
while len(seen) < 60 and time.time() < deadline: time.sleep(0.05)
left, right = seen[-1]
ok = abs(left - want) < 0.001 and abs(right - LEVEL) < 0.001
print("%s  %s -- left %.4f (expected %.4f), right direct %.4f (expected %.4f)" % ("PASS" if ok else "FAIL", what, left, want, right, LEVEL))
client.deactivate(); client.close()
raise SystemExit(0 if ok else 1)
PY
}
step() { "$BUILD/peepo_output_level_live" unix:///tmp/peepo-ingen.sock "$CATALOGUE" "$1" || failed=1; }
step setup;     measure 0.125 "Output -6.02 dB halves the module's output" || failed=1
step reload;    measure 0.125 "after saving and loading back, the engine applies -6.02 dB again" || failed=1
step duplicate; measure 0.125 "a duplicate plays at its Output of -6.02 dB" || failed=1
step bypass;    measure 0.25 "bypassed, the input passes at unity, Output not applied" || failed=1
stop_engine
echo "SUMMARY general Output: $([ $failed = 0 ] && echo passed || echo FAILED)"
exit $failed
