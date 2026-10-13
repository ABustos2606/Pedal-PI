# Technical implementation plan

**Project:** Pedal PI  
**Prepared:** 13 October 2026 (America/Buenos_Aires)  
**Status:** Phase 2 desktop acceptance complete with a simulated engine. Phase 1 retains Linux-platform and debugger setup work. Phase 0 deferred while the Pi is packed.
**Display target:** 800 × 480.  
**Hardware baseline:** Raspberry Pi 5, 2 GB RAM; Windows PC for everyday interface development.

This plan turns the [requirements](01-requirements.md), [architecture](02-architecture.md), [interaction design](03-interface-and-controls.md), and [validation rules](06-validation.md) into executable phases. It preserves the new LVGL interface, Poly Ingen audio engine, Mutable Instruments-derived modules, and initial audio-only routing. Layout and control assignments are stored as engine properties inside the patch bundle; the earlier companion JSON was dropped on 13 October 2026 (D13). Casette is an unchanged technical reference. Independent Windows UI implementation was authorized on 13 October 2026; see [its development record](11-windows-ui-foundation.md). The Pi installation has not been changed.

Every phase has numbered tasks, a deliverable, and an exit check. A phase is complete only when its evidence exists; a desktop prototype does not establish Pi audio performance or finger usability. Technical choices marked proposed are defaults to evaluate, not new agreed product requirements.

## Development order and checkpoints

| Phase | Main result | Work location | Depends on |
| --- | --- | --- | --- |
| 0 | Hardware/OS profile, backup and audio baseline | PC and Pi | Access to the Pi and selected audio hardware for audio measurements |
| 1 | Shared application skeleton and LVGL simulator | Windows PC | Can start immediately; no Pi dependency |
| 2 | Interactive patch editor using a simulated engine | Windows PC | Phase 1 |
| 3 | Minimal working JACK/Ingen audio path and protocol evidence | Linux build environment and Pi | Phase 0; can overlap Phases 1–2 |
| 4 | Real engine integration and native Pi touch UI | PC and Pi | Phases 2 and 3 |
| 5 | Reliable patch/state/asset persistence | PC and Pi | Phase 4 |
| 6 | Clouds plus conventional effect, edited on the instrument | Pi, with PC UI development | Phases 4 and 5 |
| 7 | Assignable physical controls and first full milestone | PC and Pi/controller hardware | Phases 4–6 and identified controls |
| 8 | NAM/cabinet evaluation and supported resource budgets | PC and Pi | Phases 4 and 5; may start earlier than Phase 7 completion |
| 9 | Tested module catalogue and remaining interface workflows | PC and Pi | Phases 5–7; NAM results from Phase 8 for NAM entries |
| 10 | Automatic startup, readiness and recovery | Pi | Working representative patch from Phases 5–7 |
| 11 | Complete validation and reproducible release package | PC and Pi | Phases 0–10 for the selected release scope |

Phases 1–2 do not wait for the Pi. Phase 3 must pass before building the full interface around assumed engine behaviour. NAM and catalogue expansion do not block the first Clouds instrument milestone. Work can proceed on independent tasks without completing every phase strictly in numeric order.

**Checkpoint A — desktop editor:** end of Phase 2.  
**Checkpoint B — real audio controlled through our UI:** end of Phase 4.  
**Checkpoint C — first complete musical milestone:** end of Phase 7, including saved Clouds patch, one conventional effect, one knob and one stomp switch.  
**Checkpoint D — candidate dedicated instrument:** end of Phase 10 with the selected tested module set.  
**Checkpoint E — validated release:** end of Phase 11.

## Technical structure to establish

Proposed language/build baseline: C++20 and CMake, subject to compatibility checks against the selected compiler and pinned LVGL release in Phase 1. Use one pinned LVGL revision for Windows and Pi builds. Do not choose moving upstream defaults as a reproducible dependency specification.

| Component | Responsibility | Key boundary |
| --- | --- | --- |
| Application/domain core | Module identities, port descriptions, connections, typed values, pending edits, assignments | No LVGL, JACK or GPIO dependency |
| Engine adapter | Graph commands, snapshots, updates, errors and connection state | One interface implemented by simulated and Ingen adapters |
| LVGL views | Canvas, parameters, browser, patches, assignments and settings | Consumes application state; LVGL operations stay on the UI thread |
| Platform layer | Display, input, clock, paths and process integration | Windows simulator versus Linux DRM/framebuffer and touch |
| Catalogue | Versioned LV2 metadata export plus project-owned presentation overlay | Linux discovery is independent of the Windows simulator |
| Persistence | Ingen graph/state bundle carrying layout and assignments as properties, asset references and committed revisions | Uses verified host serialization; never reconstructs plugin state from sliders alone |
| Controls | Device events, debounce, mapping, ranges and action dispatch | No blocking reads or persistence on an audio callback |
| Diagnostics | Engine readiness, DSP/xruns, memory and thermal observations | Labels real measurements separately from simulated values |

