#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
# Details behind three surprises: reply cadence, dead clients, save and reload.
. "$(dirname "$0")/env.sh"
cd "$(dirname "$0")"
start_engine || exit 1
python3 - <<'PY'
import os, subprocess, sys, time
import jack
sys.path.insert(0, os.getcwd())
from ingenprobe import Client

LEVEL = 0.25
class Meter:
    def __init__(self):
        self.client = jack.Client("probe"); self.out = self.client.outports.register("dc"); self.cap = self.client.inports.register("cap")
        self.blocks = []; self.xruns = 0
        self.client.set_process_callback(self.process); self.client.set_xrun_callback(lambda d: setattr(self, "xruns", self.xruns + 1))
        self.client.activate()
    def process(self, frames):
        self.out.get_array()[:] = LEVEL
        data = self.cap.get_array(); self.blocks.append((time.perf_counter(), float(data.max())))
m = Meter()
c = Client()
c.put("ingen:/main/in", "a lv2:InputPort , lv2:AudioPort"); c.put("ingen:/main/out", "a lv2:OutputPort , lv2:AudioPort")
c.put("ingen:/main/amp", "a ingen:Block ;\n\t\tlv2:prototype <http://lv2plug.in/plugins/eg-amp>")
c.connect("ingen:/main/in", "ingen:/main/amp/in"); c.connect("ingen:/main/amp/out", "ingen:/main/out")
time.sleep(0.3); m.client.connect("probe:dc", "ingen:in"); m.client.connect("ingen:out", "probe:cap")
c.set("ingen:/main/amp", "ingen:canvasX", "312.0"); c.set("ingen:/main/amp", "peepo:displayName", "\"Clean boost\"")
c.set("ingen:/main/amp/gain", "peepo:assignedControl", "\"knob-1\"")

def phase(name, seconds, work=None):
    before = m.xruns; start = time.time()
    if work: work()
    time.sleep(max(0, seconds - (time.time() - start)))
    print("xruns %-46s %d" % (name, m.xruns - before), flush=True)

phase("idle, 3 s", 3)
# How long until a control change is heard, versus until its reply arrives?
audible, replied = [], []
for target, gain in [(0.5, "6.0206"), (0.25, "0.0")] * 10:
    time.sleep(0.05)
    sent = time.perf_counter(); c.seq += 1
    c.send_raw("[]\n\ta patch:Set ;\n\tpatch:subject <ingen:/main/amp/gain> ;\n\tpatch:property ingen:value ;\n\tpatch:value %s ;\n\tpatch:sequenceNumber \"%d\"^^xsd:int .\n" % (gain, c.seq))
    heard = None
    while heard is None and time.perf_counter() - sent < 1:
        heard = next((t for t, level in m.blocks[-8:] if t > sent and abs(level - target) < 0.002), None)
        time.sleep(0.0005)
    audible.append((heard - sent) * 1000 if heard else float("nan"))
    c.read_message(1); replied.append((time.perf_counter() - sent) * 1000)
print("control change audible after  median %.1f ms, max %.1f ms" % (sorted(audible)[10], max(audible)))
print("its reply arrives after       median %.1f ms, max %.1f ms" % (sorted(replied)[10], max(replied)))
phase("200 control changes, one live client", 0, lambda: [c.set("ingen:/main/amp/gain", "ingen:value", "0.0") for _ in range(40)])

def log_errors():
    return open("/tmp/peepo-spike/ingen.log").read().count("write error")
dead = []
for _ in range(10):
    d = Client(); d.get("ingen:/main"); d.close()
# Writes already in flight when a client leaves fail once; let those settle.
[c.set("ingen:/main/amp/gain", "ingen:value", "0.0") for _ in range(40)]; time.sleep(2)
base = log_errors()
phase("40 control changes after 10 clients left", 2, lambda: [c.set("ingen:/main/amp/gain", "ingen:value", "0.0") for _ in range(40)])
print("engine 'write error' lines for those 40 changes: %d  (dead clients are %s)" % (log_errors() - base, "still written to" if log_errors() > base else "dropped"))

# Save: is there ever a reply? When is the bundle complete on disk?
bundle = "/tmp/peepo-spike/detail.ingen"; subprocess.run(["rm", "-rf", bundle])
start = time.time(); status = c.copy("ingen:/main", "file://" + bundle, timeout=4)[0]
print("save reply status: %s after %.1f s; bundle files: %s" % (status, time.time() - start, sorted(os.listdir(bundle)) if os.path.isdir(bundle) else "none"))
print("engine still answers after save:", c.get("ingen:/main/amp", timeout=3)[0])
print("---- main.ttl"); print(open(bundle + "/main.ttl").read())
print("---- manifest.ttl"); print(open(bundle + "/manifest.ttl").read())
c.close(); m.client.deactivate(); m.client.close()
PY

echo "==== reload the saved bundle in a brand new engine"
start_engine "$RUN/detail.ingen" || exit 1
sleep 1
python3 - <<'PY'
import os, sys, time
import jack
sys.path.insert(0, os.getcwd())
from ingenprobe import Client
c = Client(); status, reply, _ = c.get("ingen:/main")
for item in ["ingen:/main/amp>", "eg-amp", "ingen:tail", "canvasX", "Clean boost", "knob-1"]:
    print("restored %-18s %s" % (item, item in reply))
j = jack.Client("probe"); out = j.outports.register("dc"); cap = j.inports.register("cap"); seen = []
def process(frames):
    out.get_array()[:] = 0.25; seen.append(float(cap.get_array().max()))
j.set_process_callback(process); j.activate()
print("jack ports:", [p.name for p in j.get_ports("ingen:")])
try:
    j.connect("probe:dc", "ingen:in"); j.connect("ingen:out", "probe:cap"); time.sleep(0.4)
    print("audio through restored graph: %.4f (expected 0.2500)" % seen[-1])
except Exception as e:
    print("could not patch restored graph:", e)
j.deactivate(); j.close(); c.close()
PY
tail -4 "$RUN/ingen.log"
stop_engine
