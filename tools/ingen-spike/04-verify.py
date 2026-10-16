# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
"""Engine assumptions checked against a live Ingen on JACK's dummy backend.

A JACK client feeds a constant 0.25 into the graph and measures what comes
back, so routing, summing and continuity are numbers rather than impressions.
Hardware latency, xruns under load and real plugins are out of scope here.
"""
import os
import statistics
import subprocess
import sys
import time

import jack
import numpy

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ingenprobe import Client, SOCKET_PATH, engine_alive

LEVEL = 0.25
results = []


def report(kind, name, detail=""):
    results.append(kind)
    print("%-5s %s%s" % (kind, name, (" -- " + detail) if detail else ""), flush=True)


def check(ok, name, detail=""):
    report("PASS" if ok else "FAIL", name, detail)
    return ok


class Meter:
    """Feeds DC into the engine and keeps every captured block's extremes."""

    def __init__(self):
        self.client = jack.Client("probe")
        self.out = self.client.outports.register("dc")
        self.cap = self.client.inports.register("cap")
        self.blocks = []
        self.xruns = 0
        self.client.set_process_callback(self.process)
        self.client.set_xrun_callback(self.xrun)
        self.client.activate()

    def process(self, frames):
        self.out.get_array()[:] = LEVEL
        data = self.cap.get_array()
        self.blocks.append((float(data.min()), float(data.max())))

    def xrun(self, delay):
        self.xruns += 1

    def level(self, settle=0.25):
        time.sleep(settle)
        tail = self.blocks[-20:]
        return min(b[0] for b in tail), max(b[1] for b in tail)

    def mark(self):
        return len(self.blocks)

    def span(self, mark):
        tail = self.blocks[mark:]
        return min(b[0] for b in tail), max(b[1] for b in tail), len(tail)


def near(value, target):
    return abs(value - target) < 0.002


def steady(meter, target, name):
    low, high = meter.level()
    return check(near(low, target) and near(high, target), name, "measured %.4f..%.4f, expected %.4f" % (low, high, target))


def ok(reply, name):
    status = reply[0]
    return check(status == 0, name, "status %s" % status)


meter = Meter()
c = Client()

# --- graph building -------------------------------------------------------
ok(c.put("ingen:/main/in", "a lv2:InputPort , lv2:AudioPort"), "create graph audio input")
ok(c.put("ingen:/main/out", "a lv2:OutputPort , lv2:AudioPort"), "create graph audio output")
time.sleep(0.3)
ports = [p.name for p in meter.client.get_ports("ingen:")]
check("ingen:in" in ports and "ingen:out" in ports, "graph ports appear as JACK ports", ", ".join(ports))
meter.client.connect("probe:dc", "ingen:in")
meter.client.connect("ingen:out", "probe:cap")
steady(meter, 0.0, "unconnected graph is silent")

ok(c.connect("ingen:/main/in", "ingen:/main/out"), "connect input straight to output")
steady(meter, LEVEL, "pass-through level")
ok(c.disconnect("ingen:/main/in", "ingen:/main/out"), "disconnect")
steady(meter, 0.0, "silence after disconnect")

AMP = "a ingen:Block ;\n\t\tlv2:prototype <http://lv2plug.in/plugins/eg-amp>"
ok(c.put("ingen:/main/amp", AMP), "add plugin block")
check(c.put("ingen:/main/ghost", "a ingen:Block ;\n\t\tlv2:prototype <urn:does:not:exist>")[0] not in (0, None), "unknown plugin is refused with an error status")
ok(c.connect("ingen:/main/in", "ingen:/main/amp/in"), "cable into plugin")
ok(c.connect("ingen:/main/amp/out", "ingen:/main/out"), "cable out of plugin")
steady(meter, LEVEL, "series chain at 0 dB")
ok(c.set("ingen:/main/amp/gain", "ingen:value", "6.0206"), "set control value")
steady(meter, LEVEL * 2, "control change is audible (+6.02 dB doubles the level)")
ok(c.set("ingen:/main/amp/gain", "ingen:value", "0.0"), "restore control value")

# --- fan-out and fan-in ---------------------------------------------------
ok(c.put("ingen:/main/amp2", AMP), "add second block")
ok(c.connect("ingen:/main/in", "ingen:/main/amp2/in"), "fan-out: same source to a second block")
steady(meter, LEVEL, "fan-out does not change the first branch")
ok(c.connect("ingen:/main/amp2/out", "ingen:/main/out"), "fan-in: second cable into the same input")
low, high = meter.level()
check(near(low, LEVEL * 2) and near(high, LEVEL * 2), "fan-in sums at unity, with no compensation", "two 0.25 branches measured %.4f" % high)
report("INFO", "merge policy", "two equal branches are +6 dB; the UI needs a mixer/attenuation rule")

# --- feedback -------------------------------------------------------------
ok(c.set("ingen:/main/amp2/gain", "ingen:value", "-20.0"), "attenuate before feedback test")
status = c.connect("ingen:/main/amp2/out", "ingen:/main/amp2/in")[0]
report("INFO", "feedback cable", "engine returned status %s (0 means it accepts cycles; the UI refuses them itself)" % status)
if status == 0:
    c.disconnect("ingen:/main/amp2/out", "ingen:/main/amp2/in")
c.set("ingen:/main/amp2/gain", "ingen:value", "0.0")

