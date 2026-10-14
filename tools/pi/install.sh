#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pedal PI contributors
#
# Install a Pedal PI package on the Pi. Run as root from the unpacked package:
#   sudo ./install.sh            install, keeping the previous one for rollback
#   sudo ./install.sh --enable   also start the instrument at every boot
#   sudo ./install.sh --instrument  direct boot, without desktop or MODEP
#   sudo ./install.sh --start    also start it now
#   sudo ./install.sh --rollback put the previous installation back
#   sudo ./install.sh --remove   stop and remove the services (keeps patches and settings)
#   ./install.sh --dry-run ...   say what would be done
# It never edits /boot/firmware/config.txt. Only --instrument stops and masks
# the desktop/MODEP; ordinary installation reports competing audio services.
set -euo pipefail
HERE="$(dirname "$(readlink -f "$0")")"
PREFIX=/opt/pedalpi
UNITS="pedalpi.target pedalpi-jack.service pedalpi-engine.service pedalpi-route.service pedalpi-ui.service"
dry=0; enable=0; start=0; instrument=0; action=install
for arg in "$@"; do
    case $arg in
        --dry-run) dry=1 ;; --enable) enable=1 ;; --start) start=1 ;;
        --instrument) instrument=1 ;;
        --rollback) action=rollback ;; --remove) action=remove ;;
        *) echo "unknown option $arg"; exit 2 ;;
    esac
done
[ $instrument = 0 ] || [ "$action" = install ] || { echo '--instrument is an installation option.'; exit 2; }
run() { echo "+ $*"; [ $dry = 1 ] || "$@"; }
[ $dry = 1 ] || [ "$(id -u)" = 0 ] || { echo "run as root (sudo)"; exit 1; }

if [ $action = remove ]; then
    if [ -f /etc/pedalpi/instrument-boot-state/units ] && [ -x "$PREFIX/bin/pedalpi-instrument-boot" ] && [ "$(systemctl show -p LoadState --value pedalpi.target)" = loaded ]; then
        run "$PREFIX/bin/pedalpi-instrument-boot" --restore
    fi
    for unit in $UNITS; do run systemctl disable --now "$unit" 2>/dev/null || true; run rm -f "/etc/systemd/system/$unit"; done
    run systemctl daemon-reload
    echo "Removed the services. $PREFIX, /etc/pedalpi and /var/lib/pedalpi are kept."
    exit 0
fi
if [ $action = rollback ]; then
    [ -d "$PREFIX.previous" ] || { echo "no previous installation at $PREFIX.previous"; exit 1; }
    run systemctl stop pedalpi.target 2>/dev/null || true
    run rm -rf "$PREFIX.failed"
    run mv "$PREFIX" "$PREFIX.failed"
    run mv "$PREFIX.previous" "$PREFIX"
    echo "Restored the previous installation; the replaced one is in $PREFIX.failed."
    exit 0
fi

# The package must match this machine.
package_arch=$(awk '{print $2}' "$HERE/VERSION")
machine_arch=$(dpkg --print-architecture)
[ "$package_arch" = "$machine_arch" ] || { echo "this package is for $package_arch, this machine is $machine_arch"; exit 1; }
echo "Installing Pedal PI $(cat "$HERE/VERSION")"

# What the programs need from the distribution (they were built against these).
DEPENDS="jackd2 liblilv-0-0 libserd-0-0 libsord-0-0 libsratom-0-0 libsuil-0-0 libsigc++-2.0-0v5 libjack-jackd2-0 libdrm2 mda-lv2 lv2-examples x42-plugins setbfree blop-lv2 fomp calf-plugins libzita-convolver4 libzita-resampler1 libsndfile1 libsamplerate0 libfftw3-single3 libcairo2 libx11-6 libxext6 libxrandr2 libxcursor1"
if apt-cache show jack-example-tools >/dev/null 2>&1; then DEPENDS="$DEPENDS jack-example-tools"; fi
run env DEBIAN_FRONTEND=noninteractive apt-get install -y -q --no-install-recommends $DEPENDS

