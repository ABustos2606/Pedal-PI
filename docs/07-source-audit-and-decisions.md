# Source audit and open decisions

## Audit baseline

The following public repositories were first downloaded into the earlier chat workspace for source inspection. Their working trees were clean when checked. The project now also contains a larger unchanged reference collection under `reference/`, including these repositories. No build or deployment has been performed. See [Reference sources](08-reference-sources.md) for the current download inventory and manifest.

| Repository | Inspected commit |
| --- | --- |
| [digit_ui](https://github.com/polyeffects/digit_ui/tree/feb4df451067850770ddd13af9f4e2828234dabc) | `feb4df451067850770ddd13af9f4e2828234dabc` |
| [ingen](https://github.com/polyeffects/ingen/tree/8dae740425f0f1468198e0cdf1e1c61b3fc6ec5e) | `8dae740425f0f1468198e0cdf1e1c61b3fc6ec5e` |
| [lenticular_lv2](https://github.com/polyeffects/lenticular_lv2/tree/141fdc89e250987bae421664132ddaedae735107) | `141fdc89e250987bae421664132ddaedae735107` |
| [PolyLV2](https://github.com/polyeffects/PolyLV2/tree/6f3ae80f6b4eccb8daebfc2b641fde1d48224250) | `6f3ae80f6b4eccb8daebfc2b641fde1d48224250` |
| [poly_filters](https://github.com/polyeffects/poly_filters/tree/ebf8fd1cea20e5d167f0c04ded12da13bd70829f) | `ebf8fd1cea20e5d167f0c04ded12da13bd70829f` |
| [Casette](https://github.com/pawelKapl/Casette/tree/75ded6663e2d758bfe8a290952c672b2e8c01697) — additional reference, downloaded 13 October 2026 | `75ded6663e2d758bfe8a290952c672b2e8c01697` |

These revisions establish what was originally inspected, not the final compatible build set. Submodules were not initialized during that initial inspection; the project reference collection now includes the declared recursive submodule revisions, recorded in its manifest.

## Beebo interface findings — reference only

The new interface is written from scratch (decision D01). These observations from `digit_ui` explain what Beebo assumed and what the new interface must handle in its own way. They are no longer adaptation tasks.

| Area | Source evidence | Lesson for the new interface |
| --- | --- | --- |
| UI technology | `digit_ui/README.md`, PySide2 imports in `show_widget.py` | Beebo uses Qt 5/PySide2 on X11; our toolkit is chosen independently (D12) and should not need a desktop session |
| Screen geometry | 27 QML files reference `1280`, including `TopLevelWindow.qml`, `TitleFooter.qml`, `PatchBay.qml` | Beebo is designed for 1280 × 720; design natively for 800 × 480 |
| Custom controls | About 60 QML files import `"controls"`; that directory is absent in the inspected checkout | Beebo's UI would not launch unchanged; not relevant to a new interface |
| Hardware controls | `pedal_hardware.py` | Fixed device paths and two-encoder model; design our own hardware adapter |
| Hardware bypass | `pedal_hardware.py` | Original GPIO numbers do not describe Raspberry Pi wiring; bypass is out of scope without suitable hardware |
| Input gain | `show_widget.py:set_input_level` | Beebo-specific codec commands; use the chosen audio interface's own controls |
| Persistent paths | `show_widget.py`, `ingen_wrapper.py`, asset-browser modules | Define our own storage layout |
| Maintenance actions | Firmware/reset/import routines in `show_widget.py` | Not carried over; design Pi equivalents only if needed |
| MIDI preset handling | `show_widget.py:midi_pc_thread` | Original waits for `ttymidi`; choose our own controller transport |
| Engine integration | `ingen.py`, `ingen_wrapper.py` | Shows which Ingen messages Beebo uses; a useful guide for our engine client |
| Editor restart | `digit_ui/README.md` | Ingen does not cope with Beebo's frontend restarting; our interface must reconnect and resynchronize (R13) |
| Mutable module build | `lenticular_lv2/Makefile` and `.gitmodules` | Still relevant: ARM compiler options, submodules, and build recipes need checking |
| Module metadata | `module_info.py`, `effect_proto_to_js.py` | Hand-maintained definitions; our catalogue is generated from LV2 metadata plus our own overlay |

## Casette and NAM findings — additional reference

The unchanged Casette checkout contains all 93 tracked files, including bundled `.nam` models and `.wav` IRs, and is clean at the recorded commit. It has no tracked Git submodule entries or `.gitmodules`; its ignored `deps/` folder is absent. The published project therefore does not pin or supply a complete dependency set. No build, deployment script, service, or bundled asset has been executed or installed.

Its source uses LVGL framebuffer/evdev in one executable with a custom JACK callback and fixed effects chain. Both NAM instances execute in that callback, even in the parallel blend mode. Its optional second-model quality reduction and approximate process CPU meter are not evidence of guaranteed spare processing capacity. The model-loading functions replace raw pointers without deleting previous objects; repeated switching warrants memory-ownership tests in our selected implementation. The build targets Cortex-A72 with hardcoded Windows sysroot paths; Pi 5 Cortex-A76 settings need separate validation.

The local Poly NAM fork is an older NAM Core integration. Current upstream plugin/NeuralAudio documentation was inspected online on 13 October 2026 and describes an optimized A1/A2-capable runtime. These online sources are not local dependency snapshots or a selected build. See [the detailed Casette/NAM audit](09-casette-and-nam.md) for source links, limitations, and proposed evaluation.

## Licensing observations

- `digit_ui` has no licence file at the inspected revision; its README states that code without a notice is GPL. The new interface does not copy its code or assets.
- `ingen.py` in `digit_ui` carries David Robillard's ISC-style permission notice.
- Poly's Ingen fork is AGPL-3.0-or-later, as stated in its source headers; `COPYING` contains the AGPLv3 text. The new interface communicates with it as a separate program over its socket protocol. Changes to Ingen retain that license.
- `PolyLV2/COPYING` contains GPLv2, while generated source sections also carry GPLv3-or-later, LGPLv3-or-later and exceptions. Do not assign one license to every file based only on the root license text.
- `lenticular_lv2/LICENSE.txt` contains GPLv2; `LICENSE-dist.txt` includes separate upstream notices, including the MIT notice for Mutable Instruments STM32F modules.
- The Poly NAM LV2 fork includes GPLv3 text in `LICENCE.md`; dependency and file notices remain authoritative for their respective parts.
- The pinned Casette snapshot has no root license file or license notice in its inspected README and C++ files. It remains a technical reference; copying its code or bundled model/IR assets requires establishing the applicable permissions.

**Project license — agreed 13 October 2026:** original Pedal PI code and documentation use GPL-3.0-or-later. The root [LICENSE](../LICENSE) contains the complete GPLv3 text. [Third-party notices](../THIRD_PARTY_NOTICES.md) define its scope and record the inspected upstream evidence. The repository license does not relicense reference sources, dependencies, models, IRs, fonts or other third-party assets. Phase 11 includes a license/source inventory for the actual release components.

## Engine dependency observations

The inspected Ingen `meson.build` requests the following minimum versions, in addition to other dependencies and build tools:

| Library | Minimum in inspected source |
| --- | --- |
| Serd | 0.30.4 |
| Sord | 0.16.15 |
| Sratom | 0.6.0 |
| Suil | 0.10.0 |
| LV2 | 1.18.0 |
| Lilv | 0.24.21 |
| Raul | 2.0.0, using the `raul-2` package identity |

This is a source-level dependency observation. Package availability and build compatibility on the user's Patchbox installation remain unknown. Decide whether to build dependencies, select compatible earlier revisions, or change the development OS only after identifying that installation.

## Decisions

The [technical implementation plan](10-technical-implementation-plan.md), prepared 13 October 2026, provides the authoritative proposed phase/task sequence. Its Windows simulator workflow uses shared UI code and a simulated engine, with early Pi audio validation. Source inspection of Ingen's `src/Serialiser.cpp` confirms bundle serialization using `main.ttl` and `manifest.ttl`; bundle save and reload were then verified at runtime ([evidence](14-ingen-engine-evidence.md)); plugin state inside bundles still needs testing. The companion JSON design was dropped on 13 October 2026 (D13).

| ID | Topic | State |
| --- | --- | --- |
| D01 | Build a new touchscreen interface; Beebo is reference/inspiration only; MODEP is not the main interface | Agreed (revised 13 October 2026) |
| D02 | Use the existing Raspberry Pi 5, 2 GB as starting hardware | Agreed |
| D03 | Design for the user's 5-inch, advertised 800 × 480 screen | Agreed; actual mode/controller unverified |
| D04 | Include Poly's Mutable Instruments-derived modules and compatible MODEP effects | Agreed ambition; individual compatibility unverified |
| D05 | Documentation before implementation | Agreed |
| D06 | Final OS/dependency baseline | Open |
| D07 | Audio interface, channels, sample rate, and buffers | Open; user is flexible and accepts inexpensive USB hardware |
| D08 | Knob/switch counts and transport | Counts agreed 14 October 2026: four push encoders the user owns, three momentary SPST footswitches. Transport proposed: direct GPIO through the kernel's rotary-encoder and gpio-key drivers, to verify on the Pi ([physical controls](18-physical-controls.md)) |
| D09 | Numeric cold-boot and latency targets | Open; measure first |
| D10 | Storage layout, recovery, and shutdown design | Open |
| D11 | Four encoders and three stomp switches | Agreed 14 October 2026, matching the user's hardware |
| D12 | UI toolkit and programming language | Agreed: LVGL (C/C++ on Linux DRM/KMS or framebuffer with evdev touch) |
| D13 | Storage of UI data (positions, assignments) | Agreed, revised 13 October 2026: stored as properties on blocks, ports and the graph inside the engine and its saved bundle. The earlier companion JSON file is dropped |
| D14 | Visual interaction and identity | Agreed interaction: drag-and-drop square modules, tap to enter parameter page, series/parallel audio routing; visual styling and name open |
| D15 | Keep Poly's Ingen fork as the engine | Current direction; the engine client layer keeps substitution possible |
| D16 | Routing cable types | Agreed, revised 13 October 2026: audio cables and control (CV) cables, each patched connector by connector. Audio and control are not joined to each other in the interface. MIDI cables remain deferred |
| D17 | Retain Casette as another unchanged technical reference and update project documentation | Agreed 13 October 2026; downloaded and recorded in the reference manifest |
| D18 | Evaluate a current NeuralAudio-based NAM LV2 plugin against the older Poly fork | Proposed; exact runtime/build and Ingen compatibility unverified |
| D19 | Automatic NAM quality reduction under load | Open; no automatic reduction policy agreed |
| D20 | Shared Windows 800 × 480 LVGL simulator and Pi UI development workflow | Proposed implementation route documented in Phases 1–4; exact LVGL revision, driver and compiler remain to be selected |
| D21 | Detailed phase/task implementation plan | Created at the user's request on 13 October 2026; no implementation phase has been executed |
| D23 | Engine build and protocol | Evidence 13 October 2026: upstream Ingen `13a045d0` + Poly MIDI-learn fix + three project patches, Turtle protocol, verified under WSL2 ([record](14-ingen-engine-evidence.md)). Led to the D13 revision. Pi verification pending |
| D22 | Project license | Agreed 13 October 2026: GPL-3.0-or-later for original Pedal PI files; preserve upstream licenses, including Ingen's AGPL-3.0-or-later |

## Information needed before implementation

1. Intended guitar/output connections and requirements for selecting a suitable USB audio interface.
2. Patchbox OS release, 32/64-bit architecture, kernel, current JACK settings, and active services.
3. Screen controller and actual pixel mode. The user confirms a capacitive panel with five touch points, touch and power over USB and video over HDMI, working under Zynthian and Patchbox/MODEP.
4. Answered 14 October 2026: four 20-pulse push encoders and three momentary SPST footswitches. Still open: the enclosure arrangement, and the encoders' detent count on the bench.
5. Cooling, storage, and whether a spare card is available for development. The power supply is the official 27 W unit.
6. Which conventional effects are essential in the first usable patch, alongside Clouds.
7. Whether later work will have direct access to the Pi or use user-provided diagnostic results.
8. Look-and-feel preferences, colour theme, or branding references for the instrument's visual styling (when ready).
9. Which NAM captures and representative patches to benchmark; desired quality and simultaneous model count remain open rather than guaranteed features.

## Supporting references

- [Beebo module integration guide at inspected revision](https://github.com/polyeffects/digit_ui/blob/feb4df451067850770ddd13af9f4e2828234dabc/add-module.md)
- [Beebo module definitions at inspected revision](https://github.com/polyeffects/digit_ui/blob/feb4df451067850770ddd13af9f4e2828234dabc/module_info.py) — reference only
- [Mutable Instruments-derived build at inspected revision](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/Makefile)
- [Ingen build dependencies at inspected revision](https://github.com/polyeffects/ingen/blob/8dae740425f0f1468198e0cdf1e1c61b3fc6ec5e/meson.build)
- [Blokas MODEP setup](https://blokas.io/modep/docs/setup/)
- [Blokas MODEP interface and plugin location](https://blokas.io/modep/docs/running-modep/)
- [User's display purchase link](https://www.mercadolibre.com.ar/pantalla-tactil-raspberry-pi-lcd-touch-5-800-x-480-hdmi/p/MLA2071447608?pdp_filters=item_id:MLA864457610&sid=purchases)
- [Casette at downloaded revision](https://github.com/pawelKapl/Casette/tree/75ded6663e2d758bfe8a290952c672b2e8c01697)
- [Current upstream NAM LV2 documentation](https://github.com/mikeoliphant/neural-amp-modeler-lv2/blob/main/README.md) — online reference; pin before building
- [NeuralAudio documentation](https://github.com/mikeoliphant/NeuralAudio/blob/main/README.md) — online reference; pin before building

Source links establish provenance. They do not replace target-hardware testing or resolve the open decisions above.
