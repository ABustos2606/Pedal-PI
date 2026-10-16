#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
#
# P10.7 on the Pi: the interface crashes (SIGKILL, no orderly exit) while the
# instrument plays. systemd must start it again by itself; JACK and the
# engine must keep their processes, the audio callbacks must run without a
# gap, and the engine's graph must be the same afterwards. Read-only: nothing
# is saved, loaded or connected to the speakers. Run as root:
#   sudo check-ui-crash.sh
set -euo pipefail
HERE="$(dirname "$(readlink -f "$0")")"
TOOL=/opt/peepo/libexec/peepo/peepo_effect_live
MONITOR=${MONITOR:-$HERE/monitor-ui-restart.py}
[ -f "$MONITOR" ] || MONITOR=/opt/peepo/libexec/peepo/monitor-ui-restart.py
[ "$(id -u)" = 0 ] || { echo 'Run as root'; exit 1; }
for unit in peepo-jack peepo-engine peepo-ui; do systemctl is-active --quiet "$unit"; done
python3 -c 'import jack, numpy' 2>/dev/null || env DEBIAN_FRONTEND=noninteractive apt-get install -y -qq python3-jack-client python3-numpy
pid() { systemctl show -p MainPID --value "$1"; }
graph() { runuser -u peepo -- "$TOOL" show | grep -v '^PASS'; }
# A socket of the interface's besides the journal's (its standard output and error).
# Read whole: grep -q would stop the writer early, and pipefail calls that a failure.
connected() { local list; list=$(ss -xp); [ "$(grep -v ',fd=[12])' <<<"$list" | grep -c "users:((\"peepo\",pid=$1,")" -gt 0 ]; }
# The engine logs every client it registers; after the crash only the new interface connects.
registered_since() { journalctl -u peepo-engine --since "@$1" --no-pager 2>/dev/null | grep -c 'Registering client'; }

before=$(graph)
jack_before=$(pid peepo-jack) engine_before=$(pid peepo-engine) ui_before=$(pid peepo-ui)
restarts_before=$(systemctl show -p NRestarts --value peepo-ui)
echo "BEFORE JACK=$jack_before ENGINE=$engine_before UI=$ui_before restarts=$restarts_before"
connected "$ui_before" && echo "PASS  the interface is connected to the engine" || echo "NOTE  the interface's connection was not seen before the crash"

marker=/tmp/peepo-restart-monitor-ready
rm -f "$marker"
runuser -u peepo -- python3 "$MONITOR" >/var/tmp/peepo-ui-crash-monitor.log 2>&1 &
monitor=$!
for _ in $(seq 100); do [ -e "$marker" ] && break; kill -0 "$monitor" || { cat /var/tmp/peepo-ui-crash-monitor.log; exit 1; }; sleep 0.1; done
[ -e "$marker" ] || { echo 'FAIL  the audio monitor did not become ready'; exit 1; }
sleep 4
echo 'CRASHING the interface with SIGKILL'
killed_at=$(date +%s.%N)
systemctl kill -s SIGKILL peepo-ui
ui_after=
for _ in $(seq 200); do
    ui_after=$(pid peepo-ui)
    [ "$ui_after" != 0 ] && [ "$ui_after" != "$ui_before" ] && [ "$(registered_since "${killed_at%.*}")" -gt 0 ] && break
    sleep 0.05
done
back_at=$(date +%s.%N)
wait "$monitor"
after=$(graph)
jack_after=$(pid peepo-jack) engine_after=$(pid peepo-engine)
restarts_after=$(systemctl show -p NRestarts --value peepo-ui)
echo "AFTER JACK=$jack_after ENGINE=$engine_after UI=$ui_after restarts=$restarts_after"
failures=0
check() { if eval "$1"; then echo "PASS  $2"; else echo "FAIL  $2"; failures=$((failures + 1)); fi; }
check '[ "$ui_after" != "$ui_before" ] && [ "$ui_after" != 0 ] && [ "$restarts_after" -gt "$restarts_before" ]' "systemd started a new interface by itself (restart count $restarts_before -> $restarts_after)"
check '[ "$(registered_since "${killed_at%.*}")" -gt 0 ] && connected "$ui_after"' "the new interface connected to the engine, $(python3 -c "print('%.1f' % ($back_at - $killed_at))") s after the crash"
check '[ "$jack_before" = "$jack_after" ] && [ "$engine_before" = "$engine_after" ]' "JACK and the engine kept their processes"
check '[ "$before" = "$after" ]' "the engine's graph is unchanged: same patch, modules, controls and cables"
if ! python3 - /tmp/peepo-ui-restart-audio.json <<'PY'
import json, sys
r = json.load(open(sys.argv[1]))
ok = r["callback_frame_gaps"] == 0 and r["xruns"] == 0 and r["all_finite"]
print("%s  audio callbacks ran without a gap through the crash: %d blocks over %.1f s, %d gaps, %d xruns, peak input RMS %s, silent output blocks %s"
      % ("PASS" if ok else "FAIL", r["blocks"], r["duration_seconds"], r["callback_frame_gaps"], r["xruns"],
         ["%.4f" % v for v in r["maximum_input_rms"]], r["exactly_silent_output_blocks"]))
sys.exit(0 if ok else 1)
PY
then failures=$((failures + 1)); fi
echo "$after" | head -1
exit $failures
