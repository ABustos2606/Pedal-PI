# Casette reference and NAM evaluation

**Reviewed and downloaded:** 13 October 2026 (America/Buenos_Aires).  
**Repository:** [pawelKapl/Casette](https://github.com/pawelKapl/Casette).  
**Pinned snapshot:** `75ded6663e2d758bfe8a290952c672b2e8c01697`, committed 21 July 2026.  
**Local checkout:** `reference/upstream/Casette`.  
**Status:** unchanged reference source; no compilation, installation, or Pi benchmark performed.

## Why keep it

Casette connects several of our intended technologies in a published guitar processor: C++, LVGL framebuffer graphics, evdev touch, JACK, NeuralAudio amp processing, cabinet convolution, effects, and presets. Its README reports Pi 4 operation with an 800 × 480 DSI display. This is relevant prior work, but neither the claimed operation nor its demo establishes our hardware's latency, memory use, or available processing capacity.

Beebo remains the modular-workflow reference. Casette supplies implementation lessons for a compact native instrument. Our new square-module canvas, Ingen engine, patch data stored in the engine's bundle, and interface-independent audio remain the current direction.

## Download coverage

The shallow Git checkout contains all 93 tracked files, including its complete README, CMake/toolchain/deployment files, DSP/UI sources, `.nam` captures, and `.wav` cabinet IRs. Its origin and exact commit are recorded in [the manifest](../reference/manifest.json). The working tree is clean.

There is no tracked `.gitmodules` or Git submodule entry. `.gitignore` excludes `deps/`, which is absent. Build references include NeuralAudio, LVGL/lv_port_linux, FFTConvolver, Eigen, JSON, plog, and math_approx; JACK, libgpiod, libsndfile, and aubio are system dependencies. Exact library revisions are not pinned by the published snapshot. The repository alone is therefore not a reproducible build environment. Its scripts and services have not been run.

## Source findings and lessons

Links below refer to the downloaded commit, not a moving default branch.

| Source | Observed behaviour | Lesson for Pedal PI |
| --- | --- | --- |
| [README](https://github.com/pawelKapl/Casette/blob/75ded6663e2d758bfe8a290952c672b2e8c01697/README.md) | Reports DietPi/Pi 4, HiFiBerry DAC2 ADC Pro, 800 × 480 DSI screen, and JACK at 48 kHz, 64 frames, two periods | Useful configuration example; our HDMI/touch/audio interface and OS must be identified independently |
| [UI startup](https://github.com/pawelKapl/Casette/blob/75ded6663e2d758bfe8a290952c672b2e8c01697/src/ui/gui.cpp) | LVGL uses `/dev/fb0` and hardcoded `/dev/input/event0`; fixed tabs and meters | Supports investigating direct native graphics; use stable device identities and our own modular screens |
| [Main callback](https://github.com/pawelKapl/Casette/blob/75ded6663e2d758bfe8a290952c672b2e8c01697/src/main.cpp) | Mono input passes through amp/cab and gate, then is copied to stereo for conventional effects; UI and audio share one executable | Avoid redundant stereo amp inference; retain separate UI/engine processes for R13 |
| [Amp processing](https://github.com/pawelKapl/Casette/blob/75ded6663e2d758bfe8a290952c672b2e8c01697/src/dsp/amp.cpp) | NeuralAudio loader and calibrated input/output adjustments; dual amps processed sequentially for both series and parallel blend | Inspect active runtime and calibration; signal parallelism is not automatic CPU parallelism |
| [Quality reduction](https://github.com/pawelKapl/Casette/blob/75ded6663e2d758bfe8a290952c672b2e8c01697/src/dsp/amp.cpp) | If its reported process CPU exceeds 95 for over 200 consecutive eligible callback checks, supported second-model quality switches to `0.5` once, after a real-time-safety check | Dual-amp availability does not guarantee two full-quality models with spare capacity; our automatic policy remains open |
| [CPU monitor](https://github.com/pawelKapl/Casette/blob/75ded6663e2d758bfe8a290952c672b2e8c01697/src/cpu_monitor.h) | Reads process CPU-time ticks approximately once per 201 UI iterations; assumes a one-second interval rather than measuring elapsed time | Do not adopt this as a precise system/DSP-load monitor; measure elapsed time and audio deadlines separately |
| [Model lifetime](https://github.com/pawelKapl/Casette/blob/75ded6663e2d758bfe8a290952c672b2e8c01697/src/dsp/amp.cpp) | Loading assigns new raw model pointers without deleting the previous ones; destructor deletes only the primary pointer | Source-level ownership issue; verify safe replacement and bounded memory across repeated switches in our chosen implementation |
| [Effect buffers](https://github.com/pawelKapl/Casette/blob/75ded6663e2d758bfe8a290952c672b2e8c01697/src/effects_chain.h) | Declares an array of 4,194,304 floats, 16 MiB for a four-byte float | Illustrates that effects need memory beyond model weights; this declaration is not total application RAM |
| [Storage](https://github.com/pawelKapl/Casette/blob/75ded6663e2d758bfe8a290952c672b2e8c01697/src/storage.h) | Relative-path JSON files and direct writes from parameter setters | Define our own asset paths, atomic saves, and bounded/background persistence |
| [Build](https://github.com/pawelKapl/Casette/blob/75ded6663e2d758bfe8a290952c672b2e8c01697/CMakeLists.txt) | Cortex-A72 optimization and fixed Windows sysroot paths; external dependencies absent | Validate Cortex-A76/compiler choices for Pi 5 and record a reproducible dependency set |

These are source observations and engineering implications. No suspected issue was reproduced on a running Casette device, and upstream files have not been modified.

## NAM feasibility on Pi 5, 2 GB

**Working assessment — inference:** one optimized mono NAM instance plus a cabinet IR and lightweight interface is a reasonable target. 2 GB RAM is plausibly sufficient for a modest patch. This does not establish support for arbitrary captures, multiple full-quality models, or a dense patch combining NAM, Clouds, large reverbs, and loopers. Model/runtime selection, buffers, loaded assets, and the entire audio graph determine the budget.

The local Poly NAM fork at `a3a6520ea519513a6ca1e82d350d47e3c7891732` uses an older NAM Core integration. Its model-path and worker handling can be inspected locally. Current [upstream NAM LV2](https://github.com/mikeoliphant/neural-amp-modeler-lv2/blob/main/README.md) instead uses NeuralAudio. Evaluate that candidate through Ingen rather than replacing the engine with Casette's fixed chain. Exact current plugin/dependency commits must be pinned before building; online inspection does not supply a local build set.

[NeuralAudio documentation](https://github.com/mikeoliphant/NeuralAudio/blob/main/README.md), inspected online on 13 October 2026, describes optimized common A1/A2 architectures and slower fallback paths. Compile options and target architecture affect speed. Composite models can initialize all variants up front or initialize on demand; the latter makes a first quality switch potentially unsafe in a real-time context. Confirm the selected plugin's behaviour rather than assuming that the library options are exposed by its UI.

CPU and RAM require separate measurements. A serial chain may exceed one audio thread's deadline while other cores are idle. At 48 kHz, 64 frames give a 1.33 ms period and 128 give 2.67 ms; neither value is measured end-to-end latency. Resident buffers can be larger than the model file, and repeated asset switching can reveal lifetime problems absent in a short test.

No reproducible CPU/RAM benchmark was found in the downloaded Casette documentation. Our documentation therefore makes no exact CPU-per-model claim, fixed interface RAM limit, or guaranteed concurrent-model count.

## Proposed evaluation

1. Identify actual OS architecture, audio interface, cooling, and existing JACK configuration without applying Casette's service or hardware settings.
2. Compare pinned compatible builds of the Poly fork and a current NeuralAudio-based LV2 candidate through Ingen. Verify path messages, worker/state support, save/restore, and model errors first.
3. Test one mono NAM at its trained sample rate, then add cabinet IR. Compare representative A1 model sizes and supported A2 Lite/Full settings at 64/128 frames where supported.
4. Add Clouds, delay, and reverb incrementally. Measure the complete patch during touch/control activity; only then assess additional NAM instances.
5. Record DSP load and processing peaks where available, per-thread/core CPU, xruns, RSS/PSS and available system memory, temperature/throttling, actual round-trip latency, and model/IR-switch behaviour. Use a representative 30-minute session and repeated changes to find load spikes or accumulating memory.

NeuralAudio's [ModelTest utility](https://github.com/mikeoliphant/NeuralAudio/blob/main/Utils/ModelTest/ModelTest.cpp) can provide an offline comparison; it cannot prove live low-latency stability. The detailed record and acceptance evidence are in [Validation](06-validation.md#nam-and-cabinet-ir--proposed).

## Decisions and remaining uncertainty

- **Agreed:** retain Casette unchanged as another reference and update project documentation.
- **Current direction retained:** our new LVGL interface, Poly Ingen modular engine, and audio-only cables for the initial version.
- **Proposed:** evaluate a current NeuralAudio-based NAM LV2 module and one mono NAM plus IR baseline.
- **Open:** final runtime/build, desired captures, model count, quality policy, memory/DSP margins, and any automatic reduction policy.
- **Unverified:** all builds, host compatibility, Pi performance, touchscreen operation, and startup results.
