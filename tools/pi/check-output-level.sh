#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
# The general Output (engine patch 0008) on the Pi: the steps of
# tools/ingen-spike/18-output-level.sh against the instrument's engine, with
# the audio interface detached so the test signal never reaches the speakers.
# Run as root with peepo_output_level_live beside this script:
#   sudo check-output-level.sh
# The running patch must be saved; it is loaded again at the end, and the
# audio interface is wired back, whatever happens.
set -euo pipefail
HERE="$(dirname "$(readlink -f "$0")")"
TOOL=/opt/peepo/libexec/peepo/peepo_effect_live
LIVE="$HERE/peepo_output_level_live"
CATALOGUE=/opt/peepo/share/peepo/catalogue.json
[ "$(id -u)" = 0 ] || { echo 'Run as root'; exit 1; }
shown=$(runuser -u peepo -- "$TOOL" show)
patch=$(sed -n 's/^GRAPH .* id=\([^ ]*\) dirty=.*/\1/p' <<<"$shown")
dirty=$(sed -n 's/^GRAPH .* dirty=\(.*\)/\1/p' <<<"$shown")
[ -n "$patch" ] && [ "$dirty" = 0 ] || { echo "The running patch must be saved first (id=$patch dirty=$dirty)"; exit 1; }
restore() {
    runuser -u peepo -- "$TOOL" restore "$patch" | grep -E '^(FAIL|GRAPH)' || true
    systemctl start peepo-route
}
trap restore EXIT
systemctl stop peepo-route
failed=0
measure() {
    runuser -u peepo -- python3 - "$1" "$2" <<'PY'
import sys, time, jack
LEVEL = 0.25
want, what = float(sys.argv[1]), sys.argv[2]
client = jack.Client("output_probe")
out = client.outports.register("dc"); caps = [client.inports.register(n) for n in ("l", "r")]; seen = []
def process(frames):
    out.get_array()[:] = LEVEL; seen.append([float(c.get_array()[-1]) for c in caps])
client.set_process_callback(process); client.activate()
saved = {}
for port in ("ingen:in_l", "ingen:in_r"):
    saved[port] = [p.name for p in client.get_all_connections(port)]
    for other in saved[port]: client.disconnect(other, port)
for port in ("ingen:out_l", "ingen:out_r"):
    saved[port] = [p.name for p in client.get_all_connections(port)]
    for other in saved[port]: client.disconnect(port, other)
try:
    for port in ("ingen:in_l", "ingen:in_r"): client.connect("output_probe:dc", port)
    client.connect("ingen:out_l", "output_probe:l"); client.connect("ingen:out_r", "output_probe:r")
    deadline = time.time() + 10
    while len(seen) < 60 and time.time() < deadline: time.sleep(0.05)
    left, right = seen[-1]
finally:
    out.disconnect(); [c.disconnect() for c in caps]
    for port, others in saved.items():
        for other in others:
            try: client.connect(other, port) if port.startswith("ingen:in") else client.connect(port, other)
            except jack.JackError: pass
    client.deactivate(); client.close()
ok = abs(left - want) < 0.001 and abs(right - LEVEL) < 0.001
print("%s  %s -- left %.4f (expected %.4f), right direct %.4f (expected %.4f)" % ("PASS" if ok else "FAIL", what, left, want, right, LEVEL))
raise SystemExit(0 if ok else 1)
PY
}
step() {
    local result
    result=$(runuser -u peepo -- "$LIVE" unix:///run/peepo/ingen.sock "$CATALOGUE" "$1") || failed=1
    grep -E "^(FAIL|SUMMARY)" <<<"$result" || true
}
step setup;     measure 0.125 "Output -6.02 dB halves the module's output" || failed=1
step duplicate; measure 0.125 "a duplicate plays at its Output of -6.02 dB" || failed=1
step bypass;    measure 0.25 "bypassed, the input passes at unity, Output not applied" || failed=1
echo "SUMMARY general Output on the Pi: $([ $failed = 0 ] && echo passed || echo FAILED)"
exit $failed
