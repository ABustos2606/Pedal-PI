# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
"""Plaits (Beebo's Macro Osc) and Marbles (Beebo's Chaos Controller) in the
engine: do they make signal, not merely load?

Plaits: each of its models, with no trigger cabled, should sound at its
output. Marbles: with its internal clock at 120 (x1 range), T2 should tick 8
times in 4 s, T1 give random gates and X1 changing random voltages. Read through
outputs of the graph, which the engine gives JACK as ports. Needs a running
engine (see env.sh).
"""
import os
import sys
import threading
import time

import jack
import numpy

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ingenprobe import Client


class Probe:
    def __init__(self, names):
        self.client = jack.Client("mi-probe")
        self.ports = {n: self.client.inports.register(n) for n in names}
        self.lock = threading.Lock()
        self.data = {n: [] for n in names}
        self.client.set_process_callback(self.process)
        self.client.activate()
        self.rate = self.client.samplerate

    def process(self, frames):
        with self.lock:
            for n, p in self.ports.items():
                self.data[n].append(p.get_array().copy())

    def record(self, seconds):
        with self.lock:
            self.data = {n: [] for n in self.ports}
        time.sleep(seconds)
        with self.lock:
            return {n: numpy.concatenate(v) for n, v in self.data.items()}


def value(c, path, number):
    c.set(path, "ingen:value", "\"%s\"^^xsd:float" % number)


def main():
    probe = Probe(["audio", "gate", "voltage"])
    c = Client()
    for port, kind in (("out_l", "lv2:OutputPort , lv2:AudioPort"), ("cv_a", "lv2:OutputPort , lv2:CVPort"), ("cv_b", "lv2:OutputPort , lv2:CVPort")):
        c.put("ingen:/main/" + port, "a " + kind)
    time.sleep(0.3)
    probe.client.connect("ingen:out_l", "mi-probe:audio")
    probe.client.connect("ingen:cv_a", "mi-probe:gate")
    probe.client.connect("ingen:cv_b", "mi-probe:voltage")
    failed = 0

    def report(ok, text):
        nonlocal failed
        failed += 0 if ok else 1
        print("%s  %s" % ("PASS" if ok else "FAIL", text))

    status = c.put("ingen:/main/plaits", "a ingen:Block ;\n\t\tlv2:prototype <http://polyeffects.com/lv2/polyplaits>")[0]
    report(status == 0, "the engine loads Plaits (status %s)" % status)
    c.connect("ingen:/main/plaits/out", "ingen:/main/out_l")
    silent = []
    for model in range(17):
        value(c, "ingen:/main/plaits/model", model)
        time.sleep(0.3)
        audio = probe.record(0.5)["audio"]
        level = float(numpy.sqrt(numpy.mean(audio * audio)))
        if not numpy.isfinite(audio).all() or level < 0.001:
            silent.append("%d (%.4f%s)" % (model, level, "" if numpy.isfinite(audio).all() else ", non-finite"))
    report(not silent, "Plaits sounds in every one of its 17 models without a trigger%s" % ("; silent: " + ", ".join(silent) if silent else ""))
    c.delete("ingen:/main/plaits")

    status = c.put("ingen:/main/marbles", "a ingen:Block ;\n\t\tlv2:prototype <http://polyeffects.com/lv2/polymarbles>")[0]
    report(status == 0, "the engine loads Marbles (status %s)" % status)
    # Rate 120 in the x1 range (the default range, like Beebo's, is x0.25).
    value(c, "ingen:/main/marbles/t_rate_param", 120)
    value(c, "ingen:/main/marbles/t_range_param", 1)
    value(c, "ingen:/main/marbles/x_spread_param", 0.5)
    c.connect("ingen:/main/marbles/t2_output", "ingen:/main/cv_a")
    c.connect("ingen:/main/marbles/x1_output", "ingen:/main/cv_b")
    time.sleep(0.5)
    seconds = 4.0
    signals = probe.record(seconds)
    clock, voltage = signals["gate"], signals["voltage"]
    high = clock > 0.5
    rises = int(numpy.count_nonzero(high[1:] & ~high[:-1]))
    levels = numpy.unique(numpy.round(voltage, 3))
    report(numpy.isfinite(clock).all() and 7 <= rises <= 9, "Marbles keeps its own clock: T2 rises %d times in %.0f s at 120 (expected 8)" % (rises, seconds))
    c.disconnect("ingen:/main/marbles/t2_output", "ingen:/main/cv_a")
    c.connect("ingen:/main/marbles/t1_output", "ingen:/main/cv_a")
    time.sleep(0.3)
    gates = probe.record(seconds)["gate"]
    high = gates > 0.5
    report(numpy.isfinite(gates).all() and int(numpy.count_nonzero(high[1:] & ~high[:-1])) >= 2,
           "Marbles T1 gives random gates: %d in %.0f s" % (int(numpy.count_nonzero(high[1:] & ~high[:-1])), seconds))
    report(numpy.isfinite(voltage).all() and len(levels) >= 3, "Marbles X1 gives changing random voltages: %d distinct values from %.2f to %.2f" % (len(levels), voltage.min(), voltage.max()))
    c.delete("ingen:/main/marbles")
    for port in ("cv_a", "cv_b"):
        c.delete("ingen:/main/" + port)
    time.sleep(0.2)
    c.close()
    probe.client.deactivate()
    probe.client.close()
    sys.exit(1 if failed else 0)


main()