# --- validation and metadata ---------------------------------------------
status = c.set("ingen:/main/amp/gain", "ingen:value", "\"loud\"")[0]
report("INFO", "string sent as a float control value", "status %s (0 = accepted: the engine does not type-check values)" % status)
c.set("ingen:/main/amp/gain", "ingen:value", "0.0")
status = c.set("ingen:/main/amp/gain", "ingen:value", "9999.0")[0]
low, high = meter.level()
report("INFO", "control value far outside the plugin range", "status %s, output now %s (the engine does not clamp; the client must)" % (status, high))
c.set("ingen:/main/amp/gain", "ingen:value", "0.0")
ok(c.set("ingen:/main/amp", "ingen:canvasX", "312.0"), "store canvas position on a block")
ok(c.set("ingen:/main/amp", "ingen:canvasY", "144.0"), "store canvas position (y)")
ok(c.set("ingen:/main/amp", "peepo:displayName", "\"Clean boost\""), "store a project-owned property on a block")
ok(c.set("ingen:/main/amp/gain", "peepo:assignedControl", "\"knob-1\""), "store a project-owned property on a port")
steady(meter, LEVEL * 2, "level unchanged by metadata edits")

# --- request timing -------------------------------------------------------
times = []
for i in range(200):
    start = time.perf_counter()
    c.set("ingen:/main/amp/gain", "ingen:value", "%.4f" % (-0.001 * (i % 2)))
    times.append((time.perf_counter() - start) * 1000)
c.set("ingen:/main/amp/gain", "ingen:value", "0.0")
report("INFO", "control round trip over the socket", "median %.2f ms, p95 %.2f ms, max %.2f ms (200 requests, 128-frame dummy period = 2.67 ms)"
       % (statistics.median(times), sorted(times)[189], max(times)))

# --- second client: who hears about a change? -----------------------------
watcher = Client()
watcher.get("ingen:/main")
watcher.drain(0.3)
c.set("ingen:/main/amp/gain", "ingen:value", "-0.5")
heard = watcher.drain(0.5)
check(any("amp/gain" in m for m in heard), "a second client is told about another client's change", "%d message(s)" % len(heard))
c.set("ingen:/main/amp/gain", "ingen:value", "0.0")
watcher.close()

# --- client restarts while audio runs -------------------------------------
mark = meter.mark(); xruns_before = meter.xruns
c.close()
time.sleep(0.5)
for _ in range(20):                      # orderly reconnects
    Client().close()
half = Client()                          # a client that dies mid-message
half.sock.sendall(b"[] a patch:Put ; patch:subject <ingen:/main/broken> ; patch:body [ a ingen:Bl")
half.close()
killed = subprocess.Popen([sys.executable, "-c",
    "import sys,time; sys.path.insert(0,%r); from ingenprobe import Client; c=Client(); c.get('ingen:/main'); time.sleep(30)" % os.path.dirname(os.path.abspath(__file__))])
time.sleep(0.5)
killed.kill(); killed.wait()             # SIGKILL: no orderly close at all
time.sleep(0.5)
check(os.path.exists(SOCKET_PATH) and engine_alive(), "engine survives client disconnects, a half-sent message and a killed client")
c = Client()
status, reply, _ = c.get("ingen:/main")
# Built against Debian 12's serd 0.30 (the Pi) the engine writes its paths
# relative to its base: </main/amp> for <ingen:/main/amp>.
reply = reply.replace("</main/", "<ingen:/main/")
expected = ["ingen:/main/amp>", "ingen:/main/amp2>", "ingen:/main/amp/gain>", "ingen:canvasX", "Clean boost", "knob-1", "ingen:Arc"]
missing = [item for item in expected if item not in reply]
check(status == 0 and not missing, "a new client reads back the whole live graph, including unsaved metadata", "missing: %s" % missing if missing else "blocks, ports, arcs, positions and custom properties present")
low, high, count = meter.span(mark)
check(near(low, LEVEL * 2) and near(high, LEVEL * 2), "audio level never moves across client restarts",
      "%d blocks, level %.4f..%.4f, %d xruns in this phase (%d earlier, during graph edits)" % (count, low, high, meter.xruns - xruns_before, xruns_before))

report("INFO", "xruns", "%d in total; WSL has no real-time scheduling and the dummy backend also xruns when idle, so this number means nothing until it is measured on the Pi" % meter.xruns)

# --- save -----------------------------------------------------------------
bundle = "/tmp/peepo-spike/saved.ingen"
subprocess.run(["rm", "-rf", bundle])
ok(c.copy("ingen:/main", "file://" + bundle), "save the graph to a bundle")
time.sleep(0.3)
listing = []
for root, _, files in os.walk(bundle):
    listing += [os.path.relpath(os.path.join(root, f), bundle) for f in files]
check(os.path.isdir(bundle) and "main.ttl" in listing and "manifest.ttl" in listing, "the save is a directory bundle", ", ".join(sorted(listing)))
if os.path.isfile(os.path.join(bundle, "main.ttl")):
    saved = open(os.path.join(bundle, "main.ttl")).read()
    kept = {k: (k in saved) for k in ["canvasX", "Clean boost", "knob-1", "ingen:tail", "eg-amp"]}
    check(all(kept.values()), "positions, project properties and cables are inside the saved bundle", str(kept))
c.close()
meter.client.deactivate()
meter.client.close()
print("SUMMARY %d passed, %d failed, %d informational" % (results.count("PASS"), results.count("FAIL"), results.count("INFO")))
