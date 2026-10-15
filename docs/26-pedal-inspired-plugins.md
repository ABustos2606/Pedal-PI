# Pedal-inspired LV2 plugins

**Assessment, 14 October 2026.** Effects in the style of the Chase Bliss MOOD MKII and Habit, the Red Panda Tensor and the Hologram Microcosm can all be built as LV2 plugins for the Pi 5 running Ingen. An assessment from Gemini gets the big picture right but oversells the details.

Processing power is not the limit. The real limits are keeping Ingen free of audio dropouts (xruns) and the months of sound tuning that give these pedals their character.

The current catalogue already covers much of the Microcosm and part of the Tensor through Clouds and related modules. It has nothing like Habit's long history buffer or MOOD's shared Clock. Those two are the plugins worth writing.

## Where Gemini is right

- **Feasibility.** These effects mostly come down to a buffer, a few read positions moving through it, crossfades and filters. A Pi 5 (four Cortex-A76 cores at 2.4 GHz) has plenty of headroom for that.
- **Memory.** A 3-minute stereo buffer at 48 kHz in 32-bit float is about 69 MB, which is trivial on 2 GB. Allocate it when the plugin is created, never in the audio callback.
- **Methods.** The right building blocks are:
    - a ring buffer that is always recording;
    - read positions that move at fractional speeds, with interpolation;
    - Hann-windowed grains and crossfaded jumps;
    - a simulated slower clock (hold each sample, then low-pass filter).
- **Frameworks.** DPF and Faust suit plugins without a UI, which is how Ingen uses them. Faust ships a Dattorro reverb and fractional delay lines. JUCE works but is heavier than needed.
- **Modularity.** In Ingen these plugins can be patched together in ways the hardware can't, such as a pitch shifter inside a delay's feedback loop.

## Where to push back

1. **The CPU numbers are invented.** "3–5%" and "all four at once with 70% free" are guesses. What the Pi struggles with is missed audio deadlines, not average load. The bring-up log already shows `JackEngine::XRun: client = ingen was not finished`. Ingen runs the whole patch as one JACK client, so a dense patch is limited by whether the audio keeps running without dropouts.
2. **"Easy" is misleading.** The code for Habit is simple. The feel of these pedals comes from months of tuning crossfade shapes, randomness, control response and filtering. Gemini admits this only for the Microcosm, but it applies to all four.
3. **Signalsmith Stretch "in 25 lines".** The library is real and MIT-licensed. With its default settings it adds tens of milliseconds of delay or more, which feels sluggish under a guitar. It can be tuned for less delay at the cost of more artifacts.
4. **Doubtful claims:**
    - Luppp and SooperLooper are standalone JACK apps, not LV2 plugins.
    - Beebo's looper modules (`loop_common_in/out`) have no public source.
    - "ChowMatrix compiles cleanly to ARM64 LV2" is optimistic.
    - The hardware details (Spin FV-1 chips, the 2–64 kHz clock range, which collaborator designed which side of the MOOD) look muddled. Verify them before relying on them.
5. **Missed: buffers are not saved with patches.** The Clouds tests ([20-clouds.md](20-clouds.md)) note that Clouds has no LV2 state, so a frozen buffer is lost on reload. A frozen MOOD loop or Habit's 3-minute history would be lost the same way unless the plugin saves LV2 state. Saving 69 MB per patch would be a bad idea anyway.
6. **Missed: control smoothing.** Ingen delivers control values once per audio block. Scan and speed controls must glide between values, or they will produce zipper noise.

## What the catalogue already covers

The Microcosm is mostly covered already, and Habit not at all. This table was checked against `config/catalogue/overlay.json`, `config/catalogue/inventory.txt` and `docs/evidence/`.

| Pedal | Already covered by existing modules | Still missing |
| --- | --- | --- |
| Microcosm | Most of it: Clouds (granular), Clouds Looper, Clouds Stretch, Oliverb, Beat Repeat (few controls work), avw Granulator, MVerb, convolution reverbs, the filters. Envelope follower and clock divider modules provide motion and triggering. | Rhythmic slicing modes (Mosaic, Seq) and the polish of its presets. Least worth rebuilding. |
| Tensor | Part of it: Clouds Stretch for pitch and time, TAP Pitch Shifter, TAP Reflector for reverse, toggle and sample-and-hold modules for random changes. | Instant hold, stutter and reverse in one plugin with low delay. Separate modules patched together won't feel the same. |
| MOOD | The pieces. Clouds Looper's freeze loops whatever is already in its buffer, which is close to MOOD's always-listening looper. Also the envelope follower for Env mode, Oliverb/MVerb, and the avw Downsampler and mda Degrade for grit. | The Clock. One knob slows the delay, reverb and loop together. All Ingen plugins share one sample rate, so the Clock must live inside one plugin. |
| Habit | Nothing. mda Delay and DubDelay have short buffers. | The 3-minute buffer you can scan through, the second (Spread) playback position, and Collect, which feeds processed audio back into the buffer. |

## Recommendation

Build what doesn't exist rather than redoing what Clouds already does well.

1. **Habit-style plugin first.** It needs a long history buffer with a Scan control, a Spread playback position, and a Collect/feedback input. It is simple to code, and nothing in the catalogue resembles it. Give it a send/return pair so existing modules (TAP Pitch, Degrade, filters) can be patched into its feedback loop in Ingen. That is the modular version of Habit's modifiers.
2. **MOOD-style plugin second.** An always-listening micro-looper plus a small reverb/delay, sharing one internal Clock. The shared Clock is the point of the pedal.
3. **Skip Tensor and Microcosm clones for now.** Make Clouds-based presets instead.

Describe the plugins as "inspired by" these pedals, and don't copy the real pedals' names, presets or look.

## Next steps

- [ ] Sketch the Habit-style plugin in DPF, with control ports laid out for the catalogue overlay format.
- [ ] Build it in the WSL environment, then on the Pi.
- [ ] Measure xruns on the Pi with it in a typical patch.
