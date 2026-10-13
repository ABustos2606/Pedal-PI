# Validation

## Evidence rules

Validate on the actual Pi, display, audio interface, and selected controllers. Record versions and audio settings with each result. Separate source inspection, compilation, application launch, working audio, and comfortable operation; they are different levels of evidence.

No runtime or performance result exists yet for this project.

## Audio

| Check | Evidence required |
| --- | --- |
| Guitar input/output | Correct channels, usable input gain, no unintended clipping, and the intended processed output |
| Round-trip latency | Measured end-to-end latency at documented sample rate and buffer settings; do not substitute buffer arithmetic for a measurement |
| Continuous processing | No audible dropouts or JACK xruns during a representative extended session |
| Resource use | CPU/DSP load, memory, temperature, and throttling recorded for representative patches |
| Graph edits | Add/remove/connect operations do not produce uncontrolled levels, persistent silence, or engine failure |
| Patch changes | Measure gaps, tails, and recovery; do not promise seamless preset changes without evidence |

Intentional delay or granular timing is different from interface/backend latency. Measure the basic audio path separately from algorithms that intentionally delay or reshape audio.

## NAM and cabinet IR — proposed

No NAM benchmark has been run on the user's Pi. Casette's Pi 4 claim, small model files, and its process CPU monitor are not measurements of our complete instrument.

Use the same actual audio interface, capture files, quality settings, and comparable gain levels for each runtime comparison. Record OS architecture, kernel, cooling, governor, compiler/options, plugin/runtime/dependency revisions, model hash/architecture/sample rate, block size and periods, and IR identity/length. Check 64- and 128-sample blocks at the model's sample rate where supported.

| Patch or action | Evidence required |
| --- | --- |
| One mono NAM, then NAM plus cabinet IR | Baseline DSP load, per-thread/core CPU, process RSS/PSS where available, system available memory, xruns, and measured latency |
| A1 model sizes and supported A2 quality settings | Record exact files and active implementation/quality; compare performance and sound without treating different captures as equivalent |
| NAM plus Clouds, then delay/reverb | Full-patch processing headroom and stability with screen gestures and physical control changes |
| Additional NAM instances | Incremental load measurements and explicit limit for the tested patch; no extrapolation from total CPU alone |
| Repeated model/IR/preset changes | No steadily accumulating retained allocations after warm-up; loading gaps, tails, peak memory and recovery recorded |
| Missing/corrupt models or IRs | Useful error, defined audio behaviour, no host crash, and successful recovery with a valid asset |
| Quality changes | Correct visible and restored selection; no unsafe first-use initialization on the audio callback |
| Calibration and cabinet choice | Predictable input/output levels, no unintended clipping, and correct amp-only/full-rig handling |
| Save/reload and interface restart | Exact asset/quality/state restoration and continuous audio when restarting only the interface |

Proposed endurance sample: at least 30 minutes with the intended patch under realistic editing and switching, recording xruns, temperatures, throttling, and memory trend. Extend if peaks or intermittent faults remain. Establish a practical DSP margin from processing peaks and the complete chain; an average below 100% or several idle cores is insufficient. Avoid sustained swapping during musical use.

NeuralAudio's `ModelTest` can help compare runtime throughput, but offline speed is not proof of live JACK stability or round-trip latency. If used, record its build and settings separately from live results. Do not infer per-model RAM from `.nam` file size alone. See [Casette and NAM evaluation](09-casette-and-nam.md).

## Modular processing

- Clouds responds to live input and its key parameters; freeze works for the chosen switch mode.
- Remaining Mutable Instruments-derived families produce their expected audio or control behaviour.
- Audio, CV, scalar-control, and MIDI connections behave according to the host's port semantics.
- Unsupported cable combinations are prevented or explained.
- Invalid/missing modules result in a repairable state rather than silent substitution.
- Test intentional feedback routing only with controlled levels and a suitable limiter strategy; graph feedback is not assumed safe or supported by every host configuration.

## Touchscreen
 
At native 800 × 480, use fingers to add a square module, drag-and-drop it to arrange it on the grid, patch modules in series or parallel using virtual cables, tap a square to enter its internal parameter page (adjust drive, gain, volume, time, etc.), assign a control, save a patch, and return to the patch view. Check browser scrolling, on-screen keyboard, edge handling, and navigation.
 
Test single-touch operation. If multitouch is supported, validate its gestures separately. Text on square modules and parameter pages must remain readable at normal viewing distance, and touch targets must remain reliable without touch errors.

## Interface and engine

- Every cable, module, and parameter value shown matches the engine's actual graph; a failed engine command is reported, not silently displayed as succeeded.
- Killing and restarting the interface does not interrupt audio; on reconnection it shows the engine's current patch, including unsaved edits.
- Starting the interface before the engine is ready shows a clear waiting state and connects once the engine is available.
- Plugins without a custom editor are fully usable through the generic editor.
- The interface's source contains no code, QML, icons, fonts, or assets copied from `digit_ui`.

## Physical controls

Verify correct device identity, direction, ranges, discrete values, acceleration, and fine adjustment. For potentiometers, test pickup behaviour after patch changes. For switches, test bounce, short press, release, hold, and simultaneous operations where used.

Assignments should survive a normal restart. Removing an assigned module should invalidate the mapping visibly. Reconnecting a controller should recover predictable operation without duplicate actions.

## Storage and restoration

Save/reload a patch containing routing, parameter values, control assignments, and external assets where applicable. Confirm startup patch selection separately from the last edited patch. Check graceful handling of missing IRs/models and interrupted saves.

Start with normal shutdown/restart validation. Power-loss recovery is a separate controlled test once the storage design and recovery copy are established.

## Startup

Measure from power application using video/timestamps or equivalent evidence. Record audio readiness and screen responsiveness separately. Test with network access available and unavailable, the actual audio interface attached, and the intended startup patch.

Proposed acceptance sample: five successful cold boots with recorded timings and no manual intervention. The count and numeric targets can be revised after baseline measurement. Include engine/UI startup failure and recovery behaviour in later tests.

## Result record — proposed fields

Test name; date; hardware; OS/kernel/architecture; cooling and governor; source/dependency revisions; compiler/options; plugin URI/version; model/IR hashes and active quality/runtime; sample rate; buffer size and periods; patch; expected result; observed result; DSP/per-thread CPU, memory and thermal measurements; xrun count; latency; duration; pass/fail/untested; and remaining limitation.

Suggested final status values:

- **Passed:** expected behaviour was demonstrated on the target hardware.
- **Failed:** a reproducible issue remains.
- **Not tested:** no evidence yet.
- **Accepted limitation:** the user reviewed the specific limitation and agreed to it.

The last state cannot be assigned merely because a limitation has been documented.
