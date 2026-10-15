#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
# What the engine writes when it saves a graph, and what it does when it loads
# one back while running (P5.2). Run as root inside Debian:  bash 10-bundle-probe.sh
# The last step leaves a client that never reads, which stalls the engine; the
# engine is killed afterwards, so run this on its own.
D="$(dirname "$(readlink -f "$0")")"
. "$D/env.sh"
start_engine || exit 1
cd "$D"
timeout 90 python3 10-bundle-probe.py "$RUN/bundles"; status=$?
echo "==== engine log"
grep -v "Broken pipe\|client" "$RUN/ingen.log" | tail -n +6 | sort | uniq -c
pkill -9 -x ingen; pkill -x jackd
exit $status
