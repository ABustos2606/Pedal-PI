# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
"""A NAM candidate in the engine (P8.2): models, worker, state, errors.

Loads the candidate whose bundle is on the engine's LV2_PATH, gives it models
the way the interface would (a patch:Set sent through Ingen to the plugin's
control port), and checks what comes back: whether each message form loads,
whether a model changes the sound, what the plugin reports, missing and
corrupt files, repeated switching (engine memory and xruns), quality changes,
and saving and restoring the model through a bundle. Test signals stand in for
a guitar; PC timing says nothing about the Pi, so no load is measured.
    python3 13-nam.py <label> <a1 model> <a2 model> <work directory> [--quick]
Needs a running engine (see env.sh). With --quick only loading and sound are
checked, for runs at other block sizes.
"""
import math
import os
import re
import shutil
import subprocess
import sys
import threading
import time

import jack
import numpy

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ingenprobe import Client, engine_alive, engine_pid

URI = "http://github.com/mikeoliphant/neural-amp-modeler-lv2"
MODEL = URI + "#model"
BLOCK = "ingen:/main/nam"
failures = 0


def report(ok, text):
    global failures
    failures += 0 if ok else 1
    print("%s  %s" % ("PASS" if ok else "FAIL", text), flush=True)


def note(text):
    print("NOTE  %s" % text, flush=True)


class Rig:
    def __init__(self):
        self.client = jack.Client("nam-probe")
        self.out = self.client.outports.register("src")
        self.inp = self.client.inports.register("cap")
        self.rate = self.client.samplerate
        self.level, self.phase, self.xruns = 0.3, 0.0, 0
        self.lock = threading.Lock()
        self.keep = int(self.rate * 3)
        self.cap = numpy.zeros(self.keep, dtype=numpy.float32)
        self.written = 0
        self.client.set_process_callback(self.process)
        self.client.set_xrun_callback(self.xrun)
        self.client.activate()

    def xrun(self, delay):
        self.xruns += 1

    def process(self, frames):
        step = 2 * math.pi * 220.0 / self.rate
        self.out.get_array()[:] = self.level * numpy.sin(self.phase + step * numpy.arange(frames))
        self.phase = (self.phase + step * frames) % (2 * math.pi)
        with self.lock:
            self.cap = numpy.roll(self.cap, -frames)
            self.cap[-frames:] = self.inp.get_array()
            self.written += frames

    def record(self, seconds=1.0, settle=0.6):
        time.sleep(settle)
        with self.lock:
            start = self.written
        time.sleep(seconds)
        with self.lock:
            return self.cap[-min(self.written - start, self.keep):].copy()


def rms(x):
    return float(numpy.sqrt(numpy.mean(numpy.square(x, dtype=numpy.float64))))


def describe(x, rate):
    """Level, and how much of the output is not the 220 Hz input tone."""
    s = numpy.abs(numpy.fft.rfft(x * numpy.hanning(len(x)))) ** 2
    tone = int(round(220 * len(x) / rate))
    total = float(s.sum()) + 1e-20
    return rms(x), 1 - float(s[tone - 3: tone + 4].sum()) / total


def differs(a, b, rate):
    (la, ha), (lb, hb) = describe(a, rate), describe(b, rate)
    return abs(20 * math.log10((la + 1e-9) / (lb + 1e-9))) > 0.5 or abs(ha - hb) > 0.01


def engine_rss():
    pid = engine_pid()
    try:
        with open("/proc/%s/status" % pid) as f:
            return int(re.search(r"VmRSS:\s+(\d+)", f.read()).group(1)) // 1024
    except (OSError, AttributeError):
        return -1


def send_model(c, value):
    body = "[\n\t\ta patch:Set ;\n\t\tpatch:property <%s> ;\n\t\tpatch:value %s\n\t]" % (MODEL, value)
    return c.set(BLOCK + "/control", "ingen:activity", body)[0]


def as_uri(path):
    return "<file://%s>" % path.replace("%", "%25").replace(" ", "%20")


def as_path(path):
    return "\"%s\"^^atom:Path" % path


LAST_REPLY = []


def reported_path(c):
    """Ask the plugin which model it has; it answers on its notify port.

    Ingen forwards that only to a subscribed client. The subscription is held
    just for the question: a subscribed client receives a constant stream,
    and one that stops reading stalls the engine (17-patch-storage.md)."""
    c.set("ingen:/clients/this", "ingen:broadcast", "true")
    c.drain(0.1)
    _, reply, others = c.set(BLOCK + "/control", "ingen:activity", "[ a patch:Get ]")
    seen = others + [reply] + c.drain(0.4)
    c.set("ingen:/clients/this", "ingen:broadcast", "false")
    c.drain(0.2)
    for m in seen:
        if "notify" not in m:
            continue
        LAST_REPLY[:] = [m[m.index("notify") - 40: m.index("notify") + 600]]
        found = re.search(r'patch:value\s+(?:<file://([^>]*)>|"([^"]*)"\^\^(?:atom:Path|<http://lv2plug.in/ns/ext/atom#Path>))', m[m.index("notify"):])
        if found:
            return (found.group(1) or found.group(2) or "").replace("%20", " ")
    return None


