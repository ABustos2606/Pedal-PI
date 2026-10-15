#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Run as root while the player sustains the guitar. Restarts only the UI.
# First save/reload with peepo_patch_live save-load to record its baseline.
set -euo pipefail
HERE="$(dirname "$(readlink -f "$0")")"
[ "$(id -u)" = 0 ] || { echo 'Run as root'; exit 1; }
for unit in peepo-jack peepo-engine peepo-ui; do systemctl is-active --quiet "$unit"; done
[ -f /tmp/peepo-patch-restart-baseline.ttl ] || { echo 'First run peepo_patch_live save-load'; exit 1; }
runuser -u peepo -- /opt/peepo/libexec/peepo/peepo_patch_live verify
python3 -c 'import jack, numpy' 2>/dev/null || env DEBIAN_FRONTEND=noninteractive apt-get install -y -qq python3-jack-client python3-numpy
marker=/tmp/peepo-restart-monitor-ready
rm -f "$marker"
jack_before=$(systemctl show -p MainPID --value peepo-jack)
engine_before=$(systemctl show -p MainPID --value peepo-engine)
ui_before=$(systemctl show -p MainPID --value peepo-ui)
runuser -u peepo -- python3 "$HERE/monitor-ui-restart.py" >/var/tmp/peepo-ui-restart-monitor.log 2>&1 &
monitor=$!
for _ in $(seq 100); do [ -e "$marker" ] && break; kill -0 "$monitor" || { cat /var/tmp/peepo-ui-restart-monitor.log; exit 1; }; sleep 0.1; done
[ -e "$marker" ] || { echo 'FAIL audio monitor did not become ready'; exit 1; }
echo "BEFORE JACK=$jack_before ENGINE=$engine_before UI=$ui_before"
sleep 8
echo 'RESTARTING ONLY peepo-ui'
systemctl restart peepo-ui
ui_after=$(systemctl show -p MainPID --value peepo-ui)
jack_after=$(systemctl show -p MainPID --value peepo-jack)
engine_after=$(systemctl show -p MainPID --value peepo-engine)
echo "AFTER JACK=$jack_after ENGINE=$engine_after UI=$ui_after"
[ "$jack_before" = "$jack_after" ] && [ "$engine_before" = "$engine_after" ] && [ "$ui_before" != "$ui_after" ]
echo 'PASS only the interface process changed; JACK and Ingen kept their process IDs'
wait "$monitor"
cat /var/tmp/peepo-ui-restart-monitor.log
systemctl is-active peepo.target peepo-jack peepo-engine peepo-route peepo-ui
runuser -u peepo -- /opt/peepo/libexec/peepo/peepo_patch_live verify
ss -xp | grep peepo