Proposed future workspace layout: `src/core`, `src/ui`, `src/engine`, `src/platform/windows`, `src/platform/linux`, `src/catalogue`, `src/storage`, `src/controls`, `tools`, `tests`, `assets`, and `config`. Build outputs go in a separate ignored build directory. Upstream snapshots remain unchanged under `reference/`; implementation dependencies and patches are recorded separately.

The engine is the authority for live DSP state. The UI model is its synchronized representation plus editor-only metadata. Pending commands must not appear as accepted engine changes until acknowledged or observed in authoritative updates. This clarifies the architecture's single-source-of-truth rule for views without making the interface responsible for keeping audio alive.

## Phase 0 — identify and preserve the Pi

**Purpose:** establish the actual system before choosing build dependencies or touching the existing installation.

- [ ] **P0.1 — System profile:** record board/RAM, OS release, userland architecture, kernel, storage/free space, power supply, cooling, CPU governor, network and available remote access.
- [ ] **P0.2 — Service inventory:** record active JACK/MODEP/PipeWire/PulseAudio services where present, current audio ownership, startup configuration and installed LV2 paths. Identify potential conflicts without changing them during inventory.
- [ ] **P0.3 — Audio profile:** identify the selected interface, channels, guitar input suitability, device identity, gain controls, sample rates and usable period/buffer configurations. If the interface is not selected, record selection criteria and leave audio tests blocked by that specific dependency.
- [ ] **P0.4 — Display/input profile:** confirm actual 800 × 480 output, DRM/framebuffer availability, touchscreen USB/input identity, calibration, orientation and single-touch operation.
- [ ] **P0.5 — Controls profile:** identify the owned knobs as encoders or potentiometers, their count and connection; identify stomp-switch hardware and transport options. Record unresolved wiring rather than assigning arbitrary GPIO pins.
- [ ] **P0.6 — Preservation:** make a recoverable backup or development SD card, record restoration steps, and choose an isolated development prefix/plugin path. Preserve the working MODEP installation until replacement passes validation.
- [ ] **P0.7 — Baseline:** measure current power-on-to-audio and power-on-to-screen separately; record latency, xruns, memory and temperature for an available representative patch.

**Deliverable:** hardware/OS profile, existing service/plugin inventory, baseline measurement record and recovery procedure.

**Exit check:** Linux architecture and device identities are known; development changes have a recovery path. Missing audio/control hardware is explicitly recorded. Phase 1 can proceed even if those items remain unresolved.

## Phase 1 — shared code and Windows simulator

**Purpose:** make everyday UI work possible on the PC without the Pi.

**Execution record — 13 October 2026:** P1.2 and P1.4–P1.8 are complete. The Windows Debug build and all three CTest checks passed. Domain and engine contracts, missing-module scenarios, and build/run/debug instructions are documented. VS Code launch/task configuration is prepared; its C/C++ debugger extension is not installed on this PC, so an actual breakpoint session remains part of P1.1. Linux UI platform work remains P1.3. See [UI development](11-windows-ui-foundation.md) and [domain/engine contracts](13-domain-and-engine-contracts.md).

