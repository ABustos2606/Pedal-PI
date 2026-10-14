#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pedal PI contributors
# Compatibility records for the whole inventory (P9.1, P9.3). Run as root
# inside Debian, after 02-plugins.sh, 12-nam.sh and 14-more-plugins.sh:
#   bash 15-compat.sh
# Exports the catalogue restricted to config/catalogue/inventory.txt (this
# replaces the fixture the other platforms use), loads every offered entry in
# the engine, then writes docs/evidence/compatibility.json and
# docs/evidence/compatibility-matrix.md.
D="$(dirname "$(readlink -f "$0")")"
. "$D/env.sh"
BUILD="${PEDALPI_BUILD:-/tmp/pedalpi-adapter-build}"
CONFIG="$D/../../config/catalogue"
OUT="$CONFIG/wsl-debian.json"
mkdir -p "$RUN"
# The NeuralAudio candidate stands for NAM in the catalogue (P8.1).
export LV2_PATH="$LV2_PATH:$PREFIX/nam/current"
dpkg -s python3-lilv >/dev/null 2>&1 || apt-get install -y -q python3-lilv >/dev/null
cmake -S "$D/../.." -B "$BUILD" -G Ninja -DPEDAL_BUILD_UI=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo >/dev/null || exit 1
cmake --build "$BUILD" 2>&1 | grep -E "error|warning|FAILED"
echo "==== export (LV2_PATH=$LV2_PATH)"
"$BUILD/pedal_catalogue_export" --only "$CONFIG/inventory.txt" -o "$OUT" 2>"$RUN/export.log" || { cat "$RUN/export.log"; exit 1; }
tail -1 "$RUN/export.log"
"$BUILD/pedal_catalogue_tests" "$CONFIG" || exit 1
echo "==== every offered entry, loaded in the engine"
start_engine || exit 1
# Without the overlay: every entry the export offers is checked, including
# those the overlay withholds for presentation reasons, but not the plugins
# known to crash the engine (15-compat.py lists them).
python3 "$D/15-compat.py" --without-crashes "$OUT" /var/tmp/pedalpi-live-catalogue.json
"$BUILD/pedal_catalogue_live" unix:///tmp/pedalpi-ingen.sock /var/tmp/pedalpi-live-catalogue.json > /var/tmp/pedalpi-live.txt; status=$?
grep -E "^(FAIL|SUMMARY)" /var/tmp/pedalpi-live.txt
echo "==== records"
start_engine || exit 1
python3 "$D/15-compat.py" "$OUT" /var/tmp/pedalpi-live.txt /tmp/pedalpi-compat || status=1
pgrep -x ingen >/dev/null && echo "PASS  the engine is still running" || { echo "FAIL  the engine did not survive"; status=1; }
grep -E "rror" "$RUN/ingen.log" | sort | uniq -c | sort -rn | head -15
stop_engine
exit $status
