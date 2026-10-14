#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pedal PI contributors
# Build the project's own engine adapter and run it against a live Ingen, then
# measure what the patch it leaves behind does to a signal. Run as root inside
# Debian, from anywhere:  bash 07-adapter.sh
D="$(dirname "$(readlink -f "$0")")"
. "$D/env.sh"
BUILD="${PEDALPI_BUILD:-/tmp/pedalpi-adapter-build}"
# The modules this engine offers, as exported by 08-catalogue.sh.
CATALOGUE="$D/../../config/catalogue/wsl-debian.json"
command -v cmake >/dev/null || apt-get install -y -q cmake >/dev/null
cmake -S "$D/../.." -B "$BUILD" -G Ninja -DPEDAL_BUILD_UI=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo >/dev/null || exit 1
cmake --build "$BUILD" 2>&1 | grep -E "error|warning|FAILED" ; [ -x "$BUILD/pedal_ingen_live" ] || exit 1
"$BUILD/pedal_ingen_tests" || exit 1
start_engine || exit 1
"$BUILD/pedal_ingen_live" unix:///tmp/pedalpi-ingen.sock "$CATALOGUE"; live=$?
# The same checks over TCP, which is how a Windows build reaches an engine in WSL.
"$BUILD/pedal_ingen_live" tcp://127.0.0.1:16180 "$CATALOGUE" | grep -E "^(FAIL|SUMMARY)"; tcp=${PIPESTATUS[0]}
python3 - <<'PY'
import time
import jack
LEVEL = 0.25
client = jack.Client("probe"); out = client.outports.register("dc"); cap = client.inports.register("cap"); seen = []
def process(frames):
    out.get_array()[:] = LEVEL; seen.append(float(cap.get_array().max()))
client.set_process_callback(process); client.activate()
ports = sorted(p.name for p in client.get_ports("ingen:"))
client.connect("probe:dc", "ingen:in_l"); client.connect("ingen:out_l", "probe:cap"); time.sleep(0.4)
left = seen[-1]
client.disconnect("probe:dc", "ingen:in_l"); client.disconnect("ingen:out_l", "probe:cap")
client.connect("probe:dc", "ingen:in_r"); client.connect("ingen:out_r", "probe:cap"); time.sleep(0.4)
right = seen[-1]
ok = abs(left - 2 * LEVEL) < 0.002 and abs(right - LEVEL) < 0.002
print("%s  the patch the adapter built is what is heard -- left through the amp at +6.02 dB %.4f (expected 0.5000), right direct %.4f (expected 0.2500); JACK ports %s"
      % ("PASS" if ok else "FAIL", left, right, ", ".join(ports)))
client.deactivate(); client.close()
raise SystemExit(0 if ok else 1)
PY
audio=$?
# Stop the engine under a connected adapter, start a fresh one, and see it return.
"$BUILD/pedal_ingen_live" unix:///tmp/pedalpi-ingen.sock "$CATALOGUE" --survive-restart > "$RUN/restart.log" 2>&1 &
waiter=$!
for _ in $(seq 100); do grep -q WAITING "$RUN/restart.log" && break; sleep 0.1; done
start_engine
wait $waiter; restart=$?
grep -vE "^(WAITING|PASS  connect)" "$RUN/restart.log"
stop_engine
[ "$live" = 0 ] && [ "$tcp" = 0 ] && [ "$audio" = 0 ] && [ "$restart" = 0 ]
