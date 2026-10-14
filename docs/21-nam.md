# NAM in Ingen

**Recorded:** 14 October 2026, on the Windows PC with the engine under WSL2. **Scope:** P8.1 (candidate versions) and P8.2 (host integration). **Verified with:** the patched Ingen of [the engine evidence](14-ingen-engine-evidence.md) on JACK's dummy backend at 48 kHz, with 64, 128 and 1024 frames. Two captures from the unchanged Casette reference were used: an A1 standard WaveNet and an A2 slimmable container, both 48 kHz high-gain amps. Recorded output: [evidence/nam-results.txt](evidence/nam-results.txt). **Not covered:** the Pi, any processor or latency measurement (a PC says nothing about the Pi's), cabinet IRs, the NAM page in the interface (P8.3), and P8.4–P8.7.

**Selected: the current NeuralAudio-based plugin.** Both candidates load and run models in Ingen, and both save and restore their model with the patch. Only the current one loads A2 models and has a quality control. It also takes the host's maximum block size, and it does no allocation in its audio path that the source shows. Its one behaviour to design around: a model file that fails to load *clears* the running model rather than keeping it.

## Candidates (P8.1)

| | Current | Poly fork |
| --- | --- | --- |
| Plugin | `mikeoliphant/neural-amp-modeler-lv2` `96924205f9cbfbbf0e63f7fe7f4b6d2f1b60c402` (after tag v0.2.3; metadata minor 2, micro 3) | `polyeffects/neural-amp-modeler-lv2` `a3a6520ea519513a6ca1e82d350d47e3c7891732` (17 May 2023), as in [the manifest](../reference/manifest.json) |
| Model runtime | NeuralAudio `0bff1150` (v0.1.3-31), with NAM Core `896c5f17`, RTNeural `5909c449`, xsimd `a00c81f7`, math_approx `f6d55e70` | NAM Core `ae9ea35a`, Eigen `7bf2968f` |
| LV2 headers | `e9d94328` | `dbdbe3e5` |
| Build | CMake Release, `-O3`. NeuralAudio's own optimized A1 WaveNet and A2 implementations, with NAM Core as the fallback for other architectures. `USE_NATIVE_ARCH` off, so x86-64 baseline. | CMake Release |
| On the Pi | `-mcpu=cortex-a76` added by `12-nam.sh`; not built yet | the same |
| Binary (x86-64) | 2.4 MB; libstdc++, libm, libgcc_s, libc | 0.3 MB; also libmvec |
| Required features | `urid:map`, `work:schedule`, both provided by Ingen | the same |
| Optional | `options` (maximum block length), `state:threadSafeRestore`, `hardRTCapable` | `hardRTCapable` |
| Controls | Input and output level (±20 dB), Quality (0–1) | Input and output level |
| Model types (`mod:fileTypes`) | nam, nammodel, json, aidax, aidadspmodel | nammodel |

**Both declare the same plugin URI.** Only one can be installed where the engine looks, so `12-nam.sh` puts each in its own folder (`/opt/pedalpi/nam/current`, `/opt/pedalpi/nam/poly`). The engine is started with one of them on `LV2_PATH`. The catalogue export uses the current one.

| Model | Architecture | sha256 (first 16) |
| --- | --- | --- |
| `EVH 5150 Stealth.nam` | WaveNet 0.5.4 (A1 standard, 16/8 channels), 48 kHz, amp | `7b781974d180d5bf` |
| `APP-6505+-Crunch-I.nam` | SlimmableContainer 0.7.0 (A2), 48 kHz, amp | `9c1976410fbda885` |

## Host integration (P8.2)

