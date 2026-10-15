# Instrument reference sources

Poly sources downloaded on 12 October 2026; Casette added on 13 October 2026. Source adaptation, compilation, and installation have not started.

## Scope

This collection contains **20 public Poly Effects repositories** related to Beebo/Digit and supporting software/hardware, **15 initialized recursive submodules**, and **one separately retrieved build-library reference**. Only Poly's unrelated `poly_erp` inventory/manufacturing repository was excluded from its 21-repository public list at download time.

It also contains **one Casette repository**, for **21 repositories total**. Casette is a technical reference for LVGL, JACK, NAM, cabinet IRs, and presets. Our separate LVGL interface and modular Ingen engine remain the project direction. See [Casette and NAM evaluation](../docs/09-casette-and-nam.md).

These are unchanged source snapshots at recorded commits. The downloads are shallow: the selected default-branch revision is present, while complete historical branches, tags, and releases are not included. This is not a complete firmware image or a validated compatible build set.

Every upstream snapshot, dependency and archive retains its original copyright and licensing terms. The root Peepo GPL-3.0-or-later license covers original project files, not these third-party sources or their assets. See [third-party notices](../THIRD_PARTY_NOTICES.md) for inspected license evidence; keep each upstream license and file notice when adapting or packaging a component.

## Layout

- `upstream/`: 20 ordinary Git checkouts (19 Poly and Casette) and the kernel's `linux.git` reference repository.
- `archives/`: the complete kernel source archive.
- `dependencies/autowaf/`: the exact build-library revision referenced by PolyMclkIn's unmapped `waflib` pointer.
- `manifest.json`: full URLs, revisions, dependency pins, source status, and archive checksum.

## Repositories

