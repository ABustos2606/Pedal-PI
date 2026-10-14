#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pedal PI contributors
# Switch an installed Pi to direct DRM instrument startup. No packages or
# patches are removed. --restore restores the saved boot policy; then reboot.
set -euo pipefail
STATE=/etc/pedalpi/instrument-boot-state
PATCHES=/var/lib/pedalpi/patches
dry=0; restore=0; startup=
while [ $# -gt 0 ]; do
    case $1 in
        --dry-run) dry=1 ;;
        --restore) restore=1 ;;
        --startup) shift; startup=${1:?--startup needs a saved patch identity} ;;
        *) echo "Usage: $0 [--dry-run] [--startup ID | --restore]" >&2; exit 2 ;;
    esac
    shift
done
run() { printf '+ '; printf '%q ' "$@"; printf '\n'; [ "$dry" = 1 ] || "$@"; }
[ "$dry" = 1 ] || [ "$(id -u)" = 0 ] || { echo 'Run as root (sudo).' >&2; exit 1; }
[ "$restore" = 0 ] || [ -z "$startup" ] || { echo '--restore cannot select a startup patch.' >&2; exit 2; }
if [ "$dry" = 0 ]; then
    exec 9>/run/pedalpi-boot.lock
    flock -n 9 || { echo 'Another boot-policy change is running.' >&2; exit 1; }
fi

if [ "$restore" = 1 ]; then
    [ -f "$STATE/units" ] && [ -f "$STATE/default-target" ] || { echo "No saved boot policy in $STATE." >&2; exit 1; }
    run systemctl stop pedalpi.target
    while read -r unit policy; do
        run systemctl unmask "$unit"
        case $policy in
            enabled) run systemctl enable "$unit" ;;
            enabled-runtime) run systemctl enable --runtime "$unit" ;;
            disabled) run systemctl disable "$unit" ;;
            masked) run systemctl mask "$unit" ;;
            masked-runtime) run systemctl mask --runtime "$unit" ;;
            static|indirect|alias) : ;;
            *) echo "Unsupported saved policy: $unit $policy" >&2; exit 1 ;;
        esac
    done < "$STATE/units"
    run systemctl set-default "$(cat "$STATE/default-target")"
    if [ "$dry" = 1 ]; then echo 'Dry run: previous boot policy would be restored.'
    else echo 'Previous boot policy restored. Reboot to return to it. Patches and startup choice are kept.'; fi
    exit 0
fi

[ "$(systemctl show -p LoadState --value pedalpi.target)" = loaded ] || { echo 'Install Pedal PI first.' >&2; exit 1; }
[ -x /opt/pedalpi/bin/pedal_pi ] && [ -r /etc/pedalpi/pedalpi.conf ] || { echo 'Pedal PI installation is incomplete.' >&2; exit 1; }
# A direct display is essential: a desktop/SDL service cannot be an instrument.
grep -Eq '^UI_OPTIONS=.*--display[[:space:]]+(drm|fbdev)([[:space:]]|$)' /etc/pedalpi/pedalpi.conf || {
    echo 'Set UI_OPTIONS=--display drm (or fbdev) in /etc/pedalpi/pedalpi.conf first.' >&2; exit 1;
}
if [ -n "$startup" ]; then
    [[ "$startup" =~ ^[a-z0-9-]+$ ]] && [ "${#startup}" -le 64 ] || { echo 'Invalid saved patch identity.' >&2; exit 1; }
    [ -d "$PATCHES/$startup" ] && find "$PATCHES/$startup" -mindepth 2 -maxdepth 2 -name manifest.ttl -print -quit | grep -q . || {
        echo "No saved revision for $startup." >&2; exit 1;
    }
fi

# Include MODEP helpers and its update trigger, not just its web UI and host.
mapfile -t competitors < <(systemctl list-unit-files --no-legend --no-pager 'modep-*' 'jack.service' 'lightdm.service' 'getty@tty1.service' | awk '{print $1}')
# Instantiated getty units are not necessarily listed by list-unit-files.
if ! printf '%s\n' "${competitors[@]}" | grep -qx getty@tty1.service; then competitors+=(getty@tty1.service); fi
if [ ! -f "$STATE/units" ]; then
    echo "Saving previous boot policy in $STATE"
    if [ "$dry" = 0 ]; then
        install -d -m 0755 /etc/pedalpi
        temporary=$(mktemp -d "$STATE.tmp.XXXXXX")
        systemctl get-default > "$temporary/default-target"
        for unit in pedalpi.target "${competitors[@]}"; do
            policy=$(systemctl is-enabled "$unit" 2>/dev/null || true)
            case $policy in
                enabled|enabled-runtime|disabled|masked|masked-runtime|static|indirect|alias) ;;
                *) echo "Cannot preserve $unit policy: $policy" >&2; exit 1 ;;
            esac
            printf '%s %s\n' "$unit" "$policy" >> "$temporary/units"
        done
        mv "$temporary" "$STATE"
    fi
fi
if [ -n "$startup" ]; then
    echo "Startup patch: $startup (latest saved revision)"
    if [ "$dry" = 0 ]; then
        temporary=$(mktemp "$PATCHES/.startup-boot.XXXXXX")
        printf '%s\n' "$startup" > "$temporary"
        chown pedalpi:pedalpi "$temporary"
        chmod 0644 "$temporary"
        mv "$temporary" "$PATCHES/startup"
    fi
fi
# Masking also prevents another enabled unit from pulling MODEP or LightDM in.
# Stop the helpers before JACK. Pedal PI's existing audio services are untouched.
for unit in "${competitors[@]}"; do run systemctl disable --now "$unit"; run systemctl mask "$unit"; done
run systemctl set-default multi-user.target
run systemctl enable pedalpi.target
run systemctl reset-failed pedalpi-jack.service pedalpi-engine.service pedalpi-ui.service
run systemctl start pedalpi.target
if [ "$dry" = 1 ]; then echo 'Dry run: direct Pedal PI boot would be enabled.'
else echo 'Direct Pedal PI boot enabled. SSH remains available. Reboot to verify unattended startup.'; fi
echo 'Restore: sudo /opt/pedalpi/bin/pedalpi-instrument-boot --restore; sudo reboot'