| Check | Current | Poly fork |
| --- | --- | --- |
| Loads in Ingen, passes audio unchanged without a model | yes | yes |
| A1 model runs | yes, at 64, 128 and 1024 frames | yes, at all three |
| A2 model runs | yes, at all three | **no**, at all three: "Unable to load model", and the A1 model keeps playing |
| Missing or corrupt file | engine unaffected; the plugin **drops to no model** and passes the dry signal | engine unaffected; the plugin keeps the previous model |
| 60 model switches (A1 ↔ A2) while playing | engine memory 118 → 120 MiB, still processing, no xruns | not comparable: every A2 load fails, so half the switches change nothing |
| Quality 1 → 0 → 0.5 → 1 on the A2 model | the sound changes and returns, no xruns | no control |
| Saved with the patch | yes, and the model file is copied into the bundle | the same |
| Restored into a running engine, and into a freshly started one | yes, the same sound | yes |
| Bundle moved and the original model removed | still runs the model from the bundle | the same |

**How a model reaches the plugin.** The interface sends a `patch:Set` for the plugin's `#model` property. It goes to the block's `control` port as `ingen:activity`, which is how Beebo does it. Ingen passes it into the plugin's atom input. Both forms of the value work: Beebo's `<file:///…>` URI (Ingen's reader turns a file URI into an `atom:Path`) and an `atom:Path` literal. Paths with spaces arrive intact.

**How the plugin reports its model.** It answers on its `notify` port. Ingen forwards that only to a client that has subscribed (`ingen:/clients/this ingen:broadcast true`), and only for a port asked to (`ingen:broadcast true` on `notify`). The adapter subscribes to neither today.

A subscribed client receives a constant stream of activity. One that stops reading stalls the engine ([patch storage](17-patch-storage.md), point 7). So the NAM page (P8.3) should subscribe only while it needs an answer, or read the model from the saved state instead. The probe does the former.

**Worker and real-time safety, from the source.**

- *Current.* The model is loaded on Ingen's worker thread. The worker's reply swaps the model pointer on the audio thread, and the old model is freed back on the worker. Ingen delivers worker replies in its post-process step.
- *Current, A2 quality.* An A2 file is loaded as a composite with every quality level built up front, so a quality change only switches index on the audio thread. NAM Core's own slimmable models would take a lock and reset on that thread; NeuralAudio does not route SlimmableContainer files that way.
- *Current, block size.* It applies the host's maximum block length (Ingen offers `options`).
- *Poly fork.* It resizes a buffer inside `run()` whenever the block size changes, which allocates on the audio thread. It does not read the host's maximum block length.

**State and assets (with P5.4).** Saving runs the plugin's state through Ingen's `mapPath`. The model file is **copied into the bundle** (`nam/<model>.nam`), and `state.ttl` refers to it relatively. A saved patch therefore carries its model and survives moving, as P5.4 requires. Each stored revision holds its own copy: about 280–410 KB per model for these captures. On restore the plugin schedules the load on the worker, so the model is back moments after the patch.

**Engine messages seen.** Every probe that disconnected left "write error (Broken pipe)" lines; that is the engine noticing, and harmless. The Poly fork tries to load a default `click.wav` named in its own metadata when instantiated, and fails. The engine segfaulted while being stopped after some runs. That was seen with and without NAM loaded, so it belongs to engine shutdown, not to NAM; audio and patches were not affected.

## What this means for the next steps

- **P8.3.** The page needs a file selector that sends the `patch:Set` above. It also needs a way to show the active model: a short subscription, or the saved state. And it must handle the current plugin clearing its model on a failed load — keep the previous path and offer it again rather than assume the old model still plays.
- **Catalogue.** NAM is exported and loads, but stays *needs adaptation* until P8.3. With no model it is a silent passthrough, and the generic page cannot choose one.
- **P8.4 onwards** need the Pi. Nothing here measures cost.

## Run it

**Pi update, 14 October:** both candidates now build for arm64; the selected current plugin is installed and on the Pi engine's plugin path. It passed a native load, port, control and bypass check without a model. This does not cover model loading, CPU cost or listening on the Pi. The overlay continues to withhold it until file selection exists. See [Pi update](23-pi-bring-up.md#todays-package-brought-to-the-pi).

Inside Debian (WSL), as root:

```bash
bash tools/ingen-spike/12-nam.sh    # build both candidates
bash tools/ingen-spike/13-nam.sh    # host checks, about 10 minutes
```

`13-nam.sh` takes two model paths as arguments; without them it uses the two Casette captures above.