def restore(label, bundle):
    """Load a saved bundle into a freshly started engine: the model must come
    back through the plugin's state restore, with no message from us."""
    print("==== %s, restoring %s into a fresh engine" % (label, bundle), flush=True)
    if not os.path.isdir(bundle):
        report(False, "no saved bundle to restore")
        sys.exit(1)
    rig = Rig()
    c = Client()
    report(c.copy("file://" + bundle, "ingen:/main", timeout=10)[0] == 0, "the bundle loads")
    time.sleep(2.0)
    rig.client.connect("nam-probe:src", "ingen:in")
    rig.client.connect("ingen:out", "nam-probe:cap")
    c.set(BLOCK + "/notify", "ingen:broadcast", "true")
    out = rig.record()
    level, harmonics = describe(out, rig.rate)
    note("the restored patch: %.4f RMS, %.1f%% of the output is not the input tone; plugin reports %s"
         % (level, 100 * harmonics, repr(reported_path(c))))
    return finish(c, rig)


def main():
    if sys.argv[1] == "--restore":
        return restore(sys.argv[2], sys.argv[3])
    label, a1_source, a2_source, work = sys.argv[1:5]
    quick = "--quick" in sys.argv
    shutil.rmtree(work, ignore_errors=True)
    os.makedirs(os.path.join(work, "models"))
    a1 = os.path.join(work, "models", "a1 standard.nam")
    a2 = os.path.join(work, "models", "a2.nam")
    shutil.copy(a1_source, a1)
    shutil.copy(a2_source, a2)
    corrupt = os.path.join(work, "models", "corrupt.nam")
    with open(corrupt, "w") as f:
        f.write('{"version": "0.5.4", "architecture": "WaveNet", "config": {"layers": [')

    rig = Rig()
    c = Client()
    c.put("ingen:/main/in", "a lv2:InputPort , lv2:AudioPort")
    c.put("ingen:/main/out", "a lv2:OutputPort , lv2:AudioPort")
    time.sleep(0.3)
    rig.client.connect("nam-probe:src", "ingen:in")
    rig.client.connect("ingen:out", "nam-probe:cap")
    rate = rig.rate
    print("==== %s, engine at %d Hz, %d frames" % (label, rate, rig.client.blocksize), flush=True)

    report(c.put(BLOCK, "a ingen:Block ;\n\t\tlv2:prototype <%s>" % URI)[0] == 0, "the plugin loads in Ingen (it requires urid:map and work:schedule)")
    c.connect("ingen:/main/in", BLOCK + "/input")
    c.connect(BLOCK + "/output", "ingen:/main/out")
    # The plugin reports its model on its notify port. Ingen passes that on
    # only to clients that subscribe, and only for a port asked to.
    c.set(BLOCK + "/notify", "ingen:broadcast", "true")
    empty = rig.record()
    level, harmonics = describe(empty, rate)
    note("without a model: %.4f RMS out for %.4f in, %.1f%% not the input tone" % (level, rig.level / math.sqrt(2), 100 * harmonics))

    # How the model reaches the plugin: Beebo's form, then a typed path.
    forms = {}
    for form, value in (("file URI, as Beebo sends it", as_uri(a1)), ("atom:Path literal", as_path(a1))):
        send_model(c, "<file:///nonexistent-reset.nam>")
        time.sleep(0.5)
        status = send_model(c, value)
        time.sleep(1.0)
        out = rig.record()
        path = reported_path(c)
        loaded = differs(out, empty, rate)
        forms[form] = loaded
        if path is None and LAST_REPLY:
            note("notify traffic seen: %r" % LAST_REPLY[0])
        report(status == 0, "%s: accepted by the engine; sound %s; plugin reports %s"
               % (form, "changed (model running)" if loaded else "unchanged", repr(path)))
    if not any(forms.values()):
        report(False, "no message form loaded a model; nothing further can be checked")
        return finish(c, rig)
    value_of = as_uri if forms["file URI, as Beebo sends it"] else as_path
    note("continuing with the %s form" % ("file URI" if value_of is as_uri else "atom:Path"))

    sounds = {}
    for name, path in (("A1 standard", a1), ("A2", a2)):
        send_model(c, value_of(path))
        time.sleep(1.5)
        out = rig.record()
        level, harmonics = describe(out, rate)
        # A model that failed to load leaves the previous one playing, so the
        # second must also sound unlike the first.
        ok = numpy.isfinite(out).all() and differs(out, empty, rate) and level > 1e-4 and all(differs(out, other, rate) for other in sounds.values())
        sounds[name] = out
        report(ok, "%s model runs -- %.4f RMS, %.1f%% of the output is not the input tone, peak %.3f" % (name, level, 100 * harmonics, float(numpy.max(numpy.abs(out)))))
    if quick:
        return finish(c, rig)

    # Errors: the engine must survive; what the plugin keeps is recorded.
    for name, path in (("a missing file", os.path.join(work, "models", "absent.nam")), ("a corrupt file", corrupt)):
        send_model(c, value_of(a1))
        time.sleep(1.5)
        before = rig.record()
        send_model(c, value_of(path))
        time.sleep(1.5)
        out = rig.record()
        alive = engine_alive()
        kept = not differs(out, before, rate)
        bypass = not differs(out, empty, rate)
        report(alive and numpy.isfinite(out).all(), "%s: the engine survives; the plugin %s" % (
            name, "keeps the previous model" if kept else "drops to no model (dry signal)" if bypass else "produces something else"))

    # Repeated switching while audio runs.
    send_model(c, value_of(a1))
    time.sleep(1.0)
    start_rss, start_xruns = engine_rss(), rig.xruns
    for rounds in (1, 2):
        for i in range(30):
            send_model(c, value_of(a2 if i % 2 else a1))
            time.sleep(0.15)
        time.sleep(1.5)
        note("after %d model switches: engine RSS %d MiB (was %d), %d xruns on the PC's dummy backend"
             % (30 * rounds, engine_rss(), start_rss, rig.xruns - start_xruns))
    out = rig.record()
    report(numpy.isfinite(out).all() and rms(out) > 1e-4, "still processing after 60 switches -- %.4f RMS" % rms(out))

    # Quality: only the NeuralAudio candidate has the control.
    reply = c.get(BLOCK + "/quality_scale")[1]
    if "quality_scale" in reply and "ingen:value" in reply:
        send_model(c, value_of(a2))
        time.sleep(1.5)
        takes, xruns = {}, rig.xruns
        for q in (1.0, 0.0, 0.5, 1.0):
            c.set(BLOCK + "/quality_scale", "ingen:value", "\"%s\"^^xsd:float" % q)
            takes.setdefault(q, []).append(rig.record(1.0, settle=0.8))
        changed = differs(takes[1.0][0], takes[0.0][0], rate)
        back = not differs(takes[1.0][0], takes[1.0][1], rate)
        report(back, "A2 quality 1 -> 0 -> 0.5 -> 1: the sound %s between 1 and 0 and returns at 1; %d xruns while switching"
               % ("changes" if changed else "does not change", rig.xruns - xruns))
        c.set(BLOCK + "/quality_scale", "ingen:value", "\"1.0\"^^xsd:float")
    else:
        note("no quality control on this candidate")

    # State: save with a model, restore into the running engine.
    send_model(c, value_of(a1))
    time.sleep(1.5)
    reference = rig.record()
    bundle = os.path.join(work, "nam.ingen")
    report(c.copy("ingen:/main", "file://" + bundle, timeout=10)[0] == 0, "saved to %s" % bundle)
    files = sorted(os.path.relpath(os.path.join(d, f), bundle) for d, _, fs in os.walk(bundle) for f in fs)
    note("bundle files: %s" % ", ".join(files))
    for name in files:
        text = open(os.path.join(bundle, name), errors="replace").read()
        for line in text.splitlines():
            if "#model" in line or ".nam" in line:
                note("%s: %s" % (name, line.strip()))
    c.delete(BLOCK)
    time.sleep(0.5)
    report(c.copy("file://" + bundle, "ingen:/main", timeout=10)[0] == 0, "loaded back into the running engine")
    time.sleep(2.0)
    out = rig.record()
    report(not differs(out, reference, rate) and differs(out, empty, rate),
           "the reloaded block runs the saved model -- %.4f RMS against %.4f before saving" % (rms(out), rms(reference)))

    # The model file moves with the bundle only if the host copied it in.
    moved = os.path.join(work, "moved")
    os.makedirs(moved)
    shutil.copytree(bundle, os.path.join(moved, "nam.ingen"))
    os.rename(a1, a1 + ".away")
    c.delete(BLOCK)
    time.sleep(0.5)
    c.copy("file://" + os.path.join(moved, "nam.ingen"), "ingen:/main", timeout=10)
    time.sleep(2.0)
    out = rig.record()
    note("bundle moved and the original model file renamed: the plugin %s"
         % ("still runs the model (the bundle carries it)" if not differs(out, reference, rate) else "has no model (the bundle only points at the file)"))
    os.rename(a1 + ".away", a1)
    return finish(c, rig)


def finish(c, rig):
    c.close()
    rig.client.deactivate()
    rig.client.close()
    print("==== %s" % ("all passed" if not failures else "%d failed" % failures), flush=True)
    sys.exit(1 if failures else 0)


main()
