#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Passive JACK monitor: records aggregate levels, never saves or injects audio.
import json, math, sys, time
from pathlib import Path
import jack
import numpy as np

client=jack.Client('pedalpi_restart_monitor')
rate, period = client.samplerate, client.blocksize
ports=[client.inports.register(name) for name in ['input_l','input_r','output_l','output_r']]
records=[]; xruns=[]
def process(frames):
    frame=int(client.last_frame_time)
    data=[p.get_array() for p in ports]
    rms=[math.sqrt(float(np.mean(x*x))) for x in data]
    records.append((frame,frames,rms,all(bool(np.isfinite(x).all()) for x in data)))
def xrun(*args): xruns.append([time.monotonic(),str(args)])
client.set_process_callback(process)
client.set_xrun_callback(xrun)
client.activate()
for source, port in zip(['system:capture_1','system:capture_2','ingen:out_l','ingen:out_r'],ports):
    client.connect(source,port.name)
time.sleep(1)
records.clear(); xruns.clear()
Path('/tmp/pedalpi-restart-monitor-ready').write_text('ready')
start=time.monotonic()
time.sleep(22)
duration=time.monotonic()-start
client.deactivate(); client.close()
frames=sum(row[1] for row in records)
gaps=sum(records[i][0] != records[i-1][0]+records[i-1][1] for i in range(1,len(records)))
report={'duration_seconds':duration,'sample_rate':rate,'period_frames':period,'blocks':len(records),'frames':frames,
        'callback_frame_gaps':gaps,'xruns':len(xruns),'all_finite':all(row[3] for row in records),
        'exactly_silent_output_blocks':[sum(row[2][channel] == 0 for row in records) for channel in [2,3]],
        'minimum_output_rms':[min(row[2][channel] for row in records) for channel in [2,3]],
        'maximum_input_rms':[max(row[2][channel] for row in records) for channel in [0,1]],
        'maximum_output_rms':[max(row[2][channel] for row in records) for channel in [2,3]]}
Path('/tmp/pedalpi-ui-restart-audio.json').write_text(json.dumps(report,indent=2))
print(json.dumps(report,indent=2),flush=True)
ok = report['all_finite'] and not gaps and not xruns and not any(report['exactly_silent_output_blocks'])
print(('PASS' if ok else 'CHECK')+' audio monitor across interface restart',flush=True)
sys.exit(0 if ok else 1)
