# Touchscreen and physical controls

## Agreed interaction

The interface is a **new, simple drag-and-drop design for this instrument — agreed**, built with LVGL and laid out natively for the 800 × 480 display.

- Each module is a **square showing its name**.
- Squares are **dragged** to arrange them on the screen.
- **Entering** a module opens a page with its internal parameters (for example drive, gain, volume, time).
- Modules are **patched in series or in parallel** with virtual cables.

Physical knobs and stomp switches control assigned parameters.

## Beebo as reference

Beebo's interface (1280 × 720, with many fixed coordinates) shows that a modular patch bay works on a pedal touchscreen. Concepts carried over: modules connected by visible cables, a separate parameter page per module, a categorized module browser, and assignments linking physical controls to parameters.

Screens, layouts, styling, icons, and code are our own. The new interface is deliberately simpler than Beebo's.

The user's five reference photos also illustrate why parameter pages need not all have the same layout: the tuner emphasizes a note and cents, the oscillator emphasizes model selection, and the granular page groups continuous controls. Keep a consistent navigation frame while adapting the content to the task. The photos' branching graphs reinforce visible split/merge routing; their cables alone do not establish engine summing behavior.

## Casette as an additional reference

Casette's published source uses LVGL framebuffer output, evdev touch, tabs, level meters, a tuner, and a CPU/temperature display for an advertised 800 × 480 screen. These are useful implementation observations, not proof of usability on our 5-inch HDMI display. Retain our square-module canvas and module pages with a generic fallback. See [the source audit](09-casette-and-nam.md).

## Visual identity — open

The basic form is agreed: named squares and cables. Colour palette, typography, square and cable styling, and the instrument's name/branding remain open. Design for legibility on a small screen viewed from standing height on a pedalboard, and review on the physical display.

## Patch view (main screen)

```
┌──────────────────────────────────────────────────────────────┐
│ Patch name ●            audio ok        [+]  [patches]  [⚙]  │  header
├──────────────────────────────────────────────────────────────┤
│                         ┌───────┐                            │
│  ┌──────┐   ┌───────┐ ┌─┤ DELAY ├─┐   ┌───────┐   ┌───────┐  │
│  │  IN  ├───┤ DRIVE ├─┤ └───────┘ ├───┤  EQ   ├───┤  OUT  │  │
│  └──────┘   └───────┘ │ ┌───────┐ │   └───────┘   └───────┘  │
│                       └─┤CLOUDS ├─┘                          │
│                         └───────┘                            │
└──────────────────────────────────────────────────────────────┘
          series ─────────── parallel ─────────── series
```

*Illustrative layout, not a final design.*

- **IN** and **OUT** are fixed squares representing the audio interface's input and output. They cannot be deleted.
- Each square has an **input jack on its left edge** and an **output jack on its right edge**. Sound sources without an audio input show only an output jack.
- Squares **snap to a grid** so patches stay tidy and touch targets stay predictable.
- If a patch does not fit, dragging empty space pans the view. Zoom is not required for the first version.
- The header shows the patch name, an unsaved-changes marker, audio readiness, and buttons to add a module, open the patch manager, and open settings.

### Gestures — proposed

All gestures work with a single finger. Multitouch is not required.

| Gesture | Action |
| --- | --- |
| Tap a square | Enter the module's parameter page |
| Press and drag a square | Move it |
| Drag from an output jack to a square's input jack | Create a cable |
| Tap a cable | Select it; a delete button appears |
| Long-press a square | Menu: bypass, duplicate, delete, assign to switch |
| Tap **+** | Open the module browser; the new square appears in a free grid position |

A drag starts only after a small movement threshold, so a tap is never mistaken for a move.

### Series and parallel routing

- **Series:** one module's output connects to the next module's input.
- **Parallel (split):** one output may connect to several inputs.
- **Parallel (merge):** several outputs may connect to one input; the engine mixes them.

Every audio port has its own connector, and **every cable is one channel from one connector to one connector** (agreed 13 October 2026, replacing the earlier single logical stereo cable). A stereo module shows two connectors on each side, left above right; they are patched separately. Any output may go to any input:

| Wanted | How it is patched |
| --- | --- |
| Stereo chain | Two cables: left to left, right to right |
| Left and right through different effects | One cable from each output to a different module |
| Mono source into a stereo effect | One cable to one input, or two cables from the same output to both |
| Stereo into a mono effect | One cable from the side wanted, or both into the one input (the engine sums them) |
| Swap left and right | Cross the two cables |

**Measured:** Ingen sums several cables into one input at unity, with no compensation ([evidence](14-ingen-engine-evidence.md)); two equal signals are +6 dB. Whether to attenuate automatically or offer a mixer module is still to be decided.

The desktop simulator's **Patches → Parallel demo** starts with IN → Drive, splits into Clouds and Delay, and reconnects both branches at OUT. Branches use ordinary editable cables, one per channel; loading the demo uses the unsaved-replacement guard. This demonstrates graph editing, with no audio processing.

Feedback loops (a cable that leads back to an earlier module) are refused in the first version.

The first version routes **audio only**. Modulation, CV, and MIDI connections between modules are deferred (decision D16). A visual cable must always correspond to a valid engine connection; a refused connection shows a short reason.

