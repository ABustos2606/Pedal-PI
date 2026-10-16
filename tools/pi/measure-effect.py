#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Peepo contributors
# Gain of the effect in the running test graph (peepo_effect_live test-graph),
# or with `clouds`, how Clouds' blend mixes the input with its own sound.
# Feeds a 1 kHz tone into the engine's IN instead of the audio interface and
# reads its OUT instead of the speakers, so nothing is heard; the previous
# connections are put back at the end. Run as the peepo user, with peepo-route
# stopped (check-effect.sh does both).
import json, math, subprocess, sys, threading, time
from pathlib import Path
import jack
import numpy as np

TOOL = '/opt/peepo/libexec/peepo/peepo_effect_live'
FREQ = 1000.0
CLOUDS = len(sys.argv) > 1 and sys.argv[1] == 'clouds'
client = jack.Client('peepo_effect_measure')
rate, period = client.samplerate, client.blocksize
tone = [client.outports.register(n) for n in ('tone_l', 'tone_r')]
back = [client.inports.register(n) for n in ('return_l', 'return_r')]
state = {'amp': 0.0, 'phase': 0, 'impulse': False, 'collect': None}
lock = threading.Lock()
xruns = []

def process(frames):
    t = (state['phase'] + np.arange(frames)) / rate
    signal = (state['amp'] * np.sin(2 * math.pi * FREQ * t)).astype(np.float32)
    if state['impulse']:
        signal[:] = 0; signal[0] = 0.5; state['impulse'] = False
    state['phase'] += frames
    for port in tone: port.get_array()[:] = signal
    if state['collect'] is not None:
        state['collect'].append((signal.copy(), back[0].get_array().copy(), back[1].get_array().copy()))
client.set_process_callback(process)
client.set_xrun_callback(lambda *a: xruns.append(time.monotonic()))

def capture(seconds):
    time.sleep(0.6)  # settle after a change; Clouds smooths its controls over about 0.1 s
    state['collect'] = []
    time.sleep(seconds)
    blocks, state['collect'] = state['collect'], None
    return [np.concatenate([b[i] for b in blocks]) for i in range(3)]

def db(x): return 20 * math.log10(x) if x > 0 else float('-inf')
def rms(x): return float(np.sqrt(np.mean(np.square(x, dtype=np.float64))))
def thd(x):
    # Harmonics 2-5 against the fundamental, from a windowed spectrum.
    x = x[-16384:] * np.hanning(min(len(x), 16384))
    spectrum = np.abs(np.fft.rfft(x))
    def peak(f): k = int(round(f * len(x) / rate)); return float(spectrum[max(k - 3, 0):k + 4].max())
    fundamental = peak(FREQ)
    return math.sqrt(sum(peak(FREQ * h) ** 2 for h in range(2, 6))) / fundamental if fundamental else float('nan')

def set_control(symbol, value):
    subprocess.run([TOOL, 'set', symbol, str(value)], check=True, stdout=subprocess.DEVNULL)

