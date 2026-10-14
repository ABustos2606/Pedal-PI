# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pedal PI contributors
"""Save a graph to a bundle and load it back into a running engine (P5.2).

Answers what Phase 5 needs before any client code is written: what a saved
bundle contains, whether a running engine loads one into its root graph,
what the loading client is told, whether graph properties survive, what
happens to a block whose plugin is missing, and whether plugin state goes in
the bundle. Run with a fresh engine:  python3 10-bundle-probe.py <dir>
"""
import os
import shutil
import subprocess
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ingenprobe import Client, show

root = sys.argv[1] if len(sys.argv) > 1 else "/tmp/pedalpi-spike/bundles"
shutil.rmtree(root, ignore_errors=True)
os.makedirs(os.path.join(root, "with space"))
transcript = []
c = Client(transcript=transcript)
ctx = "patch:context ingen:internalContext ;\n\t"

def check(label, status):
    print("%-58s %s" % (label, status))

check("put in_l, out_l", (c.put("ingen:/main/in_l", "a lv2:InputPort , lv2:AudioPort")[0], c.put("ingen:/main/out_l", "a lv2:OutputPort , lv2:AudioPort")[0]))
check("put amp m3 with name, position", c.put("ingen:/main/m3", "a ingen:Block ;\n\t\tlv2:prototype <http://lv2plug.in/plugins/eg-amp> ;\n\t\tingen:canvasX \"300\"^^xsd:float ;\n\t\tingen:canvasY \"200\"^^xsd:float ;\n\t\tpedalpi:displayName \"Boost\"")[0])
check("put params m4 (has LV2 state)", c.put("ingen:/main/m4", "a ingen:Block ;\n\t\tlv2:prototype <http://lv2plug.in/plugins/eg-params>")[0])
check("set gain 6", c.set("ingen:/main/m3/gain", "ingen:value", "\"6\"^^xsd:float")[0])
check("cables", (c.connect("ingen:/main/in_l", "ingen:/main/m3/in")[0], c.connect("ingen:/main/m3/out", "ingen:/main/out_l")[0]))
for key, value in (("pedalpi:patchName", "\"Saved one\""), ("pedalpi:patchId", "\"p-0001\""), ("pedalpi:schemaVersion", "\"1\"^^xsd:int")):
    check("graph property %s" % key, c.request("a patch:Set ;\n\t%spatch:subject <ingen:/main> ;\n\tpatch:property %s ;\n\tpatch:value %s" % (ctx, key, value))[0])
check("assignment property on a port", c.set("ingen:/main/m3/gain", "pedalpi:assignedControl", "\"knob1\"")[0])
c.drain()
transcript.clear()

bundle = os.path.join(root, "one.ingen")
start = time.time()
check("save to %s" % bundle, (c.copy("ingen:/main", "file://" + bundle)[0], "%.2f s" % (time.time() - start)))
check("files", sorted(os.listdir(bundle)) if os.path.isdir(bundle) else "none")
for name in sorted(os.listdir(bundle)) if os.path.isdir(bundle) else []:
    path = os.path.join(bundle, name)
    if os.path.isfile(path):
        print("---- %s" % name)
        print(open(path).read())
    else:
        print("---- %s/ %s" % (name, os.listdir(path)))
print("==== what the saving client was told")
c.drain(); show(transcript, 1500)

spaced = os.path.join(root, "with space", "two.ingen")
check("save to a percent-encoded path", c.copy("ingen:/main", "file://" + spaced.replace(" ", "%20"))[0])
check("  written where", [p for p in (spaced, spaced.replace(" ", "%20")) if os.path.isdir(p)])
check("save to a path without .ingen", c.copy("ingen:/main", "file://" + os.path.join(root, "plain.ttl"))[0])
check("save into a missing directory", c.copy("ingen:/main", "file://" + os.path.join(root, "absent", "x.ingen"))[0])
check("save a block, not a graph", c.copy("ingen:/main/m3", "file://" + os.path.join(root, "block.ingen"))[0])

