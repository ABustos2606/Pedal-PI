# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
"""Beebo's Delay and Peepo's Foot switch in the engine, measured.

Delay: an impulse through it comes back as echoes the number of beats of its
Time apart at its BPM, and moves when the BPM changes, as the patch tempo
changes it. Foot switch: its control output gives Off value, then On value
once Pressed is set, as the interface sets it from a stomp. Read through a
control output of the graph, which the engine gives JACK as a port.
Needs a running engine (see env.sh).
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
    def __init__(self):
        self.client = jack.Client("delay-probe")
        self.out = self.client.outports.register("send")
        self.cap = self.client.inports.register("audio")
        self.cv = self.client.inports.register("cv")
        self.lock = threading.Lock()
        self.impulse = False
        self.audio, self.control = [], []
        self.client.set_process_callback(self.process)
        self.client.activate()
        self.rate = self.client.samplerate

    def process(self, frames):
        send = self.out.get_array()
        send[:] = 0
        with self.lock:
            if self.impulse:
                send[0] = 0.5
                self.impulse = False
            self.audio.append(self.cap.get_array().copy())
            self.control.append(self.cv.get_array().copy())

    def record(self, seconds, impulse=False):
        with self.lock:
            self.audio, self.control = [], []
            self.impulse = impulse
        time.sleep(seconds)
        with self.lock:
            return numpy.concatenate(self.audio), numpy.concatenate(self.control)


def onsets(signal, count):
    """The first `count` places where the signal rises out of silence."""
    found, at = [], 0
    while len(found) < count:
        loud = numpy.flatnonzero(numpy.abs(signal[at:]) > 0.01)
        if not len(loud):
            break
        found.append(at + int(loud[0]))
        at = found[-1] + 2000
    return found


def value(c, path, number):
    c.set(path, "ingen:value", "\"%s\"^^xsd:float" % number)


def main():
    probe = Probe()
    c = Client()
    for port, kind in (("in_l", "lv2:InputPort , lv2:AudioPort"), ("out_l", "lv2:OutputPort , lv2:AudioPort"), ("cv_out", "lv2:OutputPort , lv2:CVPort")):
        c.put("ingen:/main/" + port, "a " + kind)
    time.sleep(0.3)
    probe.client.connect("delay-probe:send", "ingen:in_l")
    probe.client.connect("ingen:out_l", "delay-probe:audio")
    probe.client.connect("ingen:cv_out", "delay-probe:cv")
    failed = 0

    def report(ok, text):
        nonlocal failed
        failed += 0 if ok else 1
        print("%s  %s" % ("PASS" if ok else "FAIL", text))

    # The round trip through JACK and the engine, without a module.
    c.connect("ingen:/main/in_l", "ingen:/main/out_l")
    audio, _ = probe.record(0.5, impulse=True)
    c.disconnect("ingen:/main/in_l", "ingen:/main/out_l")
    base = onsets(audio, 1)
    report(len(base) == 1, "the measurement itself -- a direct cable returns the impulse after %s samples" % (base[0] if base else "no"))
    base = base[0] if base else 0

    status = c.put("ingen:/main/delay", "a ingen:Block ;\n\t\tlv2:prototype <http://polyeffects.com/lv2/digit_delay>")[0]
    report(status == 0, "the engine loads Beebo's Delay (status %s)" % status)
    for symbol, number in (("Delay_1", 0.5), ("BPM_0", 120), ("Feedback_4", 0.5), ("Amp_5", 1), ("DelayT60_3", 0), ("EnableEcho_7", 1)):
        value(c, "ingen:/main/delay/" + symbol, number)
    c.connect("ingen:/main/in_l", "ingen:/main/delay/in0")
    c.connect("ingen:/main/delay/out0", "ingen:/main/out_l")
    time.sleep(0.5)
    for bpm, label in ((120, "1/8 at 120 BPM"), (60, "1/8 at 60 BPM, as a tempo change sets it")):
        value(c, "ingen:/main/delay/BPM_0", bpm)
        time.sleep(1.0)  # the old echoes die away
        audio, _ = probe.record(1.4, impulse=True)
        beat = 0.5 * 60 / bpm * probe.rate
        heard = [n - base for n in onsets(audio, 3)]
        expected = [round(beat * k) for k in (1, 2)]
        dry = len(heard) > 0 and heard[0] < 100
        ok = not dry and len(heard) >= 2 and all(abs(h - e) <= 2 for h, e in zip(heard, expected))
        report(ok, "%s: echoes after %s samples (expected %s, no dry signal)" % (label, heard[:2], expected))
    c.delete("ingen:/main/delay")

    status = c.put("ingen:/main/foot", "a ingen:Block ;\n\t\tlv2:prototype <urn:peepo:lv2:footswitch>")[0]
    report(status == 0, "the engine loads the Foot switch (status %s)" % status)
    value(c, "ingen:/main/foot/on_value", 0.8)
    value(c, "ingen:/main/foot/off_value", 0.2)
    c.connect("ingen:/main/foot/out", "ingen:/main/cv_out")
    time.sleep(0.3)
    _, released = probe.record(0.3)
    value(c, "ingen:/main/foot/pressed", 1)
    time.sleep(0.2)
    _, pressed = probe.record(0.3)
    report(abs(float(numpy.median(released)) - 0.2) < 1e-4 and abs(float(numpy.median(pressed)) - 0.8) < 1e-4,
           "the Foot switch gives %.2f released and %.2f pressed (Off value 0.2, On value 0.8)" % (numpy.median(released), numpy.median(pressed)))
    c.delete("ingen:/main/foot")
    # The engine reports a control output's value to its clients; remove it
    # before leaving, so nothing is written to a closed connection.
    c.delete("ingen:/main/cv_out")
    time.sleep(0.2)
    c.close()
    probe.client.deactivate()
    probe.client.close()
    sys.exit(1 if failed else 0)


main()
