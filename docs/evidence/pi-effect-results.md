# Conventional effect on the Pi (P3.4, P6.5), 16 October 2026

Pi 5, Debian 12 arm64, Tascam US-100 at 48 kHz, 128 frames x 3 periods. MDA Overdrive (`http://drobilla.net/plugins/mda/Overdrive`, Debian mda-lv2 1.2.10) in the installed Peepo engine.

## Gain: tools/pi/check-effect.sh

IN -> Overdrive -> OUT; a 1 kHz tone from a JACK client replaced the audio interface and the output was read back instead of reaching the speakers. Gain is output RMS over input RMS; THD is harmonics 2-5 on the left channel.

| Case | Drive | Muffle | Output | Input peak dBFS | Gain dB (L, R) | Output peak dBFS | THD % |
| --- | ---: | ---: | ---: | ---: | --- | ---: | ---: |
| neutral | 0 | 0 | 0.5 | -30 | -0.00, -0.00 | -30.0 | 0.0 |
| neutral | 0 | 0 | 0.5 | -18 | -0.00, -0.00 | -18.0 | 0.0 |
| neutral | 0 | 0 | 0.5 | -6 | +0.00, +0.00 | -6.0 | 0.0 |
| output min | 0 | 0 | 0.0 | -30 | -20.00, -20.00 | -50.0 | 0.0 |
| output min | 0 | 0 | 0.0 | -18 | -20.00, -20.00 | -38.0 | 0.0 |
| output min | 0 | 0 | 0.0 | -6 | -20.00, -20.00 | -26.0 | 0.0 |
| output max | 0 | 0 | 1.0 | -30 | +20.00, +20.00 | -10.0 | 0.0 |
| output max | 0 | 0 | 1.0 | -18 | +20.00, +20.00 | 2.0 | 0.0 |
| output max | 0 | 0 | 1.0 | -6 | +20.00, +20.00 | 14.0 | 0.0 |
| muffle full | 0 | 1 | 0.5 | -30 | -14.38, -14.38 | -44.38 | 0.0 |
| muffle full | 0 | 1 | 0.5 | -18 | -14.38, -14.38 | -32.38 | 0.0 |
| muffle full | 0 | 1 | 0.5 | -6 | -14.38, -14.38 | -20.38 | 0.0 |
| light drive | 0.3 | 0.2 | 0.5 | -30 | +8.10, +8.10 | -22.55 | 10.37 |
| light drive | 0.3 | 0.2 | 0.5 | -18 | +4.16, +4.16 | -14.34 | 8.16 |
| light drive | 0.3 | 0.2 | 0.5 | -6 | +1.22, +1.22 | -5.12 | 5.73 |
| full drive | 1 | 0 | 0.5 | -30 | +16.04, +16.04 | -15.0 | 16.58 |
| full drive | 1 | 0 | 0.5 | -18 | +10.04, +10.04 | -9.0 | 16.58 |
| full drive | 1 | 0 | 0.5 | -6 | +4.04, +4.04 | -3.0 | 16.58 |

JACK latency: capture [128, 128] frames, playback [384, 384] frames; nominal round trip 10.67 ms, converter delay excluded.
Impulse through the engine: 128 frames, exactly one period, as a single test client looped through the engine must show.
Xruns during the measurement (graph build plus 18 control changes): 2. All output finite.

## Combined patch: peepo_effect_live combine 0.3 0.2 0.45

IN -> Overdrive (drive 0.3, muffle 0.2, output 0.45) -> Clouds (controls unchanged from Clouds guitar r2) -> OUT, saved as `Overdrive into Clouds` (p20261016-133756-59b6, r000001). The bundle on the card matched the running graph; after a temporary drive change, reloading from the card restored name, identity, both modules, all six cables, every control and layout. The startup patch was left as Clouds guitar.

## Steady playback of the combined patch (monitor-ui-restart.py, passive, nothing changed)

```
{
 "duration_seconds": 22.000064991000045,
 "blocks": 8251,
 "callback_frame_gaps": 0,
 "xruns": 0,
 "all_finite": true,
 "exactly_silent_output_blocks": [
  0,
  0
 ],
 "maximum_input_rms": [
  0.004629574814935261,
  0.005126682413424636
 ],
 "maximum_output_rms": [
  0.00514283303830793,
  0.005566083852734087
 ]
}
```

Afterwards `vcgencmd get_throttled` = 0x50000 (undervoltage and throttling had occurred earlier this boot, not at that moment); 50.5 C.

