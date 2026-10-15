#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
# Show the Peepo boot screen instead of Linux's boot messages, from power-on
# until the touch interface draws its first screen.
#   sudo peepo-splash             switch it on (the previous boot files are saved)
#   sudo peepo-splash --restore   put the saved boot files back
#   peepo-splash --dry-run ...    say what would be done
#   --theme DIR                   the theme to install (default: the installed one)
# Then reboot. It changes /boot/firmware/cmdline.txt (quiet boot, console moved
# to tty3) and adds a marked block to config.txt (no rainbow square).
set -euo pipefail
HERE="$(dirname "$(readlink -f "$0")")"
STATE=/etc/peepo/splash-state
BOOT=/boot/firmware
THEMES=/usr/share/plymouth/themes
MARK='# peepo-splash'
QUIET="quiet splash loglevel=3 plymouth.ignore-serial-consoles logo.nologo vt.global_cursor_default=0"
dry=0; restore=0; theme=
for d in "$HERE/../share/peepo/splash/peepo" "$HERE/../config/pi/splash/peepo"; do [ -d "$d" ] && theme=${theme:-$d}; done
while [ $# -gt 0 ]; do
    case $1 in
        --dry-run) dry=1 ;;
        --restore) restore=1 ;;
        --theme) shift; theme=${1:?--theme needs a directory} ;;
        *) echo "Usage: $0 [--dry-run] [--restore] [--theme DIR]" >&2; exit 2 ;;
    esac
    shift
done
run() { printf '+ '; printf '%q ' "$@"; printf '\n'; [ "$dry" = 1 ] || "$@"; }
write() { echo "+ write $1"; if [ "$dry" = 1 ]; then cat >/dev/null; else cat > "$1"; fi; }
append() { echo "+ append to $1"; if [ "$dry" = 1 ]; then cat >/dev/null; else cat >> "$1"; fi; }
[ "$dry" = 1 ] || [ "$(id -u)" = 0 ] || { echo 'Run as root (sudo).' >&2; exit 1; }
[ -f "$BOOT/cmdline.txt" ] && [ -f "$BOOT/config.txt" ] || { echo "No Raspberry Pi boot files in $BOOT." >&2; exit 1; }
# The interface takes the screen over from the splash.
ui=peepo-ui.service
[ "$(systemctl show -p LoadState --value "$ui")" = loaded ] || { echo 'Install Peepo first.' >&2; exit 1; }

if [ "$restore" = 1 ]; then
    [ -f "$STATE/cmdline.txt" ] || { echo "Nothing saved in $STATE." >&2; exit 1; }
    # Only our own cmdline is put back, so a later edit is never lost silently.
    if ! cmp -s "$BOOT/cmdline.txt" "$STATE/cmdline.peepo"; then
        echo "$BOOT/cmdline.txt changed since peepo-splash wrote it; restore it by hand from $STATE/cmdline.txt." >&2; exit 1
    fi
    run cp "$STATE/cmdline.txt" "$BOOT/cmdline.txt"
    run sed -i "/^$MARK begin\$/,/^$MARK end\$/d" "$BOOT/config.txt"
    if [ -f "$STATE/plymouthd.conf" ]; then run cp "$STATE/plymouthd.conf" /etc/plymouth/plymouthd.conf
    else run rm -f /etc/plymouth/plymouthd.conf; fi
    run rm -f /etc/systemd/system/plymouth-quit.service.d/peepo.conf "/etc/systemd/system/$ui.d/splash.conf"
    run systemctl daemon-reload
    run update-initramfs -u
    run rm -rf "$STATE"
    echo 'Previous boot screen restored. Reboot to see it.'
    exit 0
fi

[ -n "$theme" ] && [ -f "$theme/peepo.plymouth" ] && [ -f "$theme/logo.png" ] || { echo 'No Peepo theme found; give it with --theme DIR.' >&2; exit 1; }
command -v plymouth >/dev/null && [ -x /usr/sbin/update-initramfs ] || { echo 'Needs plymouth and initramfs-tools.' >&2; exit 1; }
ls /usr/lib/*/plymouth/script.so >/dev/null 2>&1 || { echo "Needs Plymouth's script module (package plymouth-themes)." >&2; exit 1; }

if [ ! -f "$STATE/cmdline.txt" ]; then
    echo "Saving the previous boot files in $STATE"
    run install -d -m 0755 "$STATE"
    run cp "$BOOT/cmdline.txt" "$STATE/cmdline.txt"
    [ ! -f /etc/plymouth/plymouthd.conf ] || run cp /etc/plymouth/plymouthd.conf "$STATE/plymouthd.conf"
fi

run install -d -m 0755 "$THEMES/peepo"
run install -m 0644 "$theme/peepo.plymouth" "$theme/peepo.script" "$theme/logo.png" "$THEMES/peepo/"
printf '[Daemon]\nTheme=peepo\nShowDelay=0\n' | write /etc/plymouth/plymouthd.conf

# One line: boot messages and the cursor off the screen, the console on tty3.
cmdline=$(tr -d '\n' < "$BOOT/cmdline.txt" | sed 's/console=tty1\b/console=tty3/')
for word in $QUIET; do [[ " $cmdline " == *" $word "* ]] || cmdline="$cmdline $word"; done
printf '%s\n' "$cmdline" | write "$BOOT/cmdline.txt"
[ "$dry" = 1 ] || cp "$BOOT/cmdline.txt" "$STATE/cmdline.peepo"
grep -qx "$MARK begin" "$BOOT/config.txt" || printf '%s begin\n[all]\ndisable_splash=1\n%s end\n' "$MARK" "$MARK" | append "$BOOT/config.txt"

# Keep the logo until the interface takes the screen: Plymouth quits leaving
# its last frame up, and only once the interface is starting.
run install -d -m 0755 /etc/systemd/system/plymouth-quit.service.d "/etc/systemd/system/$ui.d"
write /etc/systemd/system/plymouth-quit.service.d/peepo.conf <<EOF
[Unit]
After=$ui

[Service]
ExecStart=
ExecStart=-/usr/bin/plymouth quit --retain-splash
EOF
write "/etc/systemd/system/$ui.d/splash.conf" <<'EOF'
[Unit]
After=plymouth-start.service

[Service]
# As root ("+"): Plymouth ignores a quit from the service user.
ExecStartPre=-+/usr/bin/plymouth quit --retain-splash
EOF
run systemctl daemon-reload
# The splash starts from the initramfs, before the root file system.
run update-initramfs -u
if [ "$dry" = 1 ]; then echo 'Dry run: the Peepo boot screen would be switched on.'
else echo 'Peepo boot screen on. Reboot to see it. Undo: sudo peepo-splash --restore; sudo reboot'; fi