for name in sorted(os.listdir(os.path.join(bundle, "m4"))):
    print("---- m4/%s" % name)
    print(open(os.path.join(bundle, "m4", name)).read())

def blocks(text):
    return sorted(set(w for w in text.replace("<", " ").replace(">", " ").split() if w.startswith("ingen:/main/m") and w.count("/") == 2))

print("==== clear the graph, then load the bundle into the root")
for path in ("ingen:/main/m3", "ingen:/main/m4"):
    c.delete(path)
c.drain(); transcript.clear()
status = c.copy("file://" + bundle, "ingen:/main", timeout=5)[0]
check("load into ingen:/main", status)
c.drain(0.5)
order = []
for direction, text in transcript:
    if direction != "<":
        continue
    if "patch:Response" in text:
        order.append("response")
    if "ingen:/main/m3>" in text and "patch:Put" in text:
        order.append("content")
check("what arrives first", " then ".join(dict.fromkeys(order)))
status, text, _ = c.get("ingen:/main")
check("blocks afterwards", blocks(text))
check("gain restored", '"6.0"^^xsd:float' in text or '"6"^^xsd:float' in text)
check("name, position, assignment restored", ("Boost" in text, "canvasX" in text, "knob1" in text))
check("graph properties restored", ("Saved one" in text, "p-0001" in text))
check("plugin state reloaded (engine log)", "see the engine log below")

print("==== load again on top of the same blocks")
check("load into a graph that already has m3", c.copy("file://" + bundle, "ingen:/main", timeout=5)[0])
c.drain()
check("blocks afterwards", blocks(c.get("ingen:/main")[1]))

print("==== the bundle moved to another folder, plugin state with it")
moved = os.path.join(root, "moved", "elsewhere.ingen")
shutil.copytree(bundle, moved)
for path in ("ingen:/main/m3", "ingen:/main/m4"):
    c.delete(path)
c.drain()
check("load the moved bundle", c.copy("file://" + moved, "ingen:/main", timeout=5)[0])
c.drain()
check("blocks afterwards", blocks(c.get("ingen:/main")[1]))
c.copy("ingen:/main", "file://" + os.path.join(root, "moved", "again.ingen"))
again = os.path.join(root, "moved", "again.ingen", "m4")
check("saved again: its state and the file it names", sorted(os.listdir(again)) if os.path.isdir(again) else "none")

print("==== a bundle whose plugin is missing")
missing = os.path.join(root, "missing.ingen")
shutil.copytree(bundle, missing)
main = os.path.join(missing, "main.ttl")
open(main, "w").write(open(main).read().replace("http://lv2plug.in/plugins/eg-amp", "http://example.org/not-installed"))
for path in ("ingen:/main/m3", "ingen:/main/m4"):
    c.delete(path)
c.drain()
check("load bundle with an absent plugin", c.copy("file://" + missing, "ingen:/main", timeout=5)[0])
c.drain()
check("blocks afterwards", blocks(c.get("ingen:/main")[1]))
for path in ("ingen:/main/m3", "ingen:/main/m4"):
    c.delete(path)
c.drain()
check("load a bundle that does not exist", c.copy("file://" + os.path.join(root, "nothing.ingen"), "ingen:/main", timeout=5)[0])
garbage = os.path.join(root, "garbage.ingen"); os.makedirs(garbage)
open(os.path.join(garbage, "manifest.ttl"), "w").write("this is not turtle")
check("load an unreadable bundle", c.copy("file://" + garbage, "ingen:/main", timeout=5)[0])
c.drain()
check("engine still answers; blocks", (c.get("ingen:/main")[0], blocks(c.get("ingen:/main")[1])))

print("==== a client that connects and never reads")
idle = Client()
c.copy("file://" + bundle, "ingen:/main", timeout=5)
start = time.time()
status = c.get("ingen:/main", timeout=5)[0]
check("next request answered while that client is idle", "%s after %.1f s" % (status, time.time() - start))
