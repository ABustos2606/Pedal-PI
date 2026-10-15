# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Pedal PI contributors
"""Controlled JACK tone for pedal_tuner_live; no hardware input is used."""
import math
import pathlib
import subprocess
import sys
import time
import jack
import numpy

control = pathlib.Path(sys.argv[2])
control.write_text("0")
frequency = 0.0
phase = 0.0
client = jack.Client("pedalpi-tuner-tone")
port = client.outports.register("out")

def process(frames):
    global phase
    increment = 2 * math.pi * frequency / client.samplerate
    angle = phase + increment * numpy.arange(frames)
    # A guitar-like harmonic signal also exercises automatic fundamental
    # detection; the tuner's FFT deliberately rejects isolated weak peaks.
    port.get_array()[:] = 0 if frequency == 0 else 0.12 * (numpy.sin(angle) + 0.4 * numpy.sin(2 * angle) + 0.2 * numpy.sin(3 * angle))
    phase = (phase + increment * frames) % (2 * math.pi)

client.set_process_callback(process)
client.activate()
test = subprocess.Popen([sys.argv[1], "unix:///tmp/pedalpi-tuner.sock", sys.argv[3], str(control)])
connected = False
try:
    while test.poll() is None:
        if not connected and client.get_ports("ingen:in_l"):
            client.connect(port, "ingen:in_l")
            connected = True
            print("Tone connected to ingen:in_l", flush=True)
        try:
            next_frequency = float(control.read_text())
            if next_frequency != frequency:
                print("Tone frequency:", next_frequency, flush=True)
            frequency = next_frequency
        except ValueError:
            pass
        time.sleep(0.02)
finally:
    client.deactivate()
    client.close()
sys.exit(test.returncode)
