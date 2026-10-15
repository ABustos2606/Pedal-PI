#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
# Clouds' build record, binary and host checks, and test-signal checks
# (P6.1, P6.2, P6.4 in part). Run as root inside Debian, after 02-plugins.sh:
#   bash 11-clouds.sh
# With PEEPO_ARM64_ROOT set to an arm64 root built by tools/pi, the binary
# record covers that root's build as well.
D="$(dirname "$(readlink -f "$0")")"
. "$D/env.sh"
SRC=$PREFIX/src/lenticular_lv2
BUNDLE=$PREFIX/lib/lv2/polylenticular.lv2
ARM=${PEEPO_ARM64_ROOT:-/opt/peepo-arm64}

echo "==== build record"
echo "lenticular_lv2 $(git -C "$SRC" rev-parse HEAD)"
for part in eurorack eurorack/stmlib parasites parasites/stmlib; do
    [ -d "$SRC/$part/.git" ] || [ -f "$SRC/$part/.git" ] && echo "$part $(git -C "$SRC/$part" rev-parse HEAD) ($(git -C "$SRC/$part" remote get-url origin 2>/dev/null))"
done
git -C "$SRC" status --porcelain --ignore-submodules=none | sed 's/^/changed in the clone: /'
grep -m1 -oE "polyclouds\.cpp.*" "$PREFIX/src/lenticular-build.log" | head -c 600; echo

binary() { # root label
    so="$1$PREFIX/lib/lv2/polylenticular.lv2/polyclouds.so"
    [ -f "$so" ] || { echo "$2: no polyclouds.so under $1$PREFIX"; return; }
    echo "---- $2"
    readelf -h "$so" | grep -E "Class" | sed "s/^ *//"
    sha256sum "$so" | cut -c1-16
    readelf -d "$so" | awk '/NEEDED/ {gsub(/[\[\]]/, "", $5); printf "needs %s\n", $5}'
    readelf -h "$so" | grep -E "Machine" | sed "s/^ *//"
    # Undefined symbols beyond the C and C++ runtimes would be a hidden dependency.
    nm -D --undefined-only "$so" | awk '{print $2}' | grep -vE "^(_Z|__|mem|str|sin|cos|exp|log|pow|sqrt|floor|ceil|fabs|f?printf|malloc|calloc|free|abort|atan|tan|round|lrint|fmod|operator|_Unwind|lv2_)" | sed 's/^/other undefined: /' | head
}
echo "==== binaries"
binary "" "x86-64 (this engine)"
binary "$ARM" "arm64 root"
[ -x /usr/bin/qemu-aarch64-static ] || [ -e /proc/sys/fs/binfmt_misc/qemu-aarch64 ] && [ -d "$ARM" ] && \
    chroot "$ARM" /bin/sh -c "ldd $PREFIX/lib/lv2/polylenticular.lv2/polyclouds.so" 2>&1 | sed 's/^/arm64 ldd: /'
ldd "$BUNDLE/polyclouds.so" | sed 's/^/x86-64 ldd: /'

echo "==== metadata as the host reads it"
for mode in granular stretch looping_delay spectral oliverb resonestor beat_repeat; do
    uri="http://polyeffects.com/lv2/polyclouds#$mode"
    lv2info "$uri" | awk -v m="$mode" '
        # lv2info puts the first feature on the heading line itself.
        /Required Features:/ {r=1; o=0; $1=$2=""} /Optional Features:/ {r=0; o=1; $1=$2=""} /Extension Data:|Presets:|Port [0-9]|Data URIs:/ {r=0; o=0}
        r && NF {req=req " " $NF} o && NF {opt=opt " " $NF}
        /Type:.*AudioPort/ {a++} /Type:.*CVPort/ {cv++} /Type:.*ControlPort/ {ctl++}
        END {printf "%-13s audio %d, CV %d, control %d; required:%s; optional:%s\n", m, a, cv, ctl, req ? req : " none", opt ? opt : " none"}'
done

echo "==== in the engine"
start_engine || exit 1
python3 "$D/11-clouds.py"; status=$?
pgrep -x ingen >/dev/null && echo "PASS  the engine is still running" || { echo "FAIL  the engine did not survive"; status=1; }
grep -E "rror|arning" "$RUN/ingen.log" | sort | uniq -c | head
stop_engine
exit $status
