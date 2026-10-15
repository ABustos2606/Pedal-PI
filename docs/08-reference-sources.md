# Downloaded reference sources

The project includes the public Poly Effects Beebo/Digit-related source collection and an additional Casette snapshot under `reference/`. The Poly collection was downloaded on 12 October 2026; Casette was added on 13 October 2026.

## Role in the project

- **Engine and modules** (`ingen`, `lenticular_lv2`, `PolyLV2`, `poly_filters`, and other plugin repositories): candidates to build and run on the Pi.
- **Beebo interface** (`digit_ui`, archived `digit_carla`): design and integration reference only. The new interface is written separately and copies no code or assets from them.
- **Hardware-specific sources** (kernel, clock library): background reference; not intended for the Pi.
- **Casette**: native LVGL, JACK, NeuralAudio/NAM, cabinet IR, and preset reference. Its fixed effects chain, hardware setup, and one-process design are not replacements for our modular engine and separate interface.

## Contents

Twenty Poly repositories cover the interface, audio engine, Mutable Instruments-derived modules, modular utilities, filters, convolution, MIDI/CV controls, amp modeling, looper, tuner, metadata and capture tools, manual, archived Digit application, and hardware-specific sources.

Casette is the **21st repository**. Its complete published snapshot is stored in `reference/upstream/Casette` at `75ded6663e2d758bfe8a290952c672b2e8c01697` (upstream commit dated 21 July 2026). All 93 tracked files are present, including source, README, build files, models, and IRs; the checkout is clean.

Fifteen declared recursive submodules are initialized at their recorded revisions. An additional unmapped `waflib` pointer was resolved to its exact Autowaf source revision and stored separately. Upstream source code is unchanged, and no build or installer has run.

See [the reference index](../reference/README.md) for the inventory and [the source manifest](../reference/manifest.json) for exact commits, origins, dependency revisions, and kernel archive checksum.

## Storage and provenance

Ordinary repositories are Git checkouts in `C:\Peepo\reference\upstream`. The hardware-specific kernel is a complete source archive plus a Git reference repository because Windows rejects some source filenames. The archive was checked against all 63,993 tracked source paths at its recorded revision.

The collection is a set of default-branch source snapshots, not every historical release and not a complete firmware image. Other third-party effects referenced by the Beebo UI and Linux distribution libraries still require separate inventory work. Only Poly's unrelated inventory/manufacturing integration repository was excluded from its public repository list.

The original source audit records the initial five inspected repositories. The reference manifest records the broader download. Neither establishes runtime compatibility on the Pi.

## Casette coverage and dependency limits

Casette's `deps/` directory is ignored and absent from the checkout. No Git submodule pointers or `.gitmodules` file are published at this revision. Its CMake configuration references NeuralAudio, LVGL/lv_port_linux, FFTConvolver, Eigen, JSON, plog, and math_approx; system libraries include JACK, libgpiod, libsndfile, and aubio. Their exact revisions are not supplied by the repository, so a later build requires a separately pinned dependency manifest.

The checkout is unchanged and its build/deployment scripts have not run. Current upstream NeuralAudio and NAM LV2 documentation are online comparison references only; those projects were not added as separate checkouts in this download. See [Casette and NAM evaluation](09-casette-and-nam.md) for inspection findings and the distinction between source evidence and proposed tests.

## Git publication

The project repository tracks documentation directly, the 20 ordinary upstream checkouts plus recovered Autowaf as 21 pinned top-level submodules, and the complete kernel archive through Git LFS. `.gitmodules` and `reference/manifest.json` preserve origins and revisions. The local kernel bare Git cache and incomplete `.part` download are ignored.

Use [the main README's retrieval procedure](../README.md#retrieve-the-project-and-reference-sources) to fetch top-level references and their mapped dependencies. PolyMclkIn's missing `waflib` mapping remains an upstream limitation; its recovered source is supplied separately. Downloading these references does not build or install them.

## Hardware clarification

- The user confirms a capacitive touchscreen with five touch points, touch and power over USB and video over HDMI; the controller identity remains unverified.
- The user owns knobs; type, quantity, and connection remain to be identified.
- Audio hardware is flexible. An inexpensive USB interface is acceptable, but selection still needs an appropriate guitar input and verified Linux audio performance.

Downloading reference material is authorized. Building upstream sources, writing the new interface, or configuring the Pi still requires a separate implementation instruction.