**P1.4 correction:** a user-reported blank native window exposed a startup refresh race missed by the original launch-only check. The initial redraw now waits for the Windows buffer, and the native test verifies rendered pixels and a completed flush without requiring a click. See the [startup fix and regression evidence](11-windows-ui-foundation.md#verification).

| Task | Implemented | Remaining before completion |
| --- | --- | --- |
| P1.1 | Windows MSVC/CMake/SDK inventory, selected build preset and prepared VS Code launch/task configuration | C/C++ debugger extension and actual breakpoint validation; Linux build route |
| P1.3 | C++20/CMake core, engine, shared LVGL views and Windows platform; source/license notices | Linux UI build target and platform setup |
| P1.5 | Complete: stable identity/display-name separation, explicit port types/channel symbols, tagged numeric values, validated ranges/graphs and retained missing-module metadata | Real plugin discovery/adaptation belongs to Phases 3–4 |
| P1.6 | Complete: abstract Engine, connection lifecycle, correlated results/snapshots, create/rename/load/session-save and capability failures | Actual protocol and persistent plugin state belong to Phases 4–5 |
| P1.7 | Complete: deterministic catalogue, delayed/rejected edits/save, missing-module graph fixture and disconnect/reconnect | No audio processing is claimed |
| P1.8 | Complete: build/run/test/debug instructions, VS Code configurations, pinned dependency and reviewed captures | Debugger installation/runtime validation remains P1.1 |

- [ ] **P1.1 — Toolchain inventory:** identify available Windows C++ compiler, CMake, debugger and dependency tooling. Select a supported combination and document setup; choose a Linux build route for later Pi deployment separately.
- [x] **P1.2 — Pin graphics dependencies:** select a stable LVGL revision and compatible Windows display/input backend. Prefer the official Win32 simulator when it fits the available compiler; SDL is a documented alternative. Record revisions, licenses and required fonts/assets; preserve supplied notices.
- [ ] **P1.3 — Application skeleton:** create the proposed shared/platform separation and build targets for a Windows simulator and later Linux UI. Keep simulator drivers out of the shared application logic. Add GPL-3.0-or-later SPDX and accurate copyright notices to original project source files; keep adapted upstream files under their applicable terms.
- [x] **P1.4 — Display baseline:** run a real LVGL application with an 800 × 480 logical drawing area, mouse pointer input and keyboard input. Account for Windows DPI scaling; optional zoom changes presentation, not logical coordinates.
- [x] **P1.5 — Domain contracts:** define stable instance IDs, plugin URIs, port symbols/types/directions, parameter value types/ranges/units, connection records, canvas positions and assignment records. Separate engine identity from user display names.
- [x] **P1.6 — Engine contract:** define connect/disconnect, snapshot, add/remove module, connect/disconnect ports, set parameter, load/save and state/error events. Define pending/success/failure semantics and unsupported-capability reporting before coding protocol details.
- [x] **P1.7 — Simulated engine:** provide a small deterministic catalogue, graph state, delayed responses, rejected commands, missing modules and disconnect/reconnect scenarios. Simulated graphs do not process audio, and simulated resource values are labelled as such.
- [x] **P1.8 — Developer loop:** document reproducible build/run/debug steps and capture the initial screenshot. Pin dependencies so an ordinary rebuild does not silently upgrade LVGL.

**Deliverable:** runnable Windows LVGL simulator, shared domain model, engine interface, simulated backend and setup instructions.

**Exit check:** launch repeatedly on the PC at 800 × 480 without the Pi or internet, exercise a command/update exchange, and switch between ready/disconnected/error states.

**Evidence sources:** [official Windows simulator](https://github.com/lvgl/lv_port_pc_visual_studio), [official SDL driver documentation](https://docs.lvgl.io/master/integration/pc/sdl.html). Exact selected versions are recorded during execution rather than inferred from these moving documentation links.

## Phase 2 — build the patch editor on the PC

**Purpose:** validate the core interaction before audio integration expands its scope.

**Execution record — 13 October 2026:** P2.1–P2.8 and the desktop exit check are complete. Actual LVGL pointer/keypad checks cover creation/naming/rename, cancellation and unsaved-replacement guards, pagination, catalogue availability, missing-module repair, all core editor workflows and rejected/pending operations. Sparse, panned, dense, long-name, keyboard and error views are reviewed at 800 × 480; domain tests cover all mono/stereo expansion rules. Logical cables retain physical routing plans using demo or supplied channel symbols; actual Ingen ports and merge levels remain unverified. See the [desktop review checklist](12-desktop-editor-review.md). Physical switch assignment remains Phase 7; Pi touch acceptance remains Phase 4.

| Task | Implemented | Remaining before completion |
| --- | --- | --- |
| P2.2 | Complete: named squares, movement threshold, snapping, 44-pixel jack hit areas, empty-space panning and long-press actions (parameters, software bypass, duplicate, delete) | Physical control assignment is tracked in Phase 7 |
| P2.3 | Complete: preview, valid targets, selection/delete, split/merge drawing, errors and physical connection plans | Verify actual Ingen port identities in Phases 3–4 |
| P2.4 | Complete: direction/type/channel, fixed endpoint, duplicate and cycle validation; four mono/stereo rules | Actual merge levels/compensation remain Phase 3 measurements |
| P2.5 | Complete: paginated categorized catalogue with available/untested/unavailable status and reasons | Actual installed/tested plugins remain later integration |
| P2.6 | Complete: tagged controls, logarithmic/integer ranges, units, software bypass and parameter pagination | Physical touch review remains Phase 4 |
| P2.7 | Complete: new/rename/edit/duplicate/delete, cleanup, pending/rejected commands, session save/restore and backend updates | Disk/plugin-state persistence remains Phase 5 |
| P2.8 | Complete: recorded sparse/dense/long-name/edge/keyboard/error review and meaningful channel/range checks | Pi usability and audio acceptance remain unverified |

- [x] **P2.1 — Canvas and header:** fixed IN/OUT modules, patch name, dirty marker, readiness state, add-module, patches and settings navigation. Begin with the sizing targets in the interaction document and use a shared theme.
- [x] **P2.2 — Module interaction:** named square modules, grid snapping, tap-to-open parameters, movement threshold, drag positioning, empty-space panning and context actions. Keep port hit areas large enough for the intended screen.
- [x] **P2.3 — Cable interaction:** output-to-input drag preview, valid-target indication, cable selection/removal, series split/merge display and clear rejected-connection messages. Represent each logical cable's eventual physical engine connections.
- [x] **P2.4 — Graph validation:** enforce port direction/type compatibility, fixed endpoint rules and rejection of feedback cycles. Implement mono/stereo expansion as explicit rules; leave merge compensation provisional until Phase 3 measurements.
- [x] **P2.5 — Module browser:** categories, names and availability status from the simulated catalogue. Keep unavailable or untested modules distinguishable from supported modules.
- [x] **P2.6 — Generic parameters:** continuous/logarithmic values, integers, toggles, enumerations and units; paginate crowded pages. Prevent out-of-range values and distinguish plugin bypass from a hardware bypass claim.
- [x] **P2.7 — Simulated workflows:** create a patch, edit it, duplicate/delete a module, remove related cables, and exercise pending/rejected operations. Include graph changes arriving from the backend rather than only from local gestures.
- [x] **P2.8 — Desktop review:** inspect sparse and dense patches, long names, edge gestures, keyboard overlays and error messages at 800 × 480. Exercise meaningful domain tests for graph validation, channel expansion and range conversion.
- [x] **P2.9 — Module-specific UI and parallel starter (user-requested extension, 13 October 2026):** Clouds Texture / Mix-hold pages and Delay Character controls, shared generic fallback for changed metadata, and an editable parallel demo with guarded replacement. Pointer checks cover custom edits, mode rejection, fallback value continuity, branch delete/reconnect, and cancelled/rejected demo loading; all three desktop tests pass. Review captures are recorded in the [desktop checklist](12-desktop-editor-review.md). Real audio summing remains Phase 3.
- [x] **P2.10 — Global tempo and CPU readouts (user-requested extension, 13 October 2026):** bottom-right footer on every page, with space retained for cable actions. Shared engine telemetry provides optional BPM and CPU percent; the simulator supplies explicit display fixtures (120 BPM / 12% CPU SIM) and clears CPU when disconnected/connecting/in error. Unknown or invalid readings show `--`. Desktop rendering and existing interaction checks pass; real tempo synchronization and CPU measurement remain engine/Pi integration work.

- [x] **P2.11 - Reference metadata, multi-port jacks and live controls (13 October 2026):** catalogue fixture generated from the unchanged Mutable-derived `.ttl` files (the Windows part of P4.4), several audio jacks per side with port-aware cables, and continuous parameters streamed during the gesture with newest-value coalescing (the UI half of P4.2). Domain and pointer checks cover real stereo symbols, a two-input module, CV-only refusal and mid-gesture engine updates. Host behaviour is still unverified.

**Deliverable:** interactive desktop editor using actual shared LVGL code and a recorded review checklist.

**Exit check:** complete add → drag → connect → edit → disconnect → delete without invalid graph state or clipped essential controls. Backend rejection must revert/resolve pending UI state. Finger usability remains unverified until Phase 4.

## Phase 3 — prove JACK, Ingen and the protocol on the Pi

**Purpose:** verify the engine assumptions while the desktop editor is still small.

**Execution record - 13 October 2026 (PC only):** the hardware-independent parts of P3.1 and P3.5-P3.8, and the bundle format of P5.2, were verified on x86-64 Debian under WSL2 with JACK's dummy backend; see [Ingen engine evidence](14-ingen-engine-evidence.md). Poly's published `main` does not compile (it needs an unpublished serd), so the engine is upstream Ingen plus Poly's MIDI-learn fix and two project patches. The tasks below stay open until repeated on the Pi with the real interface and plugins.

- [ ] **P3.1 — Build baseline:** select Poly Ingen and dependency revisions compatible with the Phase 0 OS. Check the inspected Serd/Sord/Sratom/Suil/LV2/Lilv/Raul requirements; choose build dependencies or an alternative supported development image from evidence.
- [ ] **P3.2 — Linux builds:** establish a native Pi or reproducible Linux cross-build route with correct architecture and sysroot. Do not transplant Casette's Windows paths, Cortex-A72 target or unpinned dependency folder.
- [ ] **P3.3 — Isolated engine run:** launch JACK and Ingen with explicit device/plugin paths in the development setup. Avoid competing MODEP graph ownership and record every configuration change and its rollback.
- [ ] **P3.4 — Simple effect:** build/load one compatible low-cost LV2 effect and confirm guitar input → effect → output, gain behaviour and explicit JACK port routing. Record audio settings and observed xruns/latency.
- [ ] **P3.5 — Protocol proof:** verify socket address, connection lifecycle, Turtle/Atom message structure, framing, request/status correlation, subscriptions and full graph retrieval. Capture sanitized examples for add/remove/connect/disconnect/set/read operations.
- [ ] **P3.6 — Host features:** confirm selected LV2 feature support, worker scheduling, state save/restore, path messages and optional bypass semantics. Inspect source as a guide, then verify runtime behaviour; source presence alone is not a pass.
- [ ] **P3.7 — Routing semantics:** test mono/stereo connections, fan-out, multiple connections into one input, summed levels and needed compensation/mixer behaviour. Record a valid first-version policy before enabling merges in the real UI.
- [ ] **P3.8 — Reconnection spike:** connect a minimal client, disconnect it, reconnect and retrieve the active graph without disturbing audio. Investigate any failure in Poly's fork before implementing a full UI around it.
- [ ] **P3.9 — Host suitability decision:** document passing features and any adaptations. Keep Ingen as the direction; propose a host substitution only if a reproducible blocker prevents core requirements, with the concrete impact recorded.

**Deliverable:** reproducible engine/plugin build record, working saved example graph, protocol examples and host-capability/routing results.

**Exit check:** audible processing is stable, graph commands are verified and the client can reconnect while audio continues. If a required host feature fails, resolve that specific issue before expanding dependent work.

## Phase 4 — real engine integration and native Pi graphics

**Purpose:** turn the desktop editor into an instrument controller without coupling audio to the UI process.

- [ ] **P4.1 — Ingen adapter:** implement the verified protocol using suitable RDF/Atom serialization/parsing, complete-message handling and bounded input. Do not parse Turtle by splitting arbitrary periods or assuming each socket read is a full message.
- [ ] **P4.2 — Communication threading:** isolate socket I/O from LVGL drawing, queue normalized events to the UI thread, bound queues and coalesce redundant continuous-parameter updates. Preserve discrete switch actions and graph operations.
- [ ] **P4.3 — State synchronization:** load a complete engine snapshot, apply subsequent updates and correlate pending edits with results. Define snapshot completion and update ordering so reconnect cannot lose edits or duplicate modules.
- [ ] **P4.4 — Metadata discovery:** export installed LV2 metadata on Linux using the selected host/Lilv facilities. Generate a versioned catalogue consumed by the UI; use an exported fixture on Windows so PC work does not need native LV2 binaries.
- [ ] **P4.5 — Pi platform backend:** build the same views/domain code for Linux with a validated DRM/KMS or framebuffer driver and evdev/libinput touch. Configure stable device selection, 800 × 480 output and rotation/calibration where needed.
- [ ] **P4.6 — Error and reconnect states:** show starting/disconnected/reconnecting/plugin-failed status, reject unavailable operations, and recover the current graph after UI-only restart. A transient communication error must not launch a competing audio engine.
- [ ] **P4.7 — Initial live workflows:** add/connect/edit/remove the simple effect on the physical screen; verify actual audio against the displayed graph. Test source/output endpoints, invalid cables and merge policy from Phase 3.
- [ ] **P4.8 — Deployment loop:** document copying/installing the UI to an isolated Pi prefix and starting/restarting only the UI over configured remote access. Keep Windows simulation independent; direct Windows-to-Pi Ingen control is optional and requires an explicitly verified transport.

**Deliverable:** real Ingen adapter, exported catalogue, native Pi LVGL build and working touch-controlled simple patch.

**Exit check:** graph and parameters shown match the engine; killing/restarting the UI preserves audio and recovers current unsaved engine state. Complete finger tasks at 800 × 480 and measure UI resources without assuming a RAM ceiling.

## Phase 5 — patches, plugin state and assets

**Purpose:** preserve the entire musical patch, not merely its screen positions and numeric values.

- [ ] **P5.1 — Storage contract:** assign stable patch identities, schema versions and display names. Use one Ingen graph/state bundle per patch, with positions, view state and control assignments stored as `urn:pedalpi:ns#` properties on blocks, ports and the graph.
- [ ] **P5.2 — Confirm bundle format:** verify actual host output and load behaviour. The inspected Ingen serializer supports `.ingen` directory bundles containing `main.ttl`, `manifest.ttl` and related state; do not assume the `.ingen` path always names one flat file.
- [ ] **P5.3 — Property schema:** define the project properties for positions, colours/names, assignments and view settings, with a schema version on the graph. Missing or unreadable properties should permit usable audio graph loading and default layout recovery.
- [ ] **P5.4 — Asset policy:** define model/IR/sample locations, referenced versus imported assets, portable state/path mapping and missing-file detection. Store plugin assets/state through host-supported mechanisms and verify relocation rather than rewriting paths blindly.
- [ ] **P5.5 — Complete saves:** save the bundle into staging, wait for the engine's reply, then publish it with an atomic rename where the platform supports it. Preserve the previous complete revision until the new one is in place.
- [ ] **P5.6 — Patch manager:** load, save, save as, rename, delete and select startup patch, with touch keyboard and explicit handling of unsaved changes. Renaming a display label must not invalidate instance/control identities.
- [ ] **P5.7 — Restore/error handling:** recover routing, values, plugin state/assets and UI metadata; report absent plugins, wrong versions and invalid mappings without silent substitution. Distinguish the selected startup patch from the currently edited patch.
- [ ] **P5.8 — Persistence evidence:** test stateful/file-based plugins, save/reload, a bundle with its project properties removed, missing assets and interrupted staging. Validate schema migration and revision selection with meaningful tests; defer abrupt power-loss tests until recovery storage is ready.

**Deliverable:** patch manager, documented/versioned storage format, saved examples and recovery results.

**Exit check:** reload reproduces the intended graph, sound, metadata and asset references; an incomplete save does not overwrite the last complete revision. The UI can rebuild a layout when the layout properties are missing.

**Source evidence:** [Ingen serializer at inspected revision](https://github.com/polyeffects/ingen/blob/8dae740425f0f1468198e0cdf1e1c61b3fc6ec5e/src/Serialiser.cpp).

## Phase 6 — Clouds and the first musical patch

**Purpose:** validate the distinctive granular module before broad catalogue expansion.

- [ ] **P6.1 — Build dependency record:** select the Clouds-derived target, capture its Eurorack/Parasites revisions and required ARM build changes, and keep changes outside the unchanged reference snapshot.
- [ ] **P6.2 — Binary/host check:** verify architecture, dynamic dependencies, URI, required features, ports and intended operating mode through Ingen.
- [ ] **P6.3 — Editor integration:** generate the generic parameter page and project-owned overlay for key granular controls, ranges, units and freeze. A custom Clouds editor is optional after the generic page is usable.
- [ ] **P6.4 — Musical validation:** exercise live input, wet/dry, pitch/grain controls, freeze and supported modes; check levels, unintended silence and state restoration.
- [ ] **P6.5 — Milestone patch:** build input → Clouds plus one conventional effect → output, using series or validated parallel routing as musically appropriate. Save it through Phase 5 workflows and reload it on the instrument.
- [ ] **P6.6 — Combined load:** record DSP, memory, xruns, latency and thermal behaviour while changing parameters and navigating. Document limitations before adding more expensive modules.

**Deliverable:** supported Clouds catalogue entry, saved representative patch and resource/behaviour record.

**Exit check:** the patch can be built and edited with fingers, heard, saved and restored; UI restart leaves audio running. Physical assignments needed for the full first milestone are completed in Phase 7.

## Phase 7 — physical controls and performance assignments

**Purpose:** make the saved patch usable with owned knobs and selected switches.

- [ ] **P7.1 — Select transport:** choose USB MIDI, Linux input or an appropriate GPIO/ADC adapter from the hardware profile. Document wiring and device identification only after the actual components are known.
- [ ] **P7.2 — Simulated controls:** map desktop keys/wheel or explicit simulator controls to the same normalized events as the hardware adapter. Use this to develop assignment workflows without the Pi.
- [ ] **P7.3 — Hardware events:** implement nonblocking reads, reconnect detection, debounce, press/release/hold and encoder direction/detent handling. Queue events independently of drawing work.
- [ ] **P7.4 — Mapping rules:** support relative encoder increments, bounded acceleration/fine adjustment, potentiometer pickup where applicable, inversion, numeric/logarithmic ranges and discrete values. Avoid jumps when changing patches.
- [ ] **P7.5 — Assignment UI:** select a control and target parameter/action, preview its effect, pin assignments and persist them as properties on the target port or graph. Show and disable orphaned mappings rather than targeting a different module accidentally.
- [ ] **P7.6 — Musical actions:** validate one knob parameter and one stomp action first; include freeze/toggle, then tap tempo and patch changes where supported. Preserve urgent discrete actions when coalescing continuous controls.
- [ ] **P7.7 — Resilience:** verify controller unplug/reconnect, UI restart, patch reload, bounce, rapid gestures and removal of an assigned module. Record any control interruption during UI failure; uninterrupted physical control during UI downtime is a separate architecture decision if required.

**Deliverable:** hardware adapter, assignment screen, saved mappings and first fully controlled Clouds patch.

**Exit check:** the instrument satisfies the proposed first milestone: saved patch, Clouds, conventional effect, one knob, one stomp switch, touch editing and a normal restart cycle.

## Phase 8 — NAM and cabinet IR evaluation

**Purpose:** establish what NAM can run without assuming unlimited capacity from Casette's feature list.

- [ ] **P8.1 — Candidate versions:** pin a compatible current NeuralAudio-based LV2 plugin and dependencies for comparison with the downloaded Poly NAM fork. Record compiler/target optimizations and actual model implementation selected.
- [ ] **P8.2 — Host integration:** verify model-path messages, worker/state handling, maximum block size, error recovery and model lifetime in Ingen. No loading, allocation or unsafe first-use quality initialization belongs on the real-time callback.
- [ ] **P8.3 — NAM/cabinet pages:** add local selectors, active file names, calibrated input/output controls and supported quality settings. Confirm whether each capture is amp-only or already includes a cabinet.
- [ ] **P8.4 — Single-model baseline:** benchmark one mono NAM, then NAM plus IR, at the capture's sample rate. Compare representative A1 categories and supported A2 quality levels at 64/128 frames where supported; record exact model/IR hashes and quality.
- [ ] **P8.5 — Combined patches:** add conventional effects and Clouds incrementally, then evaluate additional models only if the complete chain retains headroom. Parallel cables alone do not imply multicore processing.
- [ ] **P8.6 — Endurance and switching:** run a representative 30-minute session, repeat model/IR/preset changes and inspect memory trends, load spikes, xruns, temperatures and throttling. Include corrupt/missing assets and quality changes.
- [ ] **P8.7 — Catalogue decision:** publish supported model/runtime conditions and tested patch budgets, mark failures/untested combinations explicitly, and retain user-selected quality. Automatic quality reduction remains an open product decision.

**Deliverable:** NAM/IR compatibility results, measured comparison, working saved NAM patch and documented capacity limits for tested configurations.

**Exit check:** a selected NAM patch restores its assets/quality and remains stable under realistic use. Failed or incomplete NAM evaluation does not invalidate the already working Clouds milestone; it remains an explicit outstanding candidate feature.

## Phase 9 — complete the catalogue and interface workflows

**Purpose:** expand coverage without advertising installed plugins as automatically supported.

- [ ] **P9.1 — Inventory:** reconcile the agreed Mutable families and essential MODEP-compatible effects with the actual installed/source inventory. Classify every entry as supported, needs adaptation, unavailable or not tested.
- [ ] **P9.2 — Reproducible builds:** build candidate families incrementally with captured revisions, architecture flags and dependency fixes. Separate platform changes from unrelated upstream modifications.
- [ ] **P9.3 — Compatibility records:** verify each URI's required features, audio/control/CV/event ports, save/restore, external assets and representative behaviour through Ingen. The first-version canvas remains audio-only; mark modules that need deferred CV/MIDI routing clearly.
- [ ] **P9.4 — Presentation overlays:** add project-owned categories, names, parameter grouping and suitable value editors. Use generic pages first; add custom editors only when they materially improve musical operation.
- [ ] **P9.5 — Complete screens:** finish browser filtering, patch management, assignment management, diagnostics/settings, shutdown navigation and repair states for missing plugins/assets.
- [ ] **P9.6 — Usability and load:** exercise sparse/dense graphs, long names, touch keyboard and navigation on the physical screen. Record module costs and tested combination budgets rather than one universal module count.

**Deliverable:** versioned supported catalogue, completed interface workflows and per-entry compatibility matrix.

**Exit check:** all entries in the selected release inventory have a result; every entry advertised as supported passes its required evidence. Full collection completion still requires resolving remaining agreed entries or explicit acceptance of limitations.

## Phase 10 — dedicated startup and recovery

**Purpose:** turn the tested application into an unattended instrument.

- [ ] **P10.1 — Service boundaries:** configure JACK, Ingen and UI startup separately with correct user/device permissions and a deliberate restart policy. Normal UI failure must not restart Ingen or JACK.
- [ ] **P10.2 — Readiness:** gate startup on usable devices, responsive engine, successful patch restoration and explicit routing. Provide waiting/error states rather than relying on fixed sleeps or a splash screen.
- [ ] **P10.3 — Offline operation:** ensure startup and everyday playing/editing do not wait for network or model downloads. Cache the tested catalogue/assets locally.
- [ ] **P10.4 — Startup patch:** select and restore the intended saved revision, handle missing plugins/assets and show a repairable result without silently substituting a different musical patch.
- [ ] **P10.5 — Shutdown/storage:** finish saves before graceful shutdown, document safe recovery and decide whether an overlay/read-only root is needed after measuring its implications for persistent user storage.
- [ ] **P10.6 — Cold-boot measurements:** perform the proposed five cold boots and measure power-on-to-audio and power-on-to-responsive-editor separately. Optimize observed delays; set numeric targets from the measured baseline.
- [ ] **P10.7 — Failure recovery:** exercise UI crash, late audio-device availability, engine startup failure and controller reconnect. Distinguish UI recovery with continuous audio from engine recovery where an audio interruption is expected and must be reported.

**Deliverable:** verified service configuration, startup patch behaviour, shutdown/recovery instructions and measured boot results.

**Exit check:** repeated unattended boots reach the intended usable patch and controls without login or desktop interaction, including offline operation. UI-only failure preserves audio.

## Phase 11 — validation, packaging and handoff

**Purpose:** make the result reproducible and state precisely what is supported.

- [ ] **P11.1 — Requirements matrix:** map R01–R13 to named test results on the actual hardware. Treat NAM and extra model counts as separately recorded candidate capabilities, not implied requirements.
- [ ] **P11.2 — End-to-end sessions:** test representative Clouds, conventional and supported NAM patches under realistic screen/control activity; record DSP/per-thread CPU, xruns, memory, latency and thermal stability.
- [ ] **P11.3 — Lifecycle regression:** verify normal boot/shutdown, UI restart, engine failure recovery, saved revisions, interrupted writes, asset relocation where supported, missing plugins and controller reconnect.
- [ ] **P11.4 — Provenance and licenses:** record all upstream and project commits, dependency pins, compiler options, plugin URIs, fonts/assets, license/version grants, exceptions and applicable notices. Inventory the selected modules and their complete dependencies; preserve Ingen's AGPL-3.0-or-later terms. Establish permissions for every distributed model/IR/asset. Ensure the new interface contains no copied Beebo code or assets. Follow [third-party notices](../THIRD_PARTY_NOTICES.md).
- [ ] **P11.5 — Installation package:** prepare repeatable build/install steps, configuration templates, supported plugin/asset inventory and rollback instructions. Include required license/copyright notices and the corresponding source and build/install material required by the shipped components' terms, with an exact-version source retrieval record. Validate them on spare storage or a recoverable installation, not only the development tree.
- [ ] **P11.6 — User operation guide:** document patching, parameters, assignments, models/IRs, save/startup patch selection, shutdown, diagnostics and known limitations with actual screenshots.
- [ ] **P11.7 — Release record:** report passed, failed, not-tested and explicitly accepted limitations. Do not mark the full instrument complete while agreed requirements remain unresolved.

**Deliverable:** versioned release candidate, reproducible manifest, validation matrix, operating guide and recovery package.

**Exit check:** agreed release requirements have evidence, installation can be reproduced, and all remaining limitations are visible. A compiler success or a screenshot is not release acceptance.

## Verification strategy and phase records

Use focused automated tests for failure-prone shared logic: graph cycle/channel rules, parameter conversions, protocol parsing, event ordering/reconnect, metadata schemas, assignment pickup and coherent persistence. Use Windows interaction checks for actual view workflows. Use live Pi tests for audio/plugin behaviour, finger operation, GPIO/MIDI, device permissions and boot. Avoid tests that simply mirror widget construction or repeat already passed checks without a new change or unresolved issue.

Each phase record includes task IDs, date, source/build versions, hardware/audio settings where relevant, expected/observed behaviour, result, evidence files and blockers. Screenshots show UI progress; saved graphs and measurement logs show audio/state progress. Keep simulated results labelled separately from target-hardware results.

## Requirements coverage

| Requirement | Primary phases | Acceptance focus |
| --- | --- | --- |
| R01 automatic start | 10–11 | Unattended power-on reaches usable patch |
| R02 restore saved patch | 5, 7, 10–11 | Routing, values, state/assets and assignments recover |
| R03 modules/cables | 2, 4, 9 | Real graph and touch editor stay consistent |
| R04 usable parameters | 2, 4, 6, 9 | Correct value types and finger operation |
| R05 guitar processing | 0, 3–4, 11 | Measured correct stable input/output |
| R06 Mutable processing | 6, 9, 11 | Clouds first, then recorded family results |
| R07 MODEP-compatible LV2 | 3, 9, 11 | Per-plugin binary/host/editor/state verification |
| R08 knob assignments | 7, 11 | Correct mappings, pickup/ranges and persistence |
| R09 switch assignments | 7, 11 | Actions, timing, debounce and persistence |
| R10 offline operation | 1–2, 5, 10–11 | Local simulator and installed instrument do not require internet |
| R11 patch management | 5, 9, 11 | Save/load/rename/delete/startup selection |
| R12 new native LVGL UI | 1–2, 4, 9, 11 | Shared project-owned UI at 800 × 480; Pi runs without desktop |
| R13 independent audio | 3–4, 10–11 | UI restart preserves audio and resynchronizes graph |

## Decisions that remain open

The final OS/build baseline, audio interface, controller transport/count, visual theme, LVGL/backend revision, release plugin inventory, NAM runtime/quality policy and numeric boot/latency/resource targets remain open. Resolve routine build choices from compatibility evidence; ask for user preferences when they change musical behaviour, hardware selection or the intended scope. Record the decision in [the audit log](07-source-audit-and-decisions.md) before relying on it across phases.

The immediate implementation starting point, when requested, is **Phase 1 on the PC**. Phase 0 can begin when Pi access is available; Phase 3 should follow early enough to settle engine assumptions before catalogue expansion.
