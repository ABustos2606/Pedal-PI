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

1. **Habit-style plugin first.** It needs a long history buffer with a Scan control, a Spread playback position, and a Collect/feedback input. It is simple to code, and nothing in the catalogue resembles it. Give it a send and a return so existing modules (TAP Pitch, Degrade, filters) can be patched into its feedback loop in Ingen. That is the modular version of Habit's modifiers. Ingen does not allow a loop back into the same plugin, so the send and return must be two separate plugins (see [Discipline](#discipline-the-habit-style-plugin)).
2. **MOOD-style plugin second.** An always-listening micro-looper plus a small reverb/delay, sharing one internal Clock. The shared Clock is the point of the pedal.
3. **Skip Tensor and Microcosm clones for now.** Make Clouds-based presets instead.

Describe the plugins as "inspired by" these pedals, and don't copy the real pedals' names, presets or look.

## Discipline: the Habit-style plugin

**Update, 15 October 2026.** The Habit-style plugin will be called Discipline. It will be the first LV2 plugin written for Peepo.

### Where it lives

Discipline gets its own git repository, outside Peepo:

- Every other plugin Peepo runs is built from another repository at a pinned commit. Discipline should be built the same way.
- It is a normal LV2 bundle, so it can be tested in Carla or Ardour on a desktop, without Ingen or the Pi.
- It uses its own build (Makefile, and DPF if used), apart from Peepo's CMake and LVGL build.
- Its license can be chosen separately from Peepo's GPLv3.
- Sound tuning will take months. Tagged releases let Peepo update to a new version on purpose.

What stays in Peepo:

- its entry in `config/catalogue/overlay.json`;
- the build step that fetches and builds it at a pinned commit;
- patches that use it, and xrun measurements;
- this document and any test notes.

Use a plugin URI under a domain or repository you control, such as `https://github.com/ABustos2606/discipline#`. Don't put it under Peepo's namespace. Fix the URI before the first saved patch, because changing it later breaks patches. Don't use "Habit" or "Chase Bliss" in names or descriptions.

### Send and return: two plugins, not one

Ingen refuses connections that create a loop. When a connection is added, Ingen works out a new order to run the plugins in. If the connection closes a loop, that step fails and the connection is rejected with `COMPILATION_FAILED` and a "Feedback compiling ..." log line (`reference/upstream/ingen/src/server/events/Connect.cpp:137`, `CompiledGraph.cpp`). So a cable from a plugin's send, through other modules, back to the same plugin's return can never be patched.

Split Discipline into two plugins in the same bundle, sharing one buffer:

```
Discipline ── send ──▶ [any modules] ──▶ Discipline Return
```

- **Discipline** records the input into the long buffer, reads the playback positions, and outputs the echo on its send.
- **Discipline Return** writes the processed audio back into the shared buffer. Discipline reads it on the next audio block. That adds one block of delay (about 2.7 ms at 128 samples), which doesn't matter because the shortest echo is 50 ms.
- The two instances find each other through a shared "pair" ID control. Beebo's `loop_common_in/out` modules appear to be the same kind of pair.
- The user always wires send to return, either directly or through effects. A plugin can't tell whether a port is patched (Ingen connects unpatched ports to silence), so the plugin shouldn't try to guess.

### Modulation through CV

Habit's built-in ramp engine exists because the hardware has no other way to modulate its knobs. Peepo already has CV modules, so Discipline should expose a CV input for each main control (Repeats, Size, Spread, Scan, Modify) instead of copying that engine. Each control's value is its knob setting plus its CV input, limited to the control's range. Each control can then follow a different LFO, envelope or random source, which the hardware cannot do.

The six built-in modifiers can also wait. Existing modules patched between Discipline and Discipline Return do that job.

### Gemini's blueprint and skeleton (15 October 2026)

Gemini produced a feature list for Habit and a C++ skeleton. The milestone order is good: memory buffer and delay first, then Scan, then the OUT / IN / FEED routing. The rest needs care:

- **Unverified hardware details.** Check the full DIP-switch list, "Memory Dry", the 50 ms to 60 s Size range, Size changing without a pitch change, and what Collect does against the official Chase Bliss Habit manual before laying out the controls. Gemini's description of Collect (overdubbing the long buffer) differs from the one in the table above.
- **Wrong catalogue claims.** Marbles (`polymarbles`) and Warps are in `config/catalogue/inventory.txt`. Frames and Beads are not.
- **Bugs in the skeleton.** Don't build on it:
    - Wipe clears the whole 34 MB buffer with `std::fill` in a single `run()` call, a guaranteed xrun. Clear a slice per block, or fade the buffer out.
    - The "fall back to the internal path when the return is silent" test runs per sample. It flips at every zero crossing and in quiet passages, which distorts the sound. The two-plugin pair replaces it.
    - IN and FEED run identical code. All three modes write feedback into the long buffer, but in OUT and IN the long buffer should hold only the input. That needs a separate echo loop, with FEED alone writing into the long buffer.
    - Size glides through a smoothing filter, which bends the pitch. A pitch-free change needs a crossfade between two playback positions.
    - Size plus Scan can reach about 239 s in a 180 s buffer. The read position wraps around and plays recent audio, with a click.
    - `tanh` on every write colours even the dry signal. Level is not smoothed, so it can zipper. The URI `http://peepo.dsp/...` is not a domain anyone owns.

## Next steps

- [ ] Get the Habit manual and confirm the controls and behaviours listed above.
- [ ] Create the Discipline repository and choose its license and URI.
- [ ] Write milestone 1: the Discipline and Discipline Return pair, the shared buffer, a separate echo loop, Size changes by crossfade, a Wipe that can't cause xruns, and the `.ttl` files with CV inputs.
- [ ] Lay out the control ports to fit the catalogue overlay format.
- [ ] Build it in the WSL environment, then on the Pi, through a pinned build step in Peepo.
- [ ] Measure xruns on the Pi with it in a typical patch.