saved = {}
client.activate()
try:
    # Detach the interface from the engine, remembering exactly what was there.
    for port in ('ingen:in_l', 'ingen:in_r'):
        saved[port] = [p.name for p in client.get_all_connections(port)]
        for other in saved[port]: client.disconnect(other, port)
    for port in ('ingen:out_l', 'ingen:out_r'):
        saved[port] = [p.name for p in client.get_all_connections(port)]
        for other in saved[port]: client.disconnect(port, other)
    client.connect(tone[0], 'ingen:in_l'); client.connect(tone[1], 'ingen:in_r')
    client.connect('ingen:out_l', back[0]); client.connect('ingen:out_r', back[1])

    # As JACK reports them (Debian's jack module has no latency calls). The
    # converters' own delay is not included; that needs a loopback cable.
    def latency(port, kind):
        text = subprocess.run(['jack_lsp', '-l', port], capture_output=True, text=True, check=True).stdout
        line = next(l for l in text.splitlines() if f'port {kind} latency' in l)
        return [int(n) for n in line.split('[')[1].split(']')[0].split()]
    capture_latency = latency('system:capture_1', 'capture')
    playback_latency = latency('system:playback_1', 'playback')
    report = {'sample_rate': rate, 'period_frames': period,
              'jack_capture_latency_frames': list(capture_latency), 'jack_playback_latency_frames': list(playback_latency),
              'nominal_round_trip_ms': (capture_latency[1] + playback_latency[1]) / rate * 1000, 'settings': []}

    # Delay added inside the engine: an impulse, at neutral settings.
    neutral = (('blend_param', 0),) if CLOUDS else (('drive', 0), ('muffle', 0), ('output', 0.5))
    for symbol, value in neutral: set_control(symbol, value)
    state['amp'] = 0.0; time.sleep(0.3)
    state['collect'] = []; state['impulse'] = True; time.sleep(0.3)
    blocks, state['collect'] = state['collect'], None
    sent, got = np.concatenate([b[0] for b in blocks]), np.concatenate([b[1] for b in blocks])
    report['engine_delay_frames'] = int(np.argmax(np.abs(got) > 0.01) - np.argmax(sent > 0)) if np.abs(got).max() > 0.01 else None

    if CLOUDS:
        # An octave up, random grains (density above its silent middle), nothing
        # held or fed back: Clouds' own sound sits at 2 kHz, so the part of the
        # output that is exactly the input is the dry path.
        for symbol, value in (('freeze_param', 0), ('reverse_param', 0), ('pitch_param', 12), ('feedback_param', 0),
                              ('spread_param', 0), ('position_param', 0.5), ('size_param', 0.5), ('density_param', 0.75), ('texture_param', 0.5)):
            set_control(symbol, value)
        delay = report['engine_delay_frames'] or 0
        state['amp'] = 10 ** (-18 / 20)
        # Every blend at 0 dB output, with and without reverb; then Output
        # turned down and up at blend 0.5, where dry and Clouds add up.
        runs = [(reverb, blend, 0) for reverb in (0, 1) for blend in (0, 0.25, 0.5, 0.75, 1)] + [(0, 0.5, -6), (0, 0.5, 6)]
        for reverb, blend, output in runs:
            set_control('reverb_param', reverb)
            set_control('blend_param', blend)
            set_control('output_param', output)
            sent, left, right = capture(1.0)
            x = sent[:len(sent) - delay]
            row = {'reverb': reverb, 'blend': blend, 'output_db': output}
            for side, y in (('left', left[delay:]), ('right', right[delay:])):
                dry = float(np.dot(y, x) / np.dot(x, x))
                rest = y - dry * x
                row[side] = {'dry_gain_db': round(db(abs(dry)), 2), 'clouds_rms_db_re_input': round(db(rms(rest) / rms(x)), 2),
                             'max_abs_difference_from_input': float(np.abs(y - x).max())}
            row['output_peak_dbfs'] = round(db(float(max(np.abs(left).max(), np.abs(right).max()))), 2)
            row['finite'] = bool(np.isfinite(left).all() and np.isfinite(right).all())
            report['settings'].append(row)
            print(json.dumps(row), flush=True)
        state['amp'] = 0.0
    cases = [] if CLOUDS else [('neutral', 0, 0, 0.5), ('output min', 0, 0, 0.0), ('output max', 0, 0, 1.0),
             ('muffle full', 0, 1, 0.5), ('light drive', 0.3, 0.2, 0.5), ('full drive', 1, 0, 0.5)]
    for name, drive, muffle, output in cases:
        for symbol, value in (('drive', drive), ('muffle', muffle), ('output', output)): set_control(symbol, value)
        for level in (-30, -18, -6):
            state['amp'] = 10 ** (level / 20)
            sent, left, right = capture(0.5)
            row = {'case': name, 'drive': drive, 'muffle': muffle, 'output': output, 'input_dbfs_peak': level,
                   'gain_db': [round(db(rms(left) / rms(sent)), 2), round(db(rms(right) / rms(sent)), 2)],
                   'output_peak_dbfs': round(db(float(max(np.abs(left).max(), np.abs(right).max()))), 2),
                   'thd_percent': round(100 * thd(left), 2), 'finite': bool(np.isfinite(left).all() and np.isfinite(right).all())}
            report['settings'].append(row)
            print(json.dumps(row), flush=True)
        state['amp'] = 0.0
    report['xruns'] = len(xruns)
finally:
    state['amp'] = 0.0
    for p in tone + back:
        try: p.disconnect()
        except jack.JackError: pass
    for port, others in saved.items():
        for other in others:
            try: client.connect(other, port) if port.startswith('ingen:in') else client.connect(port, other)
            except jack.JackError: pass
    client.deactivate(); client.close()

Path('/tmp/peepo-clouds-blend.json' if CLOUDS else '/tmp/peepo-effect-gain.json').write_text(json.dumps(report, indent=2))
print(json.dumps({k: v for k, v in report.items() if k != 'settings'}, indent=2), flush=True)
ok = report['xruns'] == 0 and all(r['finite'] for r in report['settings'])
print(('PASS' if ok else 'CHECK') + ' effect gain measurement', flush=True)
sys.exit(0 if ok else 1)