## Clouds blend

`sudo check-effect.sh clouds`: IN -> Clouds (granular) -> OUT, a -18 dBFS 1 kHz tone, Clouds an octave up with random grains (density 0.75), no freeze or feedback. The dry gain is the part of the output that is exactly the input (aligned by the measured 128-frame loop delay); Clouds' sound is the rest, relative to the input.

Before, with the upstream wrapper (that run used density 0.5, Clouds' silent middle, so only the dry path and reverb are meaningful): blend 0 gave the dry signal at -9.04 dB with reverb 0, and -10.82 / -17.17 dB (L/R) plus reverb at -15 dB with reverb 1; blend 0.5 at reverb 1 gave dry -12.3 / -21.7 dB.

After the blend part of `tools/plugin-patches/clouds-blend-and-output.patch`, before Output was added (0 xruns during the run):

| Reverb | Blend | Dry gain dB (L, R) | Clouds dB (L, R) | Max abs(out - in) | Output peak dBFS |
| ---: | ---: | --- | --- | ---: | ---: |
| 0 | 0 | 0.0, 0.0 | -251.81, -251.81 | 6.90e-13 | -18.0 |
| 0 | 0.25 | -0.01, -0.01 | -11.03, -11.03 | 1.17e-01 | -12.37 |
| 0 | 0.5 | -0.03, -0.03 | -2.72, -2.72 | 2.66e-01 | -8.58 |
| 0 | 0.75 | -6.07, -6.07 | -2.4, -2.4 | 3.13e-01 | -10.16 |
| 0 | 1 | -52.06, -52.06 | -4.39, -4.39 | 3.40e-01 | -12.7 |
| 1 | 0 | -0.0, -0.0 | -75.49, -78.94 | 1.12e-04 | -17.99 |
| 1 | 0.25 | -0.02, 0.0 | -7.13, -7.47 | 1.74e-01 | -10.68 |
| 1 | 0.5 | -0.01, 0.0 | -0.8, -1.14 | 3.18e-01 | -7.8 |
| 1 | 0.75 | -6.07, -6.04 | -1.21, -1.9 | 3.07e-01 | -10.25 |
| 1 | 1 | -51.53, -73.07 | -0.08, 0.81 | 4.19e-01 | -10.28 |

At blend 1 the remaining 'dry' figure (-52 dB and below) is Clouds' own sound happening to correlate with the tone, not the input path, which is multiplied by exactly 0.

## Clouds Output

The user found the sum at blend 0.5 above unity and asked for an output control. `clouds-blend-and-output.patch` adds `output_param` (Output, -24 to +12 dB, default 0 dB, smoothed like the other controls) to all seven Clouds modes; it scales the whole mix. Patches saved before it load with 0 dB. Same measurement as above, after installing it on the Pi and re-exporting the Pi's catalogue (0 xruns):

| Reverb | Blend | Output dB | Dry gain dB (L, R) | Clouds dB (L, R) | Output peak dBFS |
| ---: | ---: | ---: | --- | --- | ---: |
| 0 | 0 | +0 | 0.0, 0.0 | -258.67, -258.67 | -18.0 |
| 0 | 0.25 | +0 | -0.01, -0.01 | -9.88, -9.88 | -12.06 |
| 0 | 0.5 | +0 | -0.02, -0.02 | -3.38, -3.38 | -8.34 |
| 0 | 0.75 | +0 | -6.08, -6.08 | -2.03, -2.03 | -10.28 |
| 0 | 1 | +0 | -51.4, -51.4 | -2.86, -2.86 | -10.79 |
| 1 | 0 | +0 | -0.0, -0.0 | -86.13, -83.12 | -18.0 |
| 1 | 0.25 | +0 | -0.01, -0.0 | -7.74, -7.84 | -11.39 |
| 1 | 0.5 | +0 | -0.01, -0.02 | -1.61, -2.22 | -7.84 |
| 1 | 0.75 | +0 | -6.08, -6.04 | -1.46, -0.27 | -8.7 |
| 1 | 1 | +0 | -49.24, -70.18 | -0.48, -1.14 | -11.53 |
| 0 | 0.5 | -6 | -6.02, -6.02 | -10.13, -10.13 | -13.98 |
| 0 | 0.5 | +6 | 5.97, 5.97 | 2.88, 2.88 | -2.8 |

Clouds' own level varies by about 1 dB between runs (random grains), so its change with Output is approximate; the dry path follows Output to within 0.03 dB.
