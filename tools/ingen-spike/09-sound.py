# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pedal PI contributors
"""Do real plugins process sound in the engine, not merely load?

Feeds a 440 Hz sine at 0.25 through one module at a time and measures what
comes back. A number above the noise floor means the plugin's DSP ran inside
Ingen on this machine; it says nothing about how the module sounds, about the
Pi, or about processor load. Needs a running engine (see env.sh).
"""
import math
import os
import sys
import time

import jack
import numpy

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ingenprobe import Client

LEVEL = 0.25
# Plugin, its left input and output, and controls to set first.
CASES = [
    # These three stay silent until their own `enabled` control is set.
    ("http://polyeffects.com/lv2/polyclouds#granular", "l_in", "l_out", {"enabled": 1}),
    ("http://polyeffects.com/lv2/polyclouds#oliverb", "l_in", "l_out", {"enabled": 1}),
    ("http://polyeffects.com/lv2/polywarps#fold", "carrier", "out", {"enabled": 1}),
    ("http://drobilla.net/plugins/mda/Overdrive", "left_in", "left_out", {}),
    ("http://drobilla.net/plugins/mda/Delay", "left_in", "left_out", {}),
]


class Meter:
    def __init__(self):
        self.client = jack.Client("probe")
        self.out = self.client.outports.register("sine")
        self.cap = self.client.inports.register("cap")
        self.phase = 0.0
        self.squares = []
        self.client.set_process_callback(self.process)
        self.client.activate()

    def process(self, frames):
        step = 2 * math.pi * 440.0 / self.client.samplerate
        self.out.get_array()[:] = LEVEL * numpy.sin(self.phase + step * numpy.arange(frames))
        self.phase = (self.phase + step * frames) % (2 * math.pi)
        data = self.cap.get_array()
        self.squares.append((float(numpy.mean(data * data)), bool(numpy.isfinite(data).all())))

    def rms(self, seconds=1.0):
        time.sleep(0.5)
        mark = len(self.squares)
        time.sleep(seconds)
        tail = self.squares[mark:]
        return math.sqrt(sum(s for s, _ in tail) / len(tail)), all(ok for _, ok in tail)


def measure_left(name):
    """Measure a patch somebody else built, without touching the engine."""
    meter = Meter()
    meter.client.connect("probe:sine", "ingen:in_l")
    meter.client.connect("ingen:out_l", "probe:cap")
    level, finite = meter.rms()
    ok = finite and 0.001 < level < 4.0
    print("%s  %s, added and cabled by the adapter, is heard -- %.4f RMS out for %.4f in" % ("PASS" if ok else "FAIL", name, level, LEVEL / math.sqrt(2)))
    meter.client.deactivate()
    meter.client.close()
    sys.exit(0 if ok else 1)


def main():
    if len(sys.argv) > 2 and sys.argv[1] == "--left":
        measure_left(sys.argv[2])
    meter = Meter()
    c = Client()
    for port, kind in (("in_l", "Input"), ("out_l", "Output")):
        c.put("ingen:/main/" + port, "a lv2:%sPort , lv2:AudioPort" % kind)
    time.sleep(0.3)
    meter.client.connect("probe:sine", "ingen:in_l")
    meter.client.connect("ingen:out_l", "probe:cap")
    c.connect("ingen:/main/in_l", "ingen:/main/out_l")
    direct, _ = meter.rms(0.5)
    c.disconnect("ingen:/main/in_l", "ingen:/main/out_l")
    silent, _ = meter.rms(0.5)
    expected = LEVEL / math.sqrt(2)
    ok = abs(direct - expected) < 0.002 and silent < 1e-6
    print("%s  the measurement itself -- direct cable %.4f RMS (expected %.4f), no cable %.6f" % ("PASS" if ok else "FAIL", direct, expected, silent))
    failed = 0 if ok else 1
    for uri, port_in, port_out, controls in CASES:
        name = uri.rsplit("/", 1)[-1]
        status = c.put("ingen:/main/probe", "a ingen:Block ;\n\t\tlv2:prototype <%s>" % uri)[0]
        if status != 0:
            print("FAIL  %s -- engine did not load it (status %s)" % (name, status))
            failed += 1
            continue
        for symbol, value in controls.items():
            c.set("ingen:/main/probe/" + symbol, "ingen:value", "\"%s\"^^xsd:float" % value)
        c.connect("ingen:/main/in_l", "ingen:/main/probe/" + port_in)
        c.connect("ingen:/main/probe/" + port_out, "ingen:/main/out_l")
        level, finite = meter.rms()
        c.delete("ingen:/main/probe")
        time.sleep(0.2)
        ok = finite and 0.001 < level < 4.0
        failed += 0 if ok else 1
        print("%s  %s processes sound -- %.4f RMS out for %.4f in%s" % ("PASS" if ok else "FAIL", name, level, expected, "" if finite else " (non-finite samples)"))
    c.close()
    meter.client.deactivate()
    meter.client.close()
    sys.exit(1 if failed else 0)


main()
