# Clouds

**Recorded:** 14 October 2026, on the Windows PC with the engine under WSL2. **Scope:** P6.1 (build record), P6.2 (binary and host check), P6.3 (parameter overlay), and the part of P6.4 that test signals can do. **Verified with:** the patched Ingen of [the engine evidence](14-ingen-engine-evidence.md), JACK's dummy backend at 48 kHz and 128 frames, and generated signals. Recorded output: [evidence/clouds-results.txt](evidence/clouds-results.txt). **Not covered:** the Pi, a guitar or any listening, processor load, the milestone patch (P6.5) and combined load (P6.6).

Poly's `polyclouds` library is Mutable Instruments' Clouds with the Parasites firmware, built as seven LV2 plugins, one per mode. All seven load, process sound and restore their controls in Ingen. The overlay now presents each mode with its own control names, switches for freeze and reverse, whole-semitone pitch, and the controls that matter on the first page.

## Build record (P6.1)

| Part | Revision |
| --- | --- |
| `lenticular_lv2` (Poly) | `141fdc89e250987bae421664132ddaedae735107`, as pinned in [the manifest](../reference/manifest.json) |
| `parasites/stmlib` (submodule; Clouds' DSP is in-tree under `parasites/clouds`) | `8ab2aaee77cbacb47b646d46d22ee5d358effe2d` |
| `eurorack` and `eurorack/stmlib` (not used by Clouds) | `99432f2b…`, `d9b18967…` |

`tools/ingen-spike/02-plugins.sh` clones these fresh, so nothing under `reference/` changes. The build differs from upstream in two ways, both given on the command line:

- **The SSE flags.** The makefile's flags target x86 SSE. For the Pi 5 they are replaced by `-mcpu=cortex-a76` with the same optimizations, and `tools/pi/build-arm64.sh` builds it that way.
- **A stray command in the polyrings.ttl rule.** It is bypassed by copying that one file in first.

| Binary | Machine | Needs |
| --- | --- | --- |
| x86-64 (development engine) | X86-64 | `libstdc++`, `libm`, `libc` |
| arm64 (Debian 12 root, emulated) | AArch64 | the same three, all resolved in that root |

Neither binary has any other undefined symbol. The arm64 binary has not run on a Pi.

## Host check (P6.2)

All seven modes declare 4 audio ports, 12 CV inputs and 13 controls. Each requires `urid:map`, which Ingen provides, and offers `hardRTCapable`. None has LV2 state, a worker or a file parameter.

| Finding | Evidence | Consequence |
| --- | --- | --- |
| Silent until switched on | Its `enabled` control at 0 writes silence; 0.000000 RMS measured | The adapter sets it when adding the module and keeps it set ([catalogue export](16-catalogue-export.md)). Bypass is the engine's own block bypass, which passes the input through unchanged (measured equal to a direct cable). A patch saved when bypass cleared the control still loads as bypassed, and enabling it turns the control back on. |
| **In gain does nothing** | `in_gain_param` is read, but its use is commented out upstream; 0.0624 RMS out at 0 and at 1 | Hidden by the overlay |
| Freeze and reverse are switches | The plugin acts only at 1.0 (or a CV above 0.4); 0.6 did not hold | Shown as switches |
| Blend 0 was dry, but 9 dB down, and not dry with reverb | 100% of the output was the input tone, at 0.0624 RMS for 0.1768 in (−9.1 dB): Clouds' crossfade gives the dry path 0.707 and its output conversion halves everything (the module's analogue stage makes that up). Its reverb comes after the blend, so at reverb 1 the dry path was pulled down further (−11/−17 dB on the Pi) and reverb added. | **Fixed by `tools/plugin-patches/clouds-blend-and-output.patch` (16 October 2026).** The wrapper runs Clouds fully wet and mixes the untouched input itself, like a pedal's mix knob: dry at full level up to blend 0.5, falling to nothing at 1; Clouds' sound rising to full by 0.5, doubled back to the input's level. Resonestor keeps the original behaviour (its blend is distortion). The same patch adds an **Output** control to every mode, -24 to +12 dB, for the level of the whole mix: dry plus Clouds at blend 0.5 is above unity ([Output results](evidence/pi-effect-results.md#clouds-output)). Measured on the Pi: blend 0 equals the input (within 7e-13 at reverb 0; 1e-4 at reverb 1, the end of the control's smoothing from the previous setting); 0.5 is full dry plus Clouds at −1 to −3 dB; 1 leaves Clouds alone, about 0 dB. [Results](evidence/pi-effect-results.md#clouds-blend). |
| Input clamps at full scale | The plugin converts to 16 bits; a 1.5 input came out no louder than 0.99 did | None needed |
| Pitch is exact | Wet only, a 440 Hz input came out at 439.7, 884.1, 216.5 and 662.1 Hz for 0, +12, −12 and +7 semitones | Whole semitones in the overlay, as on Beebo |
| Freeze holds the buffer | Input stopped: 0.0669 RMS a second later with freeze on, silence with it off | — |
| Values survive a bundle | 11 changed controls and the on switch saved and loaded back exactly, and the reloaded patch is heard | — |
| The buffer does not survive | Clouds has no LV2 state, so a frozen sound is not saved with the patch | Saved patches restore settings, not captured audio |
| Runs at the host rate | The DSP is the firmware's, which ran at 32 kHz with a fixed buffer | Grain and buffer times are shorter than on the module (inference from the source, not measured) |

**Which controls change the sound in each mode.** Each control was moved from one end to the other, with noise and with gated tones as input. It counts when the change exceeds twice the spread between three takes at the same settings. This is a coarse screen, not a listening test. Granular output is random, so its threshold is high.

| Mode | Clearly changes the sound | No clear change in this screen |
| --- | --- | --- |
| granular | position, pitch, density, texture | size, blend, spread, feedback, reverb |
| stretch | pitch, density, blend, feedback, reverb | position, size, texture, spread |
| looping_delay | position, size, pitch, blend, feedback, reverb | density, texture, spread |
| spectral | pitch, density, texture, blend, feedback, reverb | position, size, spread |
| oliverb | pitch, density, texture, blend | position, size, spread, feedback, reverb |
| resonestor | everything except blend | blend |
| beat_repeat | reverb only | everything else |

**Beat repeat needs adaptation.** The firmware derives its beat-repeat settings from the knobs: probability, clock divider, pitch mode, slice selection. The LV2 port never sets them, so they stay at zero and the mode responds only to reverb. Beebo does not offer it either. It stays loadable but is recorded as *needs adaptation*. Spectral is not offered by Beebo either; it works, and its names follow the firmware source.

Spread acts between the two channels, which the screen measures only weakly.

## Parameter overlay (P6.3)

The overlay (`config/catalogue/overlay.json`) can now present individual controls. The rules are in [catalogue.hpp](../src/catalogue/catalogue.hpp) and tested in `peepo_catalogue_tests`. A per-module rule may list controls, each by symbol, and give it:

- a name;
- a unit;
- a kind: continuous, whole numbers, or an on/off switch;
- a default inside the plugin's range;
- or hide it.

Listed controls come first, in the order listed, and the rest keep theirs. The overlay cannot change a range: the engine neither reports nor checks ranges, so it stays the plugin's.

A symbol the module lacks, a switch over a range other than 0–1, a default outside the range, or an attempt to invent choices refuses the whole overlay. A chosen default is set by the adapter when the module is added, because the plugin starts at its own.

For Clouds the overlay:

- hides In gain;
- makes freeze and reverse switches that start off;
- makes pitch whole semitones from 0 (upstream's default is 0.1, a slight detune);
- names each mode's controls as the Parasites firmware does, following Beebo's labels where Beebo offers the mode;
- orders the controls so the first page of four holds the ones that shape the sound and the second holds pitch, blend and freeze;
- puts Peepo's Output (added by the plugin patch, -24 to +12 dB) right after Blend, so the mix and its level sit together.

| Mode | Shown as | Page 1 | Page 2 |
| --- | --- | --- | --- |
| granular | Clouds | Position, Size, Density, Texture | Pitch, Blend, Output, Freeze |
| stretch | Clouds Stretch | Position, Size, Pitch, Blend | Output, Diffusion, Filter, Freeze |
| looping_delay | Clouds Looper | Tape length, Pitch window, Pitch, Blend | Output, Diffusion, Filter, Loop |
| spectral | Clouds Spectral | Position, Warp, Refresh, Quantize | Pitch, Blend, Output, Freeze |
| oliverb | Oliverb | Pre-delay, Size, Decay, Damp | Pitch, Blend, Output, Hold |
| resonestor | Resonestor | Timbre, Chord, Decay, Filter | Pitch, Harmonics, Scatter, Switch voice |
| beat_repeat | Clouds Beat Repeat | as exported | |

The remaining controls, Reverse and Output among them where not listed above, follow on page 3. Resonestor's Output comes after Random mod there.

The remaining controls follow on a third page. The generic page is the Clouds editor for now. The interface's custom Clouds page still serves only the simulator's demo module; the plan makes a custom editor optional.

## Run it

Inside Debian (WSL), as root, after `02-plugins.sh`:

```bash
bash tools/ingen-spike/11-clouds.sh
```

It prints the build record and both binaries (with `PEEPO_ARM64_ROOT` naming the arm64 root, default `/opt/peepo-arm64`), then runs `11-clouds.py` against a fresh engine. That takes about 15 minutes, most of it the per-mode screen.

## Still open

**First musical milestone, 14 October 2026:** the updated package runs on the Pi and the user confirmed guitar listening through Clouds ("it's working!"). The user's patch has also been saved and reloaded on the Pi, and the user heard no interruption while only the interface restarted during playing; the monitored audio had no gaps or xruns. See [Pi bring-up](23-pi-bring-up.md). The original Freeze-on settings remain as revision 1; revision 2 has Freeze off for live input. The captured buffer is not saved. A complete listening sweep of the modes and controls, and the formal Clouds-plus-conventional-effect milestone patch, are still separate work.

- **P6.1, P6.2 on the Pi:** now checked in the [Pi update](23-pi-bring-up.md#todays-package-brought-to-the-pi). The six offered modes load and their controls and bypass pass native checks; granular Clouds also processes stereo test audio. The guitar listening milestone remains pending.
- **P6.4.** Needs live guitar input, listening, and the Pi's levels. The checks here establish only that the DSP responds as the source says.
- **P6.5 and P6.6.**
- **Beat repeat.** Adapting it means setting the firmware's beat-repeat parameters from controls. That is a change to Poly's port, not to the overlay.