# Anything else that may own the audio interface is reported, never stopped.
for other in modep-mod-host modep-mod-ui jack jackd pipewire pulseaudio; do
    if systemctl is-active --quiet "$other" 2>/dev/null; then
        echo "NOTE: $other is running and may hold the audio interface. Stop it yourself before starting Pedal PI."
    fi
done

# The service account, with access to audio, the screen and the controls.
if ! id pedalpi >/dev/null 2>&1; then
    groups=""
    for group in audio video render input gpio; do getent group "$group" >/dev/null && groups="$groups,$group"; done
    run useradd --system --home-dir /var/lib/pedalpi --shell /usr/sbin/nologin --groups "${groups#,}" pedalpi
fi
# Real-time scheduling for JACK and the engine is granted by the units (LimitRTPRIO).

# The programs: the previous installation stays as $PREFIX.previous.
if [ -d "$PREFIX" ]; then run rm -rf "$PREFIX.previous"; run mv "$PREFIX" "$PREFIX.previous"; fi
run cp -a "$HERE/root/opt/pedalpi" "$PREFIX"
run install -m 0755 "$HERE/config/bin/pedalpi-engine" "$HERE/config/bin/pedalpi-route" "$PREFIX/bin/"
run install -m 0755 "$HERE/export-catalogue.sh" "$PREFIX/bin/pedalpi-export-catalogue"
run install -m 0755 "$HERE/instrument-boot.sh" "$PREFIX/bin/pedalpi-instrument-boot"
run install -m 0755 "$HERE/check-ui-restart.sh" "$HERE/monitor-ui-restart.py" "$PREFIX/libexec/pedalpi/"
run "$PREFIX/bin/pedalpi-export-catalogue"

# Settings are the user's: installed once, never overwritten.
run install -d -m 0755 /etc/pedalpi
for file in pedalpi.conf controls.conf; do
    if [ -e "/etc/pedalpi/$file" ]; then echo "keeping /etc/pedalpi/$file (new default: $PREFIX/share/pedalpi/$file)"; fi
    run install -m 0644 "$HERE/config/$file" "$PREFIX/share/pedalpi/$file"
    [ -e "/etc/pedalpi/$file" ] || run install -m 0644 "$HERE/config/$file" "/etc/pedalpi/$file"
done
run install -m 0644 "$HERE/config/config.txt" "$PREFIX/share/pedalpi/config.txt"
run install -d -o pedalpi -g pedalpi -m 0755 /var/lib/pedalpi /var/lib/pedalpi/patches

for unit in $UNITS; do run install -m 0644 "$HERE/config/systemd/$unit" "/etc/systemd/system/$unit"; done
run systemctl daemon-reload
if [ $instrument = 1 ]; then
    if [ $dry = 1 ]; then run "$PREFIX/bin/pedalpi-instrument-boot" --dry-run
    else run "$PREFIX/bin/pedalpi-instrument-boot"; fi
fi
if [ $enable = 1 ]; then run systemctl enable pedalpi.target; fi
if [ $start = 1 ]; then run systemctl restart pedalpi.target; fi

cat <<EOF
Installed in $PREFIX; settings in /etc/pedalpi; patches in /var/lib/pedalpi/patches.
Wiring of the encoders and footswitches: add the lines of $PREFIX/share/pedalpi/config.txt
to /boot/firmware/config.txt yourself, then reboot.
Start now:          sudo systemctl start pedalpi.target
Start at every boot: sudo systemctl enable pedalpi.target
Direct instrument boot (saves previous desktop/MODEP policy): sudo $PREFIX/bin/pedalpi-instrument-boot
Restart only the interface (audio keeps playing): sudo systemctl restart pedalpi-ui
EOF
