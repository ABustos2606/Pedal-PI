#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pedal PI contributors
#
# The Pi's build on the PC: build.sh in the Debian arm64 root of
# arm64-root.sh, under emulation. Run as root inside Debian (WSL2):
#   bash build-arm64.sh [--skip-engine] [--skip-tests]
# Emulation is slow (the engine takes most of an hour the first time) but
# needs no cross toolchain, and the tests run on the arm64 binaries.
# The package lands in build/pi/ of the repository.
set -euo pipefail
HERE="$(dirname "$(readlink -f "$0")")"
bash "$HERE/arm64-root.sh" >/dev/null
bash "$HERE/arm64-root.sh" bash -c 'apt-get update -qq && apt-get install -y -qq git ca-certificates >/dev/null'
exec bash "$HERE/arm64-root.sh" bash /work/pedalpi/tools/pi/build.sh "$@"