## Module page

Opened by tapping a square. It fills the screen with the module's parameters and a **back** button to the patch view.

Module-specific pages are supported in the shared LVGL application. Clouds opens **Texture** (position, size, density, pitch) and **Mix / hold** (mix, freeze); Delay opens **Character** mode buttons with timing, feedback and mix controls. Both retain software bypass and Actions, and **All** opens the generic parameter editor. All layouts use the same parameter symbols, range conversion and engine acknowledgment path.

Select a specialized layout by stable module URI and compatible parameter schema. Renaming a module must not change its layout. Unrecognized or changed schemas use the generic editor so parameters remain accessible. The initial custom layouts target simulator fixtures; real LV2 modules require verified metadata before a specialized view is registered. Tuner, oscillator and other dedicated layouts can be added when their module contracts exist.

- Large, labelled controls with current value and unit, about 4–6 per page; more parameters continue on further pages.
- Control type follows the LV2 parameter type: slider or arc for continuous values (drive, gain, volume, time), switch for on/off values (freeze, bypass), and a selector for lists of modes.
- Logarithmic parameters (frequency, time) move logarithmically.
- A bypass toggle and the module's name in the header.
- A way to assign a parameter to a physical knob (see below).

Every module uses this same page style, generated from plugin metadata and the project catalogue overlay. Custom pages are a possible later addition, not a requirement.

### NAM and cabinet pages — proposed

A tested NAM module needs a local `.nam` file selector, current model name, input gain/calibration, output level, and supported quality control. An amp-only capture normally needs a separate cabinet IR module; a capture that already includes a cabinet may not. Make asset and loading errors readable and retain the last usable state or show an explicit failed state according to verified engine behaviour.

Model and IR paths are not ordinary numeric sliders. The engine client must send the selected plugin's supported file-path messages and read back the accepted state. Preserve asset references and quality with the patch. Show loading progress without blocking drawing or audio, and prevent unsupported quality controls from suggesting that every model can change size.

Expose DSP load/xrun status and a settings view for CPU, memory, and temperature using verified engine/system measurements. Do not label a process CPU estimate as total system load or spare audio capacity. Automatic quality changes are not agreed; any later policy must make the change visible and preserve user intent.

## Module browser

A full-screen categorized list: drive/distortion, delay, reverb, modulation, filter/EQ, granular/texture, dynamics, utilities, and sound sources. Guitar effects come first. Only plugins that are installed and passed compatibility checks appear as available; others are hidden or marked unavailable.

## Patch manager

Full-screen list of saved patches: load, save, save as, rename, delete, and set as the startup patch. Text entry uses an on-screen keyboard sized for fingers.

## Initial sizing targets — proposed

- Module squares: about 96 × 96 pixels, so roughly seven columns by four rows fit below the header.
- Jacks: drawn small but with a touch area of at least 40 × 40 pixels.
- Normal labels: approximately 18–22 pixels; module names on squares: up to two lines, abbreviated if needed.
- Primary interactive controls: approximately 44–48 pixels minimum in each usable dimension.
- Cables: at least 3 pixels wide, with a wider invisible touch area.

These are starting values, not proven acceptance results. Test them on the 5-inch screen and adjust based on readability and touch errors. Do not claim usability from a desktop simulator or screenshot.

## Physical controls

No number or wiring has been selected by the user. Four push encoders and three stomp switches are an earlier **proposal**, not an agreed bill of materials. Beebo's hardware exposes two encoders and three switches with combined gestures; our control model is designed independently around whatever hardware is chosen.

| Control type | Proposed behaviour | Design question |
| --- | --- | --- |
| Encoder | Adjust selected or pinned parameter | Relative increments, acceleration, fine adjustment, and page behaviour |
| Potentiometer | Set an absolute parameter value | Pickup/soft takeover to avoid jumps after patch changes |
| Encoder push | Select, assign, or open a parameter page | Must not conflict with value adjustment |
| Stomp switch | Musical action chosen in the patch | Toggle, momentary, tap, and hold behaviour |
| Expression pedal | Optional continuous control | Hardware input and range calibration are not yet scoped |

Parameter mappings must preserve ranges and types: frequency may benefit from logarithmic movement, mode selection requires discrete values, and freeze requires a defined binary state. Where smoothing is needed, apply it without delaying urgent switch actions.

A parameter can be pinned to a knob, so changing the displayed module does not unexpectedly change a knob's role. The interface shows each knob's current assignment.

## Suggested stomp actions

- Toggle one effect or a group of effects.
- Hold or toggle Clouds freeze.
- Tap tempo for modules that accept a usable tempo reference.
- Move to the next/previous patch.
- Trigger supported looper actions once the looper is validated.

Distinguish software effect bypass from physical guitar bypass. A direct unprocessed signal path while Linux is unavailable requires suitable audio/bypass hardware; an on-screen bypass button alone cannot provide it.

## Assignment persistence

Save each mapping with its patch: control identity, target module instance, parameter symbol or action, range, mode, and any inversion. Detect mappings whose module or parameter no longer exists and show them for repair rather than silently controlling a different target.
