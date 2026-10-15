#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
#
# Check an installed Peepo's services without audio hardware: JACK on its
# dummy driver and the interface drawing offscreen. Starts the instrument,
# builds a patch through the engine, then measures that audio keeps flowing
# while the interface is killed, and that each part comes back on its own.
# Run as root after install.sh (both arguments default to the installed ones):
#   bash check-services.sh /path/to/peepo_ingen_live /path/to/catalogue.json
# The real settings are put back afterwards.
set -uo pipefail
LIVE=${1:-/opt/peepo/libexec/peepo/peepo_ingen_live}
CATALOGUE=${2:-/opt/peepo/share/peepo/catalogue.json}
CONF=/etc/peepo/peepo.conf
failures=0
pass() { echo "PASS  $*"; }
fail() { echo "FAIL  $*"; failures=$((failures + 1)); }
pid_of() { systemctl show -p MainPID --value "$1"; }
as_peepo() { runuser -u peepo -- "$@"; }
wait_for() { for _ in $(seq "$1"); do eval "$2" && return 0; sleep 0.5; done; return 1; }

python3 -c "import jack, numpy" 2>/dev/null || DEBIAN_FRONTEND=noninteractive apt-get install -y -qq python3-jack-client python3-numpy >/dev/null
cp "$CONF" "$CONF.check-backup"
trap 'systemctl stop peepo.target peepo-jack.service 2>/dev/null; mv "$CONF.check-backup" "$CONF"' EXIT
sed -i -e 's|^JACK_DRIVER=.*|JACK_DRIVER=dummy|' -e 's|^JACK_DRIVER_OPTIONS=.*|JACK_DRIVER_OPTIONS=-r 48000 -p 128|' \
    -e 's|^JACK_OPTIONS=.*|JACK_OPTIONS=|' -e 's|^UI_OPTIONS=.*|UI_OPTIONS=--display offscreen|' "$CONF"

systemctl stop peepo.target peepo-jack.service 2>/dev/null
systemctl start peepo.target
if wait_for 40 '[ "$(systemctl is-active peepo-ui)" = active ] && [ -S /run/peepo/ingen.sock ]'; then pass "the instrument starts: JACK, engine and interface active"
else fail "the instrument did not start"; systemctl --no-pager status peepo-jack peepo-engine peepo-ui | tail -30; exit 1; fi
wait_for 20 'ss -xp | grep -q peepo' && pass "the interface is connected to the engine's socket" || fail "the interface is not connected"
if ss -tlnp | grep -q ingen; then fail "the engine listens on TCP"; else pass "the engine listens on no TCP port"; fi
wait_for 30 'as_peepo jack_lsp -c ingen:in_l 2>/dev/null | grep -q system:capture_1' && pass "the engine is wired to the audio interface's ports" || fail "routing did not happen"

# A patch through the engine, as the interface would make it: IN -> amp +6 dB -> OUT (left).
# Its patches go in a folder of its own that the service account owns.
install -d -o peepo -g peepo /var/lib/peepo/check-patches
as_peepo env PEEPO_PATCHES=/var/lib/peepo/check-patches "$LIVE" unix:///run/peepo/ingen.sock "$CATALOGUE" > /var/tmp/peepo-check-live.log 2>&1
rm -rf /var/lib/peepo/check-patches
if grep -q "SUMMARY all adapter checks passed" /var/tmp/peepo-check-live.log; then pass "a patch was built through the installed engine"
else fail "building a patch failed:"; grep -v "^PASS" /var/tmp/peepo-check-live.log | head -12; fi

measure() {
    as_peepo python3 - <<'PY'
import time, jack
c = jack.Client("check"); out = c.outports.register("dc"); cap = c.inports.register("cap"); seen = []
def process(frames):
    out.get_array()[:] = 0.25; seen.append(float(cap.get_array().max()))
c.set_process_callback(process); c.activate()
c.connect("check:dc", "ingen:in_l"); c.connect("ingen:out_l", "check:cap")
time.sleep(1.5)
start = len(seen); time.sleep(2.0); window = seen[start:]
# Last level, periods measured, periods off by more than 0.002, near-silent periods, lowest level.
print("%.4f %d %d %d %.4f" % (window[-1] if window else 0, len(window), sum(1 for v in window if abs(v - 0.5) > 0.002), sum(1 for v in window if v < 0.05), min(window) if window else 0))
c.deactivate(); c.close()
PY
}
read -r level blocks off silent lowest <<< "$(measure)"
[ "${silent:-1}" = 0 ] && [ "$level" = 0.5000 ] && pass "audio passes through the patch: 0.25 in, $level out over $blocks periods ($off off by more than 0.002, lowest $lowest)" || fail "audio level $level; $silent silent and $off off of $blocks periods"

# The interface dies while audio plays: the engine and JACK keep running, audio does not stop.
engine=$(pid_of peepo-engine); jackd=$(pid_of peepo-jack); ui=$(pid_of peepo-ui)
# Killed half a second into the two measured seconds.
( sleep 2.0; systemctl kill -s SIGKILL peepo-ui ) &
read -r level blocks off silent lowest <<< "$(measure)"
wait
[ "${silent:-1}" = 0 ] && pass "audio never stopped while the interface was killed: $blocks periods, none silent ($off off by more than 0.002, lowest $lowest)" || fail "audio dropped while the interface was killed: $silent of $blocks periods silent"
wait_for 20 '[ "$(systemctl is-active peepo-ui)" = active ] && [ "$(pid_of peepo-ui)" != "$ui" ]' && pass "the interface restarted by itself" || fail "the interface did not restart"
[ "$(pid_of peepo-engine)" = "$engine" ] && [ "$(pid_of peepo-jack)" = "$jackd" ] && pass "the engine and JACK were not restarted" || fail "the engine or JACK restarted with the interface"
wait_for 20 'ss -xp | grep -q peepo' && pass "the restarted interface reconnected" || fail "the restarted interface did not reconnect"

# The engine restarts: the interface is not restarted, and reconnects; routing is redone.
ui=$(pid_of peepo-ui)
systemctl restart peepo-engine
wait_for 30 '[ -S /run/peepo/ingen.sock ] && ss -xp | grep -q peepo' && pass "after an engine restart the interface reconnected" || fail "no reconnection after an engine restart"
[ "$(pid_of peepo-ui)" = "$ui" ] && pass "an engine restart did not restart the interface" || fail "the interface restarted with the engine"
wait_for 30 'as_peepo jack_lsp -c ingen:in_l 2>/dev/null | grep -q system:capture_1' && pass "routing was redone for the new engine" || fail "routing not redone"

systemctl stop peepo.target peepo-jack.service
sleep 1
[ "$(systemctl is-active peepo-engine)" != active ] && [ "$(systemctl is-active peepo-ui)" != active ] && pass "stopping the instrument stops every part" || fail "something kept running"
echo "SUMMARY $([ $failures = 0 ] && echo "all service checks passed" || echo "$failures failed")"
exit $failures
