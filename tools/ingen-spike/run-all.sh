#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pedal PI contributors
# Build (if needed) and run every engine check. Run as root inside Debian:
#   bash run-all.sh
D="$(dirname "$(readlink -f "$0")")"
. "$D/env.sh"
bash "$D/01-build.sh" 2>&1 | grep -E "^applied|error|FAILED|^(serd|sord|sratom|suil|lv2|lilv|raul|jack) "
start_engine || exit 1
python3 "$D/04-verify.py"
bash "$D/06-details.sh" | grep -vE "^(@prefix|\s|<|$|-{4})"
stop_engine