| Repository | Reference purpose | Commit prefix |
| --- | --- | --- |
| [avwlv2](https://github.com/polyeffects/avwlv2/tree/bfb2e1f1851bdc6192e0401789a5d62045156d20) | Modular audio and CV plugins | `bfb2e1f1851b` |
| [convoLV2](https://github.com/polyeffects/convoLV2/tree/8ddddc69e3a0a4dd1e0fa220144b8c343252409d) | Convolution plugin source | `8ddddc69e3a0` |
| [cv_to_cc.lv2](https://github.com/polyeffects/cv_to_cc.lv2/tree/23152a8bf6dcdf2496df0d696dfd4c1a61f67e54) | CV-to-MIDI CC module | `23152a8bf6dc` |
| [digit_carla](https://github.com/polyeffects/digit_carla/tree/75609d7dd365628fca044f64a7141686853e7945) | Archived Digit application, retained as historical reference | `75609d7dd365` |
| [digit_ui](https://github.com/polyeffects/digit_ui/tree/feb4df451067850770ddd13af9f4e2828234dabc) | Beebo/Hector touchscreen interface and module definitions — design reference only; not adapted | `feb4df451067` |
| [ingen](https://github.com/polyeffects/ingen/tree/8dae740425f0f1468198e0cdf1e1c61b3fc6ec5e) | Poly modular LV2 audio engine | `8dae740425f0` |
| [lenticular_lv2](https://github.com/polyeffects/lenticular_lv2/tree/141fdc89e250987bae421664132ddaedae735107) | Mutable Instruments / Parasites-derived DSP modules | `141fdc89e250` |
| [linux](https://github.com/polyeffects/linux/tree/6602da91cde9a8834361793b70cee90e3a64d4a5) | Hardware-specific kernel reference; not the intended Pi kernel | `6602da91cde9` |
| [lv2_to_dict](https://github.com/polyeffects/lv2_to_dict/tree/d062ea0151ef7a8877361bf8c2ce0c5078e8ccad) | LV2 metadata conversion utility | `d062ea0151ef` |
| [manual](https://github.com/polyeffects/manual/tree/6d9e199333c2162b80dfa922ef4853c0119bbd80) | Published manual source | `6d9e199333c2` |
| [neural-amp-modeler-lv2](https://github.com/polyeffects/neural-amp-modeler-lv2/tree/a3a6520ea519513a6ca1e82d350d47e3c7891732) | Poly Neural Amp Modeler plugin fork | `a3a6520ea519` |
| [poly_capture](https://github.com/polyeffects/poly_capture/tree/f1ad646dcdfe906d51e27d45e78125bf7565ed8b) | Amp/IR capture utilities | `f1ad646dcdfe` |
| [poly_filters](https://github.com/polyeffects/poly_filters/tree/ebf8fd1cea20e5d167f0c04ded12da13bd70829f) | Poly filter algorithms | `ebf8fd1cea20` |
| [polyconvo.lv2](https://github.com/polyeffects/polyconvo.lv2/tree/fe8edd2da2e348ecc383c93c5d4e7961e7d8dc5b) | Poly convolution fork with patch-message support | `fe8edd2da2e3` |
| [PolyLV2](https://github.com/polyeffects/PolyLV2/tree/6f3ae80f6b4eccb8daebfc2b641fde1d48224250) | Modular utilities, control modules, and effects | `6f3ae80f6b4e` |
| [PolyMclkIn.lv2](https://github.com/polyeffects/PolyMclkIn.lv2/tree/64acb88d6c574b4efc743b770d1d22444391ba02) | MIDI clock input module | `64acb88d6c57` |
| [serd](https://github.com/polyeffects/serd/tree/ebe0b8697bca69abb7e7be1c731da4d54b10e462) | Poly RDF parsing library fork | `ebe0b8697bca` |
| [sooperlooper](https://github.com/polyeffects/sooperlooper/tree/0cf3001ed295011714c059fa91d987835811a0f1) | Looper source | `0cf3001ed295` |
| [tuna.lv2](https://github.com/polyeffects/tuna.lv2/tree/cfdfb6aeb065b278f58b6c6d4c23b5d82ae34279) | Instrument tuner source | `cfdfb6aeb065` |
| [xmos_lib_si5351](https://github.com/polyeffects/xmos_lib_si5351/tree/9c333896a18b37575e73d68a28ffa0be3a65ec8a) | Hardware clock-support reference | `9c333896a18b` |
| [Casette](https://github.com/pawelKapl/Casette/tree/75ded6663e2d758bfe8a290952c672b2e8c01697) | Additional LVGL/JACK/NeuralAudio reference; fixed chain, not our engine | `75ded6663e2d` |

## Git publication and retrieval

The Peepo repository records the 20 ordinary upstream checkouts and the separate Autowaf checkout as **21 pinned top-level Git submodules**. Their own declared dependencies retain their upstream commit pointers. `.gitmodules` records the origin for each top-level checkout, and `manifest.json` records source provenance and scope.

The complete 166,786,644-byte kernel source archive is tracked using Git LFS. The local `upstream/linux.git` bare repository is an ignored object cache; the archive retains the published complete source. The unfinished archive `.part` file is ignored. These local caches and scratch files are not source changes.

Follow [the main README retrieval steps](../README.md#retrieve-the-project-and-reference-sources): initialize top-level submodules without recursive traversal, then the declared dependencies in `lenticular_lv2`, `neural-amp-modeler-lv2`, and `tuna.lv2`. Avoid a blanket recursive update through PolyMclkIn's unmapped `waflib` pointer; the recovered source is already a separate top-level submodule at `dependencies/autowaf`. This preserves the unchanged upstream repositories.

## Dependency verification

Every initialized dependency was checked against its parent's recorded commit pointer. All 19 ordinary upstream checkouts and the 16 dependency references are clean. PolyMclkIn's source has a `waflib` Git pointer but no `.gitmodules` mapping; its matching source was recovered from `https://gitlab.com/drobilla/autowaf.git` at `c87cc53c643c60867665b333d5999fe07196d543` and retained separately without changing the parent repository.

Casette's additional checkout was verified clean at `75ded6663e2d758bfe8a290952c672b2e8c01697`, with all 93 tracked files present. Its ignored `deps/` folder is not published, and it declares no tracked Git submodules. No dependencies were inferred, downloaded, or installed for it. Its bundled models/IRs are retained as upstream reference assets, not an adopted production library. The exact NeuralAudio/LVGL/FFTConvolver and other dependency versions required by this snapshot remain to be established.

## Kernel on Windows

The `orange-pi-5.1` kernel source contains names such as `aux.c` that Windows cannot check out normally. The complete source is therefore preserved in `upstream/linux.git` and a commit-pinned `.tar.gz` archive, rather than a misleading incomplete checkout.

The archive's 63,993 source paths were compared with the Git tree and matched exactly. Its SHA-256 is recorded in the manifest. This kernel is hardware-specific reference material; it has not been selected for or installed on the Raspberry Pi 5.

## Coverage limits

Some modules in Beebo's catalogue come from additional third-party projects. Their presence in `module_info.py` does not mean that all their source or binaries exist in these Poly repositories. Sources outside this collection, distribution-provided system libraries, audio assets, and firmware packaging remain separate inventory/build tasks.

Git may represent symbolic links as text on Windows. For a later Linux build, reconstruct the appropriate checkouts from the recorded origins and commit pins. None of the reference repositories' build, installer, firmware, or maintenance commands has been executed.

Keep project implementation work — the new interface, build scripts, and configuration — in a separate future directory. Do not treat the reference repositories as our implementation.
