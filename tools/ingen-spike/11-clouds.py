# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
"""Clouds in the engine with test signals (P6.2 host side, P6.4 in part).

Builds stereo IN -> Clouds -> OUT in a running engine, feeds generated
signals through JACK and measures what comes back: level and clipping,
dry/wet, pitch, freeze holding the sound after the input stops, the plugin's
own on/off switch, which controls change the sound in each of the seven
modes, and whether saved values come back from a bundle. Test signals stand
in for a guitar; nothing here says how Clouds sounds, or anything about the
Pi. Needs a running engine (see env.sh):
    python3 11-clouds.py [bundle-directory]
"""
import math
import os
import re
import shutil
import sys
import threading
import time

import jack
import numpy

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ingenprobe import Client

CLOUDS = "http://polyeffects.com/lv2/polyclouds#"
MODES = ["granular", "stretch", "looping_delay", "spectral", "oliverb", "resonestor", "beat_repeat"]
CONTINUOUS = ["position_param", "size_param", "pitch_param", "density_param", "texture_param",
              "blend_param", "spread_param", "feedback_param", "reverb_param"]
BLOCK = "ingen:/main/clouds"
failures = 0


def report(ok, text):
    global failures
    failures += 0 if ok else 1
    print("%s  %s" % ("PASS" if ok else "FAIL", text))


def note(text):
    print("NOTE  %s" % text)


class Rig:
    """A JACK client that plays a chosen signal into both inputs and keeps
    the last seconds of both outputs."""

    def __init__(self):
        self.client = jack.Client("clouds-probe")
        self.outs = [self.client.outports.register(n) for n in ("src_l", "src_r")]
        self.ins = [self.client.inports.register(n) for n in ("cap_l", "cap_r")]
        self.rate = self.client.samplerate
        self.kind, self.freq, self.level, self.phase, self.seed = "sine", 440.0, 0.25, 0.0, numpy.random.default_rng(7)
        self.lock = threading.Lock()
        self.keep = int(self.rate * 4)
        self.cap = numpy.zeros((2, self.keep), dtype=numpy.float32)
        self.written = 0
        self.client.set_process_callback(self.process)
        self.client.activate()

    def process(self, frames):
        if self.kind == "sine":
            step = 2 * math.pi * self.freq / self.rate
            data = self.level * numpy.sin(self.phase + step * numpy.arange(frames))
            self.phase = (self.phase + step * frames) % (2 * math.pi)
        elif self.kind == "noise":
            data = self.level * self.seed.uniform(-1, 1, frames)
        elif self.kind == "bursts":
            # Two tones gated 120 ms in every 500 ms: something for grains,
            # echoes and tails to act on that a steady tone does not show.
            t = (self.phase + numpy.arange(frames)) / self.rate
            gate = (t % 0.5) < 0.12
            data = self.level * gate * (numpy.sin(2 * math.pi * 330 * t) + 0.5 * numpy.sin(2 * math.pi * 523 * t))
            self.phase += frames
        else:
            data = numpy.zeros(frames)
        for port in self.outs:
            port.get_array()[:] = data
        with self.lock:
            for i, port in enumerate(self.ins):
                self.cap[i] = numpy.roll(self.cap[i], -frames)
                self.cap[i, -frames:] = port.get_array()
            self.written += frames

    def play(self, kind, level=0.25, freq=440.0):
        self.kind, self.level, self.freq = kind, level, freq

    def record(self, seconds, settle=0.5):
        time.sleep(settle)
        with self.lock:
            start = self.written
        time.sleep(seconds)
        with self.lock:
            count = min(self.written - start, self.keep)
            return self.cap[:, -count:].copy()


def rms(x):
    return float(numpy.sqrt(numpy.mean(numpy.square(x, dtype=numpy.float64))))


def peak_frequency(x, rate):
    window = x * numpy.hanning(len(x))
    spectrum = numpy.abs(numpy.fft.rfft(window))
    spectrum[: int(40 * len(x) / rate)] = 0
    return float(numpy.argmax(spectrum) * rate / len(x))


