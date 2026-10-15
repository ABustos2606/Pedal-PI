#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Export the plugins installed on this instrument, using the engine's paths.
set -euo pipefail
PREFIX=/opt/peepo
export LD_LIBRARY_PATH="$(find "$PREFIX/lib" -maxdepth 2 -name 'libingen*' -printf '%h\n' | sort -u | paste -sd:)"
export LV2_PATH="$(find "$PREFIX/lib" -maxdepth 2 -name lv2 -type d | paste -sd:):$PREFIX/nam/current:/usr/lib/lv2"
DEST="$PREFIX/share/peepo/catalogue.json"
TEMP=$(mktemp "${DEST}.XXXXXX")
trap 'rm -f "$TEMP"' EXIT
"$PREFIX/bin/peepo_catalogue_export" --sample-rate 48000 --only "$PREFIX/share/peepo/catalogue-inventory.txt" -o "$TEMP"
chmod 0644 "$TEMP"
mv -f "$TEMP" "$DEST"
