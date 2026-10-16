#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
# P3.4 on the Pi: MDA Overdrive alone between the engine's IN and OUT, measured
# with a test tone that is never sent to the speakers. Run as root:
#   sudo check-effect.sh            Overdrive's gain
#   sudo check-effect.sh clouds     how Clouds' blend mixes dry and wet
# The running patch must be saved (no unsaved edits); it is loaded again at
# the end, and the audio interface is wired back, whatever happens.
set -euo pipefail
HERE="$(dirname "$(readlink -f "$0")")"
TOOL=/opt/peepo/libexec/peepo/peepo_effect_live
[ "$(id -u)" = 0 ] || { echo 'Run as root'; exit 1; }
for unit in peepo-jack peepo-engine peepo-route; do systemctl is-active --quiet "$unit"; done
python3 -c 'import jack, numpy' 2>/dev/null || env DEBIAN_FRONTEND=noninteractive apt-get install -y -qq python3-jack-client python3-numpy
shown=$(runuser -u peepo -- "$TOOL" show)
patch=$(sed -n 's/^GRAPH .* id=\([^ ]*\) dirty=.*/\1/p' <<<"$shown")
dirty=$(sed -n 's/^GRAPH .* dirty=\(.*\)/\1/p' <<<"$shown")
[ -n "$patch" ] && [ "$dirty" = 0 ] || { echo "The running patch must be saved first (id=$patch dirty=$dirty)"; exit 1; }
echo "RUNNING patch $patch; it is reloaded at the end"
restore() {
    runuser -u peepo -- "$TOOL" restore "$patch" | grep -E '^(PASS|FAIL|GRAPH)' || true
    systemctl start peepo-route
    sleep 3
    sudo -u peepo jack_lsp -c ingen:in_l ingen:in_r ingen:out_l ingen:out_r
}
trap restore EXIT
# The router would wire the guitar and speakers back in mid-measurement.
systemctl stop peepo-route
runuser -u peepo -- "$TOOL" test-graph "${1:-}"
runuser -u peepo -- python3 "$HERE/measure-effect.py" "${1:-}"
