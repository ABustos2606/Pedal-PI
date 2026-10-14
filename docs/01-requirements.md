# Requirements

## Product goal — agreed

A dedicated modular effects instrument built around the user's Raspberry Pi. Its everyday operation should feel like a hardware musical instrument: power it on, recover the saved patch, and control effects through the screen, knobs, and stomp switches.

The software direction is a **new touchscreen interface designed for this instrument**, inspired by Beebo's modular workflow, running on Poly's published audio engine and LV2 modules. The desired collection includes Poly's Mutable Instruments-derived modules and compatible LV2 effects available in MODEP.

## Hardware baseline

| Component | Known state | Required follow-up |
| --- | --- | --- |
| Computer | Raspberry Pi 5, 2 GB RAM, official 27 W supply (user-confirmed) | Confirm cooling and storage |
| Operating system | Patchbox OS | Identify release, kernel, userland architecture, and audio configuration |
| Display | 5-inch HDMI touchscreen, advertised 800 × 480; capacitive with five touch points, touch and power over USB, video over HDMI (user-confirmed 13 October 2026) | Confirm actual mode, controller identity and calibration |
| Audio interface | Flexible; not yet selected; inexpensive USB is acceptable | Select input/output channels, guitar input impedance, Linux support, and usable latency |
| Knobs | User owns knobs; details unspecified | Identify encoders/potentiometers, quantity, and connection |
| Stomp switches | Desired; hardware unspecified | Decide quantity, momentary/latching behaviour, and connection |

The display description comes from the user's purchase link, which could not be independently retrieved. The user confirms that the panel is capacitive and that touch and power use USB while video uses HDMI; the GPIO header is therefore free for controls. It reports five touch points and already works with Zynthian and with Patchbox OS running MODEP. Controller identity and calibration remain unverified.

## Functional requirements

| ID | Requirement | Acceptance evidence |
| --- | --- | --- |
| R01 | Start into the instrument automatically | Normal power-on needs no login, desktop interaction, browser launch, or manual application launch |
| R02 | Restore a saved patch | Routing, parameter values, and physical-control assignments recover correctly after restart |
| R03 | Display modules and virtual cables | User can drag and drop square modules with visible names, connect and disconnect virtual cables in series or parallel on the 800 × 480 screen |
| R04 | Make parameters readable and operable | Tapping a module opens its parameter page (drive, gain, volume, time, etc.) with readable, finger-operable controls at native 800 × 480 |
| R05 | Process external guitar audio | Selected input passes through a representative effects chain to the selected output reliably |
| R06 | Include Mutable Instruments-derived processing | Clouds is an early milestone; the remaining published module families enter the tested catalogue progressively |
| R07 | Reuse compatible MODEP LV2 effects | Each supported effect has a verified build, host compatibility, and a catalogue entry in the new interface |
| R08 | Assign physical knobs | User can map knobs to effect parameters, see the assignment, and retain it with a patch |
| R09 | Assign stomp switches | User can configure musical actions such as effect toggles, freeze, tap tempo, and patch changes where supported |
| R10 | Operate without internet | Playing and editing an installed patch do not depend on an internet connection |
| R11 | Save and manage patches | Save, rename, select, and delete patches through an interface suited to the screen |
| R12 | Use the project's own LVGL interface | The touchscreen application is built from scratch using LVGL, natively at 800 × 480 without a desktop session; it contains no copied Beebo code, QML, icons, fonts, or assets |
| R13 | Keep audio independent of the interface | Restarting or crashing the interface does not stop audio; on restart, the interface reconnects and shows the engine's current patch |

## Performance requirements

Fast cold boot is an agreed objective. A numeric startup commitment is **not yet agreed or demonstrated**. Record separately:

1. Power-on to usable audio with the intended patch restored.
2. Power-on to a visible and responsive patch editor.

A splash screen or an active Linux service does not establish musical readiness. Measure the entire power-on sequence, including bootloader, audio-device initialization, engine startup, and patch restoration.

Live-playing latency, CPU capacity, memory use, and thermal stability must be established on the actual Pi and audio interface. The number of available modules is not the number that can run simultaneously. Large reverbs, granular processing, and neural models may require different resource budgets.

## NAM evaluation — proposed

The user has requested investigation of NAM feasibility and authorized retaining Casette as another reference. NAM is a candidate module, not yet a tested capability or a commitment to a particular runtime or model count. Start with one mono amp model and a cabinet IR where required; evaluate its resource cost independently before adding Clouds and conventional effects.

The working expectation is that 2 GB RAM can accommodate a modest NAM patch with a lightweight interface. This is an engineering inference, not a measured capacity guarantee. The entire system must retain memory headroom, avoid sustained swapping, meet audio deadlines, and remain stable under screen interaction and model changes. No fixed CPU percentage, interface memory ceiling, or maximum simultaneous model count has been established.

Model file selection, input calibration, supported quality settings, and saved model/IR references are part of the proposed NAM evaluation. Automatic quality reduction is an open product choice and must not be treated as agreed behaviour. See [Casette and NAM evaluation](09-casette-and-nam.md).

## First working milestone — proposed

One self-contained patch with guitar input, Clouds-derived granular processing, a conventional effect, and audio output, built and edited in the new interface. The patch should launch automatically, be editable on the touchscreen, respond to one knob and one stomp switch, and survive a save/restart cycle.

This proves the architecture. It does not fulfil the full desired module collection.

## Scope boundaries

- Beebo is a reference for concepts and workflow. The new interface does not need to reproduce Beebo's screens, layout, or behaviour; Beebo interface parity is not a goal.
- Casette is a downloaded technical reference. Its DietPi installation, HiFiBerry audio HAT, DSI screen, GPIO assignments, and fixed effects chain do not select our hardware or software baseline.
- The user does not want MODEP's pedal-image interface as the main screen.
- A phone or computer may help with setup, but routine playing and patching must work on the instrument itself.
- Existing MODEP pedalboards are not automatically Ingen patches for the new interface. Patch conversion is a separate possible task.
- Beebo firmware features, four audio inputs/outputs, and hardware relay bypass depend on additional hardware and software work; they are not implied by owning a Pi.
- A bare-metal rewrite, commercial distribution, and purchasing additional hardware are outside the current documentation task.

## Change control

The current authorization covers documentation and downloading unchanged upstream code for reference. Proposed control counts, operating-system changes, pin assignments, boot optimizations, interface code, and installation procedures become implementation work only after the user explicitly requests implementation.