def features(take):
    """What a listener would notice, independent of where in the cycle a take
    starts: spectrum of each side, how much the level moves, how alike the
    two sides are, and overall level."""
    out = []
    for x in take:
        s = numpy.abs(numpy.fft.rfft(x * numpy.hanning(len(x))))
        out.extend(numpy.log10(numpy.array([b.mean() for b in numpy.array_split(s[: len(s) // 4], 32)]) + 1e-7))
    frames = numpy.sqrt(numpy.mean(numpy.square(take.sum(axis=0)[: len(take[0]) // 960 * 960].reshape(-1, 960)), axis=1))
    out.append(4 * float(numpy.std(frames) / (numpy.mean(frames) + 1e-6)))
    out.append(4 * float(numpy.corrcoef(take[0], take[1])[0, 1]) if rms(take[0]) > 1e-5 and rms(take[1]) > 1e-5 else 0.0)
    out.append(4 * math.log10(rms(take) + 1e-6))
    return numpy.array(out)


def distance(a, b):
    return float(numpy.mean(numpy.abs(features(a) - features(b))))


def value(c, path):
    reply = c.get(path)[1]
    m = re.search(r'ingen:value "?(-?[0-9.eE+-]+)', reply)
    return float(m.group(1)) if m else None


def set_control(c, symbol, v, block=BLOCK):
    c.set("%s/%s" % (block, symbol), "ingen:value", "\"%s\"^^xsd:float" % v)


def add_clouds(c, mode, block=BLOCK, controls=None):
    status = c.put(block, "a ingen:Block ;\n\t\tlv2:prototype <%s%s>" % (CLOUDS, mode))[0]
    if status != 0:
        return status
    set_control(c, "enabled", 1, block)
    for symbol, v in (controls or {}).items():
        set_control(c, symbol, v, block)
    for side in ("l", "r"):
        c.connect("ingen:/main/in_%s" % side, "%s/%s_in" % (block, side))
        c.connect("%s/%s_out" % (block, side), "ingen:/main/out_%s" % side)
    return 0


def main():
    sys.stdout.reconfigure(line_buffering=True)
    rig = Rig()
    c = Client()
    for side in ("l", "r"):
        c.put("ingen:/main/in_" + side, "a lv2:InputPort , lv2:AudioPort")
        c.put("ingen:/main/out_" + side, "a lv2:OutputPort , lv2:AudioPort")
    time.sleep(0.3)
    for side in ("l", "r"):
        rig.client.connect("clouds-probe:src_" + side, "ingen:in_" + side)
        rig.client.connect("ingen:out_" + side, "clouds-probe:cap_" + side)
    rate = rig.rate
    print("==== engine at %d Hz, %d frames" % (rate, rig.client.blocksize))

    # --- the measurement itself
    c.connect("ingen:/main/in_l", "ingen:/main/out_l")
    rig.play("sine")
    direct = rms(rig.record(0.5)[0])
    c.disconnect("ingen:/main/in_l", "ingen:/main/out_l")
    report(abs(direct - 0.25 / math.sqrt(2)) < 0.003, "the measurement itself: direct cable %.4f RMS" % direct)

    # --- granular: switch, level, dry/wet, pitch, freeze
    print("==== granular")
    flat = {"feedback_param": 0, "reverb_param": 0, "pitch_param": 0}
    report(add_clouds(c, "granular", controls=flat) == 0, "granular loads with the plugin's defaults")
    out = rig.record(1.0)
    report(numpy.isfinite(out).all() and rms(out[0]) > 0.005, "switched on, it is heard -- %.4f RMS left, %.4f right" % (rms(out[0]), rms(out[1])))
    set_control(c, "enabled", 0)
    off = rig.record(0.5)
    report(rms(off) < 1e-6, "its own switch at 0 silences it -- %.6f RMS (it does not pass the dry signal)" % rms(off))
    set_control(c, "enabled", 1)
    # So Peepo's bypass is the engine's, which passes each input through.
    c.set(BLOCK, "ingen:enabled", "false")
    through = rig.record(0.5)
    report(abs(rms(through[0]) - direct) < 0.003, "the engine's bypass passes the input -- %.4f RMS for %.4f direct" % (rms(through[0]), direct))
    c.set(BLOCK, "ingen:enabled", "true")

    set_control(c, "blend_param", 0)
    out = rig.record(1.0, settle=1.0)
    dry = rms(out[0])
    # Dry means the input's own waveform: one clean tone, nothing else.
    s = numpy.abs(numpy.fft.rfft(out[0] * numpy.hanning(len(out[0]))))
    tone = int(round(440 * len(out[0]) / rate))
    purity = float(numpy.sum(numpy.square(s[tone - 3: tone + 4])) / numpy.sum(numpy.square(s)))
    report(purity > 0.99 and abs(peak_frequency(out[0], rate) - 440) < 3,
           "blend 0 is the dry signal -- %.0f Hz, %.1f%% of its energy in that tone" % (peak_frequency(out[0], rate), 100 * purity))
    note("but %.1f dB quieter than the input (%.4f RMS out for %.4f in)" % (20 * math.log10(dry / direct), dry, direct))
    set_control(c, "blend_param", 1)
    wet = rig.record(1.0, settle=1.0)
    report(rms(wet[0]) > 0.005, "blend 1 is wet only and still heard -- %.4f RMS" % rms(wet[0]))

    for semitones, expected in ((0, 440.0), (12, 880.0), (-12, 220.0), (7, 440.0 * 2 ** (7 / 12))):
        set_control(c, "pitch_param", semitones)
        out = rig.record(1.5, settle=1.5)
        found = peak_frequency(out[0], rate)
        report(abs(found - expected) / expected < 0.02, "pitch %+d semitones, wet only: strongest %.1f Hz, expected %.1f" % (semitones, found, expected))
    set_control(c, "pitch_param", 0)

    # Freeze: hold what is in the buffer after the input stops.
    rig.play("sine")
    time.sleep(1.0)
    set_control(c, "freeze_param", 1)
    time.sleep(0.3)
    rig.play("silence")
    held = rig.record(1.5, settle=1.0)
    report(rms(held[0]) > 0.005, "freeze on, input stopped: still sounding after 1 s -- %.4f RMS" % rms(held[0]))
    set_control(c, "freeze_param", 0)
    released = rig.record(0.5, settle=2.5)
    report(rms(released[0]) < 0.002, "freeze off, no input: decays to %.5f RMS" % rms(released[0]))
    rig.play("sine")
    time.sleep(1.0)
    rig.play("silence")
    free = rig.record(1.5, settle=1.0)
    report(rms(free[0]) < rms(held[0]) / 4, "without freeze the same pause is quieter -- %.5f RMS" % rms(free[0]))
    set_control(c, "freeze_param", 0.6)
    rig.play("sine")
    time.sleep(1.0)
    rig.play("silence")
    half = rig.record(1.5, settle=1.0)
    report(rms(half[0]) < rms(held[0]) / 4, "freeze at 0.6 does not hold (the plugin needs 1) -- %.5f RMS" % rms(half[0]))
    set_control(c, "freeze_param", 0)

    # in_gain is read by the plugin but its use is commented out upstream.
    rig.play("sine")
    set_control(c, "blend_param", 0)
    levels = []
    for g in (0, 1):
        set_control(c, "in_gain_param", g)
        levels.append(rms(rig.record(1.0, settle=1.0)[0]))
    report(abs(levels[0] - levels[1]) < 0.01 * direct, "In gain does nothing -- %.4f at 0, %.4f at 1; the overlay hides it" % tuple(levels))

    # Loud input: the plugin converts to 16 bits and clamps.
    rig.play("sine", level=0.99)
    loud = rig.record(1.0, settle=1.0)
    report(numpy.isfinite(loud).all() and numpy.max(numpy.abs(loud)) <= 1.0001,
           "a full-scale input stays within full scale -- peak %.3f" % numpy.max(numpy.abs(loud)))
    rig.play("sine", level=1.5)
    over = rig.record(1.0, settle=1.0)
    note("an input above full scale (1.5) comes out at peak %.3f: Clouds clamps its input to 16 bits" % numpy.max(numpy.abs(over)))
    rig.play("sine")
    c.delete(BLOCK)
    time.sleep(0.3)

    # --- every mode: heard, and which controls move the sound
    print("==== the seven modes: a control counts when moving it from one end to the other changes the")
    print("     sound by more than twice the spread of three takes at the same settings, with steady")
    print("     noise or with gated tones in. A coarse screen, not a listening test.")
    for mode in MODES:
        if add_clouds(c, mode, controls={"freeze_param": 0}) != 0:
            report(False, "%s did not load" % mode)
            continue
        ratios, heard, level = {}, True, 0.0
        for signal in ("noise", "bursts"):
            rig.play(signal, level=0.2)
            time.sleep(1.5)
            base = [rig.record(2.0, settle=0.1) for _ in range(3)]
            heard = heard and numpy.isfinite(base[0]).all() and rms(base[0]) > 0.002
            level = max(level, rms(base[0]))
            spread = max(distance(base[i], base[j]) for i, j in ((0, 1), (0, 2), (1, 2)))
            for symbol in CONTINUOUS:
                original = value(c, "%s/%s" % (BLOCK, symbol))
                takes = []
                for v in ((-24, 24) if symbol == "pitch_param" else (0, 1)):
                    set_control(c, symbol, v)
                    takes.append(rig.record(2.0, settle=1.5))
                set_control(c, symbol, original)
                d = distance(takes[0], takes[1])
                ratio = (d - 0.02) / max(spread, 1e-3)
                ratios[symbol] = max(ratios.get(symbol, -1), ratio)
        moved = ["%s %.1fx" % (s.replace("_param", ""), r) for s, r in ratios.items() if r > 2]
        still = ["%s %.1fx" % (s.replace("_param", ""), r) for s, r in ratios.items() if r <= 2]
        report(heard, "%-13s heard -- %.4f RMS\n      changes the sound: %s\n      no clear change:   %s"
               % (mode, level, ", ".join(moved) or "none", ", ".join(still) or "none"))
        c.delete(BLOCK)
        time.sleep(0.3)
    rig.play("sine")

    # --- saved values come back
    print("==== state restoration through a bundle")
    root = sys.argv[1] if len(sys.argv) > 1 else "/tmp/peepo-clouds"
    shutil.rmtree(root, ignore_errors=True)
    os.makedirs(root)
    bundle = os.path.join(root, "clouds.ingen")
    chosen = {"position_param": 0.8, "size_param": 0.3, "pitch_param": 7, "density_param": 0.66, "texture_param": 0.2,
              "blend_param": 0.9, "spread_param": 0.1, "feedback_param": 0.4, "reverb_param": 0.35, "freeze_param": 1, "reverse_param": 1}
    rig.play("sine")
    add_clouds(c, "granular", controls=chosen)
    time.sleep(1.0)
    report(c.copy("ingen:/main", "file://" + bundle)[0] == 0, "saved to %s" % bundle)
    c.delete(BLOCK)
    time.sleep(0.3)
    report(c.copy("file://" + bundle, "ingen:/main", timeout=5)[0] == 0, "loaded back")
    time.sleep(0.5)
    back = {s: value(c, "%s/%s" % (BLOCK, s)) for s in list(chosen) + ["enabled"]}
    wrong = {s: v for s, v in back.items() if v is None or abs(v - chosen.get(s, 1)) > 1e-6}
    report(not wrong, "every control value and the on switch came back%s" % ("" if not wrong else " -- not: %s" % wrong))
    out = rig.record(1.0, settle=1.0)
    report(rms(out[0]) > 0.002, "the reloaded patch is heard -- %.4f RMS" % rms(out[0]))
    text = open(os.path.join(bundle, "main.ttl")).read()
    note("the bundle holds control values only: Clouds has no LV2 state, so a frozen buffer is not saved (%s)"
         % ("no state in main.ttl" if "state:state" not in text else "main.ttl has a state:state block"))

    c.close()
    rig.client.deactivate()
    rig.client.close()
    print("==== %s" % ("all passed" if not failures else "%d failed" % failures))
    sys.exit(1 if failures else 0)


main()
