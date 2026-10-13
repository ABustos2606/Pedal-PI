# Pedal PI

Modular effects instrument for Raspberry Pi 5, with a new touchscreen interface inspired by Poly Effects' Beebo.

**Status:** documentation and upstream reference sources; implementation has not started.  
**Date:** 12 October 2026 — revised 13 October 2026 (LVGL direction, Casette reference, and NAM evaluation)  
**Project folder:** `C:\Pedal PI`

## Purpose

Create a standalone guitar effects instrument. The device should start directly into a usable patch, show readable modules connected by virtual cables on a 5-inch touchscreen, and support assignable knobs and stomp switches.

**The touchscreen interface is a new LVGL application, designed and written for this instrument — agreed.** It provides a simple drag-and-drop layout: each module is a square showing its name, tapping a module opens its internal parameters (drive, gain, volume, time, etc.), and modules are patched with virtual cables in series or parallel. Beebo's published interface is a reference for concepts only; no Beebo code, QML, or assets are used. The interface runs natively at 800 × 480 without a desktop environment for fast cold boot and low overhead.

The audio side reuses Poly's published engine (its Ingen fork) and LV2 modules, including the Mutable Instruments-derived modules, extended with compatible LV2 effects used by MODEP.

The existing MODEP interface is unsuitable for the user's 800 × 480 display. Opening MODEP automatically or hiding its desktop does not meet the interface requirement.

## Documentation

| File | Contents |
| --- | --- |
| [Requirements](docs/01-requirements.md) | Agreed behaviour, hardware baseline, scope, and success criteria |
| [Architecture](docs/02-architecture.md) | Audio engine, LVGL touchscreen application, controls, storage, and startup |
| [Touchscreen and physical controls](docs/03-interface-and-controls.md) | Drag-and-drop 800 × 480 interaction design, series/parallel routing, and control assignments |
| [Effects and plugin compatibility](docs/04-effects-and-compatibility.md) | Mutable Instruments-derived modules, MODEP effects, and compatibility checks |
| [Implementation roadmap](docs/05-implementation-stages.md) | Phase overview, development order, dependencies, and evidence required to advance |
| [Validation](docs/06-validation.md) | Audio, interface, hardware, startup, and persistence checks |
| [Source audit and open decisions](docs/07-source-audit-and-decisions.md) | Inspected source versions, Beebo reference findings, decisions log, and unanswered questions |
| [Reference source collection](docs/08-reference-sources.md) | Download scope, repository versions, declared submodules, and Windows storage details |
| [Casette and NAM evaluation](docs/09-casette-and-nam.md) | Pinned Casette findings, lessons for this instrument, NAM feasibility, and proposed benchmark plan |
| [Detailed technical implementation plan](docs/10-technical-implementation-plan.md) | Phases 0–11 with numbered tasks, PC/Pi work split, deliverables, exit checks, and requirements coverage |

## Retrieve the project and reference sources

Project documentation is tracked in the main repository. The ordinary upstream references and recovered Autowaf reference are pinned Git submodules. The complete kernel source archive is tracked with Git LFS; its local bare Git object cache and incomplete `.part` download are not published.

With Git and Git LFS installed:

```powershell
git clone https://github.com/ABustos2606/Pedal-PI.git
cd Pedal-PI
git lfs install --local
git lfs pull
git submodule update --init
git -C reference/upstream/lenticular_lv2 submodule update --init --recursive
git -C reference/upstream/neural-amp-modeler-lv2 submodule update --init --recursive
git -C reference/upstream/tuna.lv2 submodule update --init --recursive
```

Initialize the top-level references first, then the three repositories with declared dependencies as shown. PolyMclkIn's unmapped `waflib` pointer is supplied separately at `reference/dependencies/autowaf`; its upstream repository is unchanged. See [the reference index](reference/README.md) and [manifest](reference/manifest.json) for exact revisions and Windows checkout limitations. This retrieves reference material, not a working instrument or a build environment.

## Reading the status labels

- **Agreed:** directly supported by the user's instructions.
- **Observed:** found in inspected upstream source; not evidence that it runs on this Pi.
- **Proposed:** a recommended design or test target awaiting review.
- **Unverified:** requires hardware inspection, a build, or a runtime test.

## Current facts

- Raspberry Pi 5 with 2 GB RAM.
- Existing Patchbox OS installation from an experiment last year; release and architecture are unknown.
- 5-inch HDMI touchscreen advertised as 800 × 480. The user confirms touch capability; the touch controller, calibration, and multitouch support still need identifying.
- The user has knobs; their type, number, and connection are not yet identified. Stomp-switch hardware remains unspecified.
- The audio interface is flexible and still to be selected; an inexpensive USB interface is acceptable to the user. Its suitability for guitar input and low-latency Linux audio must be checked when selecting it.
- Public Beebo-related source repositories are stored under `reference/` in the project. Their source code has not been compiled, installed, or tested on the Pi.
- Casette is also downloaded under `reference/upstream/Casette` at commit `75ded6663e2d758bfe8a290952c672b2e8c01697`. It is a reference for LVGL, JACK, NAM, and preset handling; its fixed effects chain does not replace our modular engine.
- NAM is a candidate for evaluation on the Pi 5, 2 GB. An optimized runtime is promising, but CPU headroom, memory use, simultaneous model count, and operation alongside Clouds remain unverified. The older downloaded Poly NAM fork and current NeuralAudio-based LV2 implementations need comparison.
- Beebo's interface repository (`digit_ui`) is **reference material only**. No code, QML, icons, fonts, or other assets from it will be copied into the new interface.

These documents and reference sources are the project baseline, not a working instrument or an installation package. No project-specific application code, build scripts, installers, wiring changes, or system services have been created. Upstream repositories may contain their own build and installation files; they have not been executed. Implementation requires a separate explicit instruction.

## Next step after documentation review

1. Start the shared LVGL application and 800 × 480 simulator on the Windows PC with a simulated engine; the Pi is not needed for initial UI development.
2. Identify the Pi OS, audio interface and touch controller, preserve the existing installation, and establish a minimal JACK/Ingen audio path early.
3. Integrate the real engine and native touch interface, then implement coherent patch/state persistence and the Clouds milestone with physical controls.
4. Evaluate NAM and expand the tested catalogue before dedicated startup and complete validation. Follow the [detailed phase plan](docs/10-technical-implementation-plan.md); no implementation phase has been executed.
