#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
#
# A Debian arm64 root for building the Pi's binaries on the PC, and for
# running them under emulation. Run as root inside Debian (WSL2):
#   bash arm64-root.sh            create it once, or refresh its mounts
#   bash arm64-root.sh COMMAND    run COMMAND inside it
# The suite defaults to bookworm (Debian 12), the base of Raspberry Pi OS
# bookworm. Choose the Pi's actual release once Phase 0 has identified it:
# binaries must be built against the C library the Pi has, or older.
# The repository is visible inside at /work/peepo.
set -euo pipefail
ROOT=${PEEPO_ARM64_ROOT:-/opt/peepo-arm64}
SUITE=${PEEPO_SUITE:-bookworm}
REPO="$(readlink -f "$(dirname "$(readlink -f "$0")")/../..")"

if [ ! -x "$ROOT/usr/bin/apt-get" ]; then
    export DEBIAN_FRONTEND=noninteractive
    apt-get install -y -qq qemu-user-static binfmt-support debootstrap >/dev/null
    # The emulator must be registered before the foreign root can run anything.
    update-binfmts --enable qemu-aarch64 2>/dev/null || systemctl restart systemd-binfmt 2>/dev/null || true
    debootstrap --arch=arm64 --variant=minbase "$SUITE" "$ROOT" http://deb.debian.org/debian
    echo "peepo-arm64" > "$ROOT/etc/hostname"
fi
[ -e /proc/sys/fs/binfmt_misc/qemu-aarch64 ] || { echo "qemu-aarch64 is not registered with binfmt_misc"; exit 1; }

mounted() { mountpoint -q "$1"; }
mounted "$ROOT/proc" || mount -t proc proc "$ROOT/proc"
mounted "$ROOT/sys" || mount --rbind /sys "$ROOT/sys"
mounted "$ROOT/dev" || mount --rbind /dev "$ROOT/dev"
mkdir -p "$ROOT/work/peepo" "$ROOT/tmp"
mounted "$ROOT/work/peepo" || mount --bind "$REPO" "$ROOT/work/peepo"
cp -f /etc/resolv.conf "$ROOT/etc/resolv.conf" 2>/dev/null || true

if [ $# -gt 0 ]; then
    exec chroot "$ROOT" /usr/bin/env -i HOME=/root PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin TERM="${TERM:-dumb}" "$@"
fi
chroot "$ROOT" /usr/bin/uname -m
