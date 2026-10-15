#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
# Run 16-instance-race.py against a fresh engine. Run as root inside Debian:
#   bash 16-instance-race.sh [rounds]
D="$(dirname "$(readlink -f "$0")")"
. "$D/env.sh"
start_engine || exit 1
python3 "$D/16-instance-race.py" "$@"; status=$?
stop_engine
exit $status
