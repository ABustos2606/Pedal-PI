#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
# Plaits and Marbles, measured in a fresh engine. Run as root inside Debian,
# after 02-plugins.sh (which builds Poly's Mutable-derived plugins):
#   bash 20-plaits-marbles.sh
D="$(dirname "$(readlink -f "$0")")"
. "$D/env.sh"
start_engine || exit 1
python3 "$D/20-plaits-marbles.py"; result=$?
grep -E "rror" "$RUN/ingen.log" | grep -v "Instrument.ttl" | sort | uniq -c | head -10
stop_engine
exit $result
