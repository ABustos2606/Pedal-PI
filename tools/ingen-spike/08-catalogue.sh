#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
# Export the catalogue of the plugins this engine can see, then load every
# available entry in the live engine and compare it with what was exported.
# Run as root inside Debian, after 01-build.sh and 02-plugins.sh:
#   bash 08-catalogue.sh [output.json]
# Without an argument the export replaces the fixture the other platforms use.
D="$(dirname "$(readlink -f "$0")")"
. "$D/env.sh"
BUILD="${PEEPO_BUILD:-/tmp/peepo-adapter-build}"
CONFIG="$D/../../config/catalogue"
OUT="${1:-$CONFIG/wsl-debian.json}"
cmake -S "$D/../.." -B "$BUILD" -G Ninja -DPEEPO_BUILD_UI=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo >/dev/null || exit 1
cmake --build "$BUILD" 2>&1 | grep -E "error|warning|FAILED"
[ -x "$BUILD/peepo_catalogue_export" ] || { echo "exporter was not built (is liblilv-dev installed?)"; exit 1; }
echo "==== export (LV2_PATH=$LV2_PATH)"
"$BUILD/peepo_catalogue_export" --only "$CONFIG/inventory.txt" -o "$OUT" || exit 1
"$BUILD/peepo_catalogue_tests" "$CONFIG" || exit 1
echo "==== every available entry, loaded in the engine"
start_engine || exit 1
"$BUILD/peepo_catalogue_live" unix:///tmp/peepo-ingen.sock "$OUT" "$CONFIG/overlay.json" "http://polyeffects.com/lv2/polyclouds#granular"; live=$?
echo "==== sound"
python3 "$D/09-sound.py" --left "Clouds" || live=1
# The same question asked of the engine directly, on a fresh one.
start_engine || exit 1
python3 "$D/09-sound.py" || live=1
pgrep -x ingen >/dev/null && echo "PASS  the engine is still running after loading every entry" || { echo "FAIL  the engine did not survive"; live=1; }
grep -E "Ignoring|rror" "$RUN/ingen.log" | sort | uniq -c | head -20
stop_engine
exit $live
