#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pedal PI contributors
# Both NAM candidates in the engine (P8.1 record, P8.2 host checks). Run as
# root inside Debian, after 12-nam.sh:
#   bash 13-nam.sh [a1-model.nam a2-model.nam]
# Without arguments it uses two captures from the unchanged Casette reference:
# an A1 standard WaveNet and an A2 slimmable container, both 48 kHz high-gain
# amps, so a running model is unmistakable in the measurement.
D="$(dirname "$(readlink -f "$0")")"
. "$D/env.sh"
MODELS="$D/../../reference/upstream/Casette/models"
A1=${1:-"$MODELS/EVH 5150 Stealth.nam"}
A2=${2:-"$MODELS/APP-6505+-Crunch-I.nam"}
BASE_PATH=$LV2_PATH
status=0

echo "==== models"
for m in "$A1" "$A2"; do
    python3 -c 'import json,sys,hashlib; p=sys.argv[1]; d=json.load(open(p)); c=d.get("config",{}); print("%s: %s %s, %s Hz, sha256 %s" % (p.rsplit("/",1)[-1], d.get("architecture"), d.get("version"), d.get("sample_rate"), hashlib.sha256(open(p,"rb").read()).hexdigest()[:16]))' "$m"
done

for c in current poly; do
    so=$PREFIX/nam/$c/neural_amp_modeler.lv2/neural_amp_modeler.so
    echo "==== $c: $(git -C "$PREFIX/src/nam-$c" rev-parse --short HEAD)"
    readelf -h "$so" | awk -F: '/Machine/ {gsub(/^ +/, "", $2); print "binary: " $2}'
    readelf -d "$so" | awk '/NEEDED/ {gsub(/[\[\]]/, "", $5); printf "needs %s\n", $5}'
    ls -l "$so" | awk '{printf "size %.1f MB\n", $5 / 1048576}'
    export LV2_PATH="$BASE_PATH:$PREFIX/nam/$c"
    lv2info http://github.com/mikeoliphant/neural-amp-modeler-lv2 | awk '/Required Features:/ {r=1; o=0; e=0; $1=$2=""} /Optional Features:/ {r=0; o=1; e=0; $1=$2=""} /Extension Data:/ {r=0; o=0; e=1; $1=$2=""} /Presets:|Port [0-9]|Parameters:|Data URIs:/ {r=0; o=0; e=0} r && NF {print "requires " $NF} o && NF {print "optional " $NF} e && NF {print "extension " $NF}'
    for period in 128 64 1024; do
        export PEDALPI_PERIOD=$period
        start_engine || exit 1
        if [ "$period" = 128 ]; then
            python3 "$D/13-nam.py" "$c" "$A1" "$A2" "/tmp/pedalpi-nam-$c" || status=1
            grep -E "rror|nable|arning" "$RUN/ingen.log" | sort | uniq -c | head -12
            # The saved patch, into an engine that has never seen the model.
            start_engine || exit 1
            python3 "$D/13-nam.py" --restore "$c" "/tmp/pedalpi-nam-$c/nam.ingen" || status=1
        else
            python3 "$D/13-nam.py" "$c" "$A1" "$A2" "/tmp/pedalpi-nam-$c-$period" --quick || status=1
        fi
        pgrep -x ingen >/dev/null && echo "PASS  the engine is still running" || { echo "FAIL  the engine did not survive"; status=1; }
    done
    stop_engine
done
exit $status
