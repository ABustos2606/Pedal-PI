#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
# Beebo's Delay and Peepo's Foot switch, measured in a fresh engine. Run as
# root inside Debian, after 14-more-plugins.sh (which builds both):
#   bash 19-delay-footswitch.sh
D="$(dirname "$(readlink -f "$0")")"
. "$D/env.sh"
start_engine || exit 1
python3 "$D/19-delay-footswitch.py"; result=$?
grep -E "rror" "$RUN/ingen.log" | sort | uniq -c | head -10
stop_engine
exit $result
