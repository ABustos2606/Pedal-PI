#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
# Switch off the system services an instrument does not use, so it boots
# sooner and nothing else wakes up while it plays. SSH, Wi-Fi, avahi
# (name.local) and the clock stay on.
#   sudo peepo-slim-boot            switch them off (their policy is saved first)
#   sudo peepo-slim-boot --restore  put every saved policy back
#   peepo-slim-boot --dry-run ...   say what would be done
# Then reboot. Units that are not installed are skipped.
set -euo pipefail
STATE=/etc/peepo/slim-boot-state
UNITS=(
    NetworkManager-wait-online.service    # holds the boot until Wi-Fi is up
    ModemManager.service                  # mobile modems
    cups.service cups.socket cups.path cups-browsed.service  # printing
    bluetooth.service hciuart.service
    udisks2.service                       # desktop disk mounting
    accounts-daemon.service               # desktop user accounts
    triggerhappy.service triggerhappy.socket  # desktop hotkeys, reads the input devices
    wayvnc-control.service patchbox-vnc.target glamor-test.service rp1-test.service  # remote desktop and Xorg checks
    patchbox-init.service                 # starts Patchbox's own module (MODEP)
    blokas-telemetry.service blokas-telemetry.target blokas-telemetry-upd.timer
    pisound-btn.service pisound-ctl.service  # Pisound board, not fitted
    amidiminder.service                   # ALSA MIDI auto-connection
    dphys-swapfile.service                # swap on the SD card
    e2scrub_reap.service e2scrub_all.timer   # file system scrubbing
    apt-daily.timer apt-daily-upgrade.timer man-db.timer  # background updates and indexing
)
dry=0; restore=0
for arg in "$@"; do
    case $arg in
        --dry-run) dry=1 ;; --restore) restore=1 ;;
        *) echo "Usage: $0 [--dry-run] [--restore]" >&2; exit 2 ;;
    esac
done
run() { printf '+ '; printf '%q ' "$@"; printf '\n'; [ "$dry" = 1 ] || "$@"; }
[ "$dry" = 1 ] || [ "$(id -u)" = 0 ] || { echo 'Run as root (sudo).' >&2; exit 1; }

if [ "$restore" = 1 ]; then
    [ -f "$STATE/units" ] || { echo "Nothing saved in $STATE." >&2; exit 1; }
    while read -r unit policy; do
        run systemctl unmask "$unit"
        case $policy in
            enabled) run systemctl enable "$unit" ;;
            disabled|static|indirect|alias) : ;;
            *) echo "Unsupported saved policy: $unit $policy" >&2; exit 1 ;;
        esac
    done < "$STATE/units"
    run rm -rf "$STATE"
    echo 'Services restored. Reboot to start them again.'
    exit 0
fi

# Each unit's policy is saved the first time it is switched off, so running
# this again (after a system update adds one) never loses the original.
[ "$dry" = 1 ] || install -d -m 0755 "$STATE"
touch_state() { [ "$dry" = 1 ] || printf '%s %s\n' "$1" "$2" >> "$STATE/units"; }
for unit in "${UNITS[@]}"; do
    [ "$(systemctl show -p LoadState --value "$unit")" = loaded ] || continue
    policy=$(systemctl is-enabled "$unit" 2>/dev/null || true)
    [ "$policy" != masked ] || continue
    case $policy in
        enabled|disabled|static|indirect|alias) ;;
        *) echo "Skipping $unit: policy $policy cannot be restored." >&2; continue ;;
    esac
    grep -q "^$unit " "$STATE/units" 2>/dev/null || touch_state "$unit" "$policy"
    run systemctl disable --now "$unit"
    run systemctl mask "$unit"
done
if [ "$dry" = 1 ]; then echo 'Dry run: these services would be switched off.'
else echo 'Unused services off. Reboot to measure. Undo: sudo peepo-slim-boot --restore; sudo reboot'; fi
