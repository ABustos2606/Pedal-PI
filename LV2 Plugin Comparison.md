# LV2 plugins: Patchstorage and Poly Effects repositories

Verified **13 October 2026**. Every Patchstorage LV2-platform entry is included below, together with every plugin identity declared by the plugin repositories in Poly Effects’ current public repository collection and every Beebo plugin mapping.

## Scope and how to read the lists

“Poly repos” means the public [polyeffects GitHub organization](https://github.com/polyeffects). Its current repository list has 21 repositories. The plugin inventory uses their default-branch commits, verified against the local source collection. The full repository audit appears at the end.

Patchstorage has **385 entries advertising 450 distinct plugin IDs**. Eighteen entries explicitly advertise multiple plugins. Inspection of one downloadable build for each of all 385 entries found **102 additional plugin IDs**, for **552 distinct IDs inventoried** across the published catalogue and inspected downloads. The 552 count is an inventory count: it includes website-advertised IDs that were not present in the inspected architecture, and does not claim that every binary archive for every architecture was inspected. [Platform](https://patchstorage.com/platform/lv2-plugins/), [public API, page 1](https://patchstorage.com/api/beta/patches/?platforms=8046&per_page=100&page=1).

The Poly source list contains **123 distinct LV2 IDs in 11 repositories**, including historical Digit plugins and three tuner identities produced by two tuner variants and a build option. Separately, [Beebo’s module map](https://github.com/polyeffects/digit_ui/blob/feb4df451067850770ddd13af9f4e2828234dabc/module_info.py) contains **157 mappings to 153 distinct IDs**. Of those IDs, 69 have matching plugin metadata in Poly’s source repositories and 84 are references without a matching implementation in this public collection. The combined Poly inventory is **207 distinct IDs**.

Comparison now records exact URI matches, verified AMS source-family variants, verified basic-operation counterparts, and partial/related functions separately. Different URIs do not establish that a plugin is unavailable. AMS ports, CV scaling and oscillator versions can differ even when source lineage is shared. Exact Beebo mapping means an ID appears in the UI map; related Beebo mappings are shown separately. A public implementation can exist without a Beebo UI mapping.

Build key: **Pi64** = `rpi-aarch64`; **PB32** = `patchbox-os-arm32`; **x64** = `linux-amd64`. **Ready** and **WIP** reproduce Patchstorage’s entry state. **B** identifies an additional plugin found in a downloaded bundle but absent from that entry’s published ID list; its build column records only the inspected target.

## Comparison summary

| Inventory / comparison | Count |
| --- | --- |
| Patchstorage entries | 385 |
| Patchstorage explicitly advertised IDs | 450 |
| Additional IDs in inspected downloads | 102 |
| Combined Patchstorage inventory | 552 |
| Poly implementation IDs | 123 |
| Beebo mappings / distinct IDs | 157 / 153 |
| Combined Poly source + Beebo inventory | 207 |
| Exact URI matches across the combined inventories | 47 |
| Patchstorage IDs without an exact Poly URI match | 505 |
| Poly IDs without an exact Patchstorage URI match | 160 |
| Exact URI matches between Poly implementations and Patchstorage | 2 |

Against the website’s advertised 450 IDs alone, there are **46 matches**, **404 Patchstorage IDs without an exact Poly URI match**, and **161 Poly IDs without an exact Patchstorage URI match**. Download inspection adds 1 further exact match, giving the expanded comparison above.

The 2 matches with published Poly implementation source are **x42 Instrument Tuner (MOD build)**, **Neural Amp Modeler**. The other 45 shared IDs are third-party plugins referenced by the Beebo interface. These totals describe identical IDs. The corrected AMS/CV comparison below adds source-family and functional counterparts without treating them as identical implementations.

## Corrected comparison: AMS and CV counterparts

The original URI-only comparison missed the AMS relationship and basic CV equivalents. The MOD AMS bundle and Poly’s AVW collection derive from the same AMS code family. The review below verifies **9 additional Poly AMS IDs** against the bundle’s **6 plugin IDs**, plus **3 additional Poly IDs** against **CV ABS** and **Control to CV**. These are variants/counterparts, not identical binaries or guaranteed replacements.

| Comparison including reviewed AMS/CV counterparts | Count |
| --- | --- |
| Exact URI matches | 47 |
| Additional Poly IDs with verified AMS/CV counterparts | 12 |
| Additional Patchstorage IDs with verified Poly counterparts | 8 |
| Poly IDs with an exact match or verified reviewed counterpart | 59 |
| Patchstorage IDs with an exact match or verified reviewed counterpart | 55 |
| Poly implementation IDs with an exact match or verified reviewed counterpart | 14 |

This is a many-to-many comparison: one MOD VCA corresponds to Poly audio and CV variants, and MOD CV ABS corresponds to two Poly implementations. Counts therefore differ between the two sides. Related variants and partial overlaps below are not included in these verified-counterpart totals. The new functional review covers AMS and MOD CV; unreviewed families are labelled “equivalent not established”, not claimed to be absent.

### Verified AMS lineage and basic CV counterparts

| Poly plugin / Beebo mapping | Poly ID | Patchstorage counterpart | Patchstorage ID | Relationship / observed differences |
| --- | --- | --- | --- | --- |
| [AMS Envelope](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/src/env.cpp) | `http://avwlv2.sourceforge.net/plugins/avw/env` | [AMS Envelope](https://patchstorage.com/mod-ams-lv2-bundle/) | `http://github.com/blablack/ams-lv2/env` | AMS lineage variant: Same AMS envelope algorithm and port layout; MOD source changes peak from 1 to 10 and sustain scaling. IDs and CV levels differ. [MOD source](https://github.com/mod-audio/mod-ams-lv2/blob/81c4aea173e6a9507312f1a8e173341b485d2792/src/env.cpp) |
| [AMS VCF](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/src/vcf.cpp) | `http://avwlv2.sourceforge.net/plugins/avw/vcf` | [AMS VCF](https://patchstorage.com/mod-ams-lv2-bundle/) | `http://github.com/blablack/ams-lv2/vcf` | AMS lineage variant: AMS multi-mode VCF lineage and same port symbols; MOD changes CV modulation scaling and filter-state handling. [MOD source](https://github.com/mod-audio/mod-ams-lv2/blob/81c4aea173e6a9507312f1a8e173341b485d2792/src/vcf.cpp) |
| [AMS VCA Exp - Audio](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/src/vcaexp_audio.cpp) / `mix_vca` | `http://avwlv2.sourceforge.net/plugins/avw/vcaexp_audio` | [AMS VCA Exp](https://patchstorage.com/mod-ams-lv2-bundle/) | `http://github.com/blablack/ams-lv2/vcaexp` | AMS lineage variant: Same AMS exponential gain/mixing algorithm; MOD scales modulation inputs by 0.1. Audio ports correspond. [MOD source](https://github.com/mod-audio/mod-ams-lv2/blob/81c4aea173e6a9507312f1a8e173341b485d2792/src/vcaexp.cpp) |
| [AMS VCA Exp - CV](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/src/vcaexp_cv.cpp) | `http://avwlv2.sourceforge.net/plugins/avw/vcaexp_cv` | [AMS VCA Exp](https://patchstorage.com/mod-ams-lv2-bundle/) | `http://github.com/blablack/ams-lv2/vcaexp` | AMS lineage variant: Same AMS exponential gain/mixing algorithm; MOD scales modulation inputs by 0.1. Poly variant processes CV signals; the MOD bundle variant uses audio signal ports, so it is a related port variant. [MOD source](https://github.com/mod-audio/mod-ams-lv2/blob/81c4aea173e6a9507312f1a8e173341b485d2792/src/vcaexp.cpp) |
| [AMS VCA Lin - Audio](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/src/vcalin_audio.cpp) | `http://avwlv2.sourceforge.net/plugins/avw/vcalin_audio` | [AMS VCA Lin](https://patchstorage.com/mod-ams-lv2-bundle/) | `http://github.com/blablack/ams-lv2/vcalin` | AMS lineage variant: Same AMS linear gain/mixing algorithm; MOD scales modulation inputs by 0.2. Audio ports correspond. [MOD source](https://github.com/mod-audio/mod-ams-lv2/blob/81c4aea173e6a9507312f1a8e173341b485d2792/src/vcalin.cpp) |
| [AMS VCA Lin - CV](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/src/vcalin_cv.cpp) | `http://avwlv2.sourceforge.net/plugins/avw/vcalin_cv` | [AMS VCA Lin](https://patchstorage.com/mod-ams-lv2-bundle/) | `http://github.com/blablack/ams-lv2/vcalin` | AMS lineage variant: Same AMS linear gain/mixing algorithm; MOD scales modulation inputs by 0.2. Poly variant processes CV signals; the MOD bundle variant uses audio signal ports, so it is a related port variant. [MOD source](https://github.com/mod-audio/mod-ams-lv2/blob/81c4aea173e6a9507312f1a8e173341b485d2792/src/vcalin.cpp) |
| [AMS LFO - Freq](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/src/lfo_freq.cpp) | `http://avwlv2.sourceforge.net/plugins/avw/lfo_freq` | [AMS LFO2 - Freq](https://patchstorage.com/mod-ams-lv2-bundle/) | `http://github.com/blablack/ams-lv2/lfo2_freq` | AMS lineage variant: Shared AMS LFO code structure and waveform set; MOD LFO2 multiplies output by 5 and changes reset threshold and state updates. [MOD source](https://github.com/mod-audio/mod-ams-lv2/blob/81c4aea173e6a9507312f1a8e173341b485d2792/src/lfo2_freq.cpp) |
| [AMS VCO2 - Audio](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/src/vco2_audio.cpp) | `http://avwlv2.sourceforge.net/plugins/avw/vco2_audio` | [AMS VCO3](https://patchstorage.com/mod-ams-lv2-bundle/) | `http://github.com/blablack/ams-lv2/vco3` | AMS lineage variant: AMS oscillator lineage; MOD VCO3 adds a sync input, changes FM/PWM scaling and frequency limits. Both output audio. [MOD source](https://github.com/mod-audio/mod-ams-lv2/blob/81c4aea173e6a9507312f1a8e173341b485d2792/src/vco3.cpp) |
| [AMS VCO2 - CV](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/src/vco2_cv.cpp) | `http://avwlv2.sourceforge.net/plugins/avw/vco2_cv` | [AMS VCO3](https://patchstorage.com/mod-ams-lv2-bundle/) | `http://github.com/blablack/ams-lv2/vco3` | AMS lineage variant: AMS oscillator lineage; MOD VCO3 adds a sync input, changes FM/PWM scaling and frequency limits. Poly variant outputs CV; MOD VCO3 outputs audio. [MOD source](https://github.com/mod-audio/mod-ams-lv2/blob/81c4aea173e6a9507312f1a8e173341b485d2792/src/vco3.cpp) |
| [Signal Absolute Value](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/src/absolute.cpp) | `http://avwlv2.sourceforge.net/plugins/avw/absolute` | [CV ABS](https://patchstorage.com/cv-abs/) | `http://moddevices.com/plugins/mod-devel/mod-cv-abs` | Functional counterpart: Both rectify a sample to its absolute value. MOD adds enabled/bypass handling; different implementation and URI. [MOD source](https://github.com/mod-audio/mod-cv-plugins/blob/a2feb53157ca5605ae8fea8149abef9cff948cb4/source/mod-cv-abs/mod-cv-abs.c) |
| [abs](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/basic_modular.lv2/difference.c) / `rectify_value` | `http://polyeffects.com/lv2/basic_modular#abs` | [CV ABS](https://patchstorage.com/cv-abs/) | `http://moddevices.com/plugins/mod-devel/mod-cv-abs` | Functional counterpart: Same absolute-value operation when Poly input offset is zero. Poly permits an input offset; MOD adds enabled/bypass handling. Beebo calls the Poly plugin rectify_value. [MOD source](https://github.com/mod-audio/mod-cv-plugins/blob/a2feb53157ca5605ae8fea8149abef9cff948cb4/source/mod-cv-abs/mod-cv-abs.c) |
| [Control to CV](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/src/controltocv.cpp) | `http://avwlv2.sourceforge.net/plugins/avw/controltocv` | [Control to CV](https://patchstorage.com/control-to-cv/) | `http://moddevices.com/plugins/mod-devel/mod-cv-control` | Functional counterpart: Both broadcast a control value as a constant CV buffer when MOD smoothing is disabled. MOD adds optional smoothing and has different control ranges. [MOD source](https://github.com/mod-audio/mod-cv-plugins/blob/a2feb53157ca5605ae8fea8149abef9cff948cb4/source/mod-cv-control/mod-cv-control.c) |

### Related variants and partial overlap

| Poly plugin / mapping | Patchstorage plugin | Relationship | Limits / evidence |
| --- | --- | --- | --- |
| [AMS LFO - Tempo](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/src/lfo_tempo.cpp) | [AMS LFO2 - Freq](https://patchstorage.com/mod-ams-lv2-bundle/) | Related variant | Same AMS waveform family, but Beebo/Poly LFO is controlled by tempo and multiplier; MOD LFO2 is frequency-controlled. This is not counted as a verified equivalent. [MOD source](https://github.com/mod-audio/mod-ams-lv2/blob/81c4aea173e6a9507312f1a8e173341b485d2792/src/lfo2_freq.cpp) |
| [AMS CV Source](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/src/cvs.cpp) | [Control to CV](https://patchstorage.com/control-to-cv/) | Partial functional overlap | Both generate constant CV values, but Poly AMS CV Source provides four coarse/fine outputs; MOD Control to CV provides one output and optional smoothing. [MOD source](https://github.com/mod-audio/mod-cv-plugins/blob/a2feb53157ca5605ae8fea8149abef9cff948cb4/source/mod-cv-control/mod-cv-control.c) |
| [AMS Amp](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/src/amp.cpp) | [Attenuverter Booster](https://patchstorage.com/attenuverter-booster/) | Partial functional overlap | Both can multiply a signal by gain. MOD adds offset, smoothing and bypass; audio/CV port typing and gain ranges require adaptation. [MOD source](https://github.com/mod-audio/mod-cv-plugins/blob/a2feb53157ca5605ae8fea8149abef9cff948cb4/source/mod-cv-attenuverter/mod-cv-attenuverter.c) |
| [boost](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/basic_modular.lv2/amp.c) | [Attenuverter Booster](https://patchstorage.com/attenuverter-booster/) | Partial functional overlap | Both have a linear gain operation, but the Poly boost is an audio effect with an on control, while MOD is a bipolar CV gain/offset processor. [MOD source](https://github.com/mod-audio/mod-cv-plugins/blob/a2feb53157ca5605ae8fea8149abef9cff948cb4/source/mod-cv-attenuverter/mod-cv-attenuverter.c) |
| [product](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/basic_modular.lv2/difference.c) | [Attenuverter Booster](https://patchstorage.com/attenuverter-booster/) | Partial functional overlap | Poly product can perform CV multiplication; MOD attenuation plus output offset/smoothing is not the same complete interface or algorithm. [MOD source](https://github.com/mod-audio/mod-cv-plugins/blob/a2feb53157ca5605ae8fea8149abef9cff948cb4/source/mod-cv-attenuverter/mod-cv-attenuverter.c) |
| [attenuverter](https://github.com/polyeffects/digit_ui/blob/feb4df451067850770ddd13af9f4e2828234dabc/module_info.py) | [Attenuverter Booster](https://patchstorage.com/attenuverter-booster/) | Partial functional overlap | Beebo maps attenuverter to BLOP product. Same gain/multiplication task; MOD also has an output offset and smoothing. No identical ID or whole-plugin equivalence is established. [MOD source](https://github.com/mod-audio/mod-cv-plugins/blob/a2feb53157ca5605ae8fea8149abef9cff948cb4/source/mod-cv-attenuverter/mod-cv-attenuverter.c) |
| [schmitt](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/basic_modular.lv2/difference.c) | [CV Gate](https://patchstorage.com/cv-gate/) | Partial functional overlap | Both use threshold logic, but Poly schmitt generates a trigger while MOD gates a separate input signal using open/close thresholds. [MOD source](https://github.com/mod-audio/mod-cv-plugins/blob/a2feb53157ca5605ae8fea8149abef9cff948cb4/source/mod-cv-gate/mod-cv-gate.c) |

### MOD CV entries: individual review

| Patchstorage CV plugin | Relationship to Poly | Finding |
| --- | --- | --- |
| [CV meter](https://patchstorage.com/cv-meter/) | Exact identity | Beebo cv_meter already uses the same MOD URI. [source](https://github.com/mod-audio/mod-cv-plugins/blob/a2feb53157ca5605ae8fea8149abef9cff948cb4/source/mod-cv-meter/mod-cv-meter.c) |
| [CV ABS](https://patchstorage.com/cv-abs/) | Functional counterparts verified | Poly AVW Signal Absolute Value and PolyLV2 abs / Beebo rectify_value. [source](https://github.com/mod-audio/mod-cv-plugins/blob/a2feb53157ca5605ae8fea8149abef9cff948cb4/source/mod-cv-abs/mod-cv-abs.c) |
| [Control to CV](https://patchstorage.com/control-to-cv/) | Functional counterpart verified | Poly AVW Control to CV; optional smoothing differs. AMS CV Source is a related four-output variant. [source](https://github.com/mod-audio/mod-cv-plugins/blob/a2feb53157ca5605ae8fea8149abef9cff948cb4/source/mod-cv-control/mod-cv-control.c) |
| [Attenuverter Booster](https://patchstorage.com/attenuverter-booster/) | Partial functional overlap | Poly gain/product and Beebo attenuverter can do multiplication; MOD adds offset/smoothing and has different ports/ranges. [source](https://github.com/mod-audio/mod-cv-plugins/blob/a2feb53157ca5605ae8fea8149abef9cff948cb4/source/mod-cv-attenuverter/mod-cv-attenuverter.c) |
| [CV Gate](https://patchstorage.com/cv-gate/) | Partial functional overlap, not the same plugin | MOD passes a separate CV input through a threshold-controlled gate. Poly schmitt emits a trigger instead. [source](https://github.com/mod-audio/mod-cv-plugins/blob/a2feb53157ca5605ae8fea8149abef9cff948cb4/source/mod-cv-gate/mod-cv-gate.c) |
| [CV Round](https://patchstorage.com/cv-round/) | No equivalent established | MOD exposes round, ceil, floor and fraction outputs. Poly musical-scale quantizer is not the same operation. [source](https://github.com/mod-audio/mod-cv-plugins/blob/a2feb53157ca5605ae8fea8149abef9cff948cb4/source/mod-cv-round/mod-cv-round.c) |
| [CV Range Divider](https://patchstorage.com/cv-range-divider/) | No equivalent established | MOD routes input to inside-range/outside-range outputs. Poly min/max arithmetic is not this routing operation. [source](https://github.com/mod-audio/mod-cv-plugins/blob/a2feb53157ca5605ae8fea8149abef9cff948cb4/source/mod-cv-range/mod-cv-range.c) |
| [Random Generator](https://patchstorage.com/random-generator/) | Related function, not the same plugin | MOD is a triggered sample-and-hold random value generator with min/max controls. Poly noise and Marbles differ in timing, outputs and behavior. [source](https://github.com/mod-audio/mod-cv-plugins/blob/a2feb53157ca5605ae8fea8149abef9cff948cb4/source/mod-cv-random/mod-cv-random.c) |
| [CV Parameter Modulation](https://patchstorage.com/cv-parameter-modulation/) | Related operation, not the same plugin | MOD applies depth and offset plus 0–10 clamping and MOD parameter integration. A Poly multiply/add graph can overlap, but no single identical processor was established. [source](https://github.com/mod-audio/mod-cv-plugins/blob/a2feb53157ca5605ae8fea8149abef9cff948cb4/source/mod-cv-parameter-modulation/mod-cv-parameter-modulation.c) |
| [Audio to CV](https://patchstorage.com/audio-to-cv/) | Related arithmetic, not the same plugin | MOD declares an audio input and CV output with gain/offset. Poly AVW Amp has different port typing and no output offset. [source](https://github.com/mod-audio/mod-cv-plugins/blob/a2feb53157ca5605ae8fea8149abef9cff948cb4/source/mod-audio-to-cv/mod-audio-to-cv.c) |
| [Multi Button to CV](https://patchstorage.com/multi-button-to-cv/) | Unconfirmed correspondence | Poly footswitch mappings have a related purpose, but their implementation is absent from the public collection; source identity cannot be verified. [source](https://github.com/mod-audio/mod-cv-plugins/blob/a2feb53157ca5605ae8fea8149abef9cff948cb4/source/mod-button-to-cv/mod-button-to-cv.c) |

The claim that all CV plugins are identical is not supported by this review. Absolute value and control-to-CV have verified basic-operation counterparts; CV meter is an exact Beebo reference. MOD CV Round, Range Divider and other CV utilities have distinct interfaces and behavior. For example, CV Round provides separate arithmetic outputs, while Poly’s quantizer maps musical scales. These share the CV format without being the same plugin.

## Exact URI matches — 47

| Plugin / Beebo mapping | Poly implementation source | Patchstorage entry | Exact ID |
| --- | --- | --- | --- |
| `amp_bass` | Beebo reference only | Exact ID: [GxSVT](https://patchstorage.com/gxsvt/) | `http://guitarix.sourceforge.net/plugins/gx_ampegsvt_#_ampegsvt_` |
| `auto_swell` | Beebo reference only | Exact ID: [GxSlowGear](https://patchstorage.com/gxslowgear/) | `http://guitarix.sourceforge.net/plugins/gx_slowgear_#_slowgear_` |
| `basic_reverb` | Beebo reference only | Exact ID: [MVerb](https://patchstorage.com/mverb/) | `http://distrho.sf.net/plugins/MVerb` |
| `cv_meter` | Beebo reference only | Exact ID: [CV meter](https://patchstorage.com/cv-meter/) | `http://moddevices.com/plugins/mod-devel/mod-cv-meter` |
| `filter` | Beebo reference only | Exact ID: [fomp.lv2 Bundle — bundle extra](https://patchstorage.com/fomp-lv2-bundle/) | `http://drobilla.net/plugins/fomp/mvclpf1` |
| `freeze` | Beebo reference only | Exact ID: [stuck.lv2 Bundle](https://patchstorage.com/stuck-lv2-bundle/) | `http://ssj71.github.io/infamousPlugins/plugs.html#stuck` |
| `midi_cc_to_note` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#cctonote` |
| `midi_channel_filter` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#channelfilter` |
| `midi_channel_map` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#channelmap` |
| `midi_chord` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#midichord` |
| `midi_clock_out` | Beebo reference only | Exact ID: [MIDI Clock Generator](https://patchstorage.com/midi-clock-generator/) | `http://gareus.org/oss/lv2/mclk` |
| `midi_delay` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#mididelay` |
| `midi_dup` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#mididup` |
| `midi_enforce_scale` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#enforcescale` |
| `midi_event_blocker` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#eventblocker` |
| `midi_keyrange` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#keyrange` |
| `midi_keysplit` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#keysplit` |
| `midi_map_key_channel` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#mapkeychannel` |
| `midi_map_key_scale` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#mapkeyscale` |
| `midi_mapcc` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#mapcc` |
| `midi_mono_legato` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#monolegato` |
| `midi_no_active_sensing` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#noactivesensing` |
| `midi_nodup` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#nodup` |
| `midi_note_to_cc` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#notetocc` |
| `midi_note_toggle` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#notetoggle` |
| `midi_ntap_delay` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#ntapdelay` |
| `midi_one_channel_filter` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#onechannelfilter` |
| `midi_quantize` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#quantize` |
| `midi_rand_velocity` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#randvelocity` |
| `midi_scale_cc` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#scalecc` |
| `midi_sostenuto` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#sostenuto` |
| `midi_strum` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#midistrum` |
| `midi_transpose` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#miditranspose` |
| `midi_velocity_range` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#velocityrange` |
| `midi_velocity_scale` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#velocityscale` |
| `mono_cab` | Beebo reference only | Exact ID: [zeroconvo.lv2 Bundle](https://patchstorage.com/zeroconvo-lv2-bundle/) | `http://gareus.org/oss/lv2/zeroconvolv#CfgMono` |
| `mono_EQ` | Beebo reference only | Exact ID: [fil4.lv2 Bundle](https://patchstorage.com/fil4-lv2-bundle/) | `http://gareus.org/oss/lv2/fil4#mono` |
| `amp_nam` | neural-amp-modeler-lv2 | Exact ID: [Neural Amp Modeler](https://patchstorage.com/neural-amp-modeler/) | `http://github.com/mikeoliphant/neural-amp-modeler-lv2` |
| `pitch_shift` | Beebo reference only | Exact ID: [TAP Pitch Shifter](https://patchstorage.com/tap-pitch-shifter/) | `http://moddevices.com/plugins/tap/pitch` |
| `quad_ir_reverb` | Beebo reference only | Exact ID: [zeroconvo.lv2 Bundle](https://patchstorage.com/zeroconvo-lv2-bundle/) | `http://gareus.org/oss/lv2/zeroconvolv#CfgStereo` |
| `reverse` | Beebo reference only | Exact ID: [TAP Reflector](https://patchstorage.com/tap-reflector/) | `http://moddevices.com/plugins/tap/reflector` |
| `rotary` | Beebo reference only | Exact ID: [setBfree Whirl Speaker (Old version)](https://patchstorage.com/setbfree-whirl-speaker-old-version/) | `http://gareus.org/oss/lv2/b_whirl#simple` |
| `saturator` | Beebo reference only | Exact ID: [TAP Sigmoid Booster](https://patchstorage.com/tap-sigmoid-booster/) | `http://moddevices.com/plugins/tap/sigmoid` |
| `stereo_EQ` | Beebo reference only | Exact ID: [fil4.lv2 Bundle](https://patchstorage.com/fil4-lv2-bundle/) | `http://gareus.org/oss/lv2/fil4#stereo` |
| `turntable_stop` | Beebo reference only | Exact ID: [the infamous power cut](https://patchstorage.com/the-infamous-power-cut/) | `http://ssj71.github.io/infamousPlugins/plugs.html#powercut` |
| `warmth` | Beebo reference only | Exact ID: [TAP Tubewarmth](https://patchstorage.com/tap-tubewarmth/) | `http://moddevices.com/plugins/tap/tubewarmth` |
| `tuner` | tuna.lv2 | Exact ID: [Instrument Tuner](https://patchstorage.com/instrument-tuner/) | `http://gareus.org/oss/lv2/tuna#mod` |

## Complete Patchstorage catalogue — 385 entries, 552 inventoried IDs

Each row records one plugin identity in one entry; an identity found in several entries is repeated to preserve packaging information. Single-plugin rows retain the website’s title. Bundle rows use names from downloaded LV2 metadata where available; otherwise the exact URI suffix is shown as the identifier. Every one of the 385 entries has at least one row. Entries marked WIP are included to preserve the complete catalogue.

| Entry / plugin | Exact ID | Builds | State | Comparison |
| --- | --- | --- | --- | --- |
| [2Voices](https://patchstorage.com/2voices/) | `http://moddevices.com/plugins/mod-devel/2Voices` | x64, Pi64 | WIP | No exact or reviewed equivalent match |
| [3 Band EQ](https://patchstorage.com/3-band-eq/) | `http://distrho.sf.net/plugins/3BandEQ` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [3 Band Splitter](https://patchstorage.com/3-band-splitter/) | `http://distrho.sf.net/plugins/3BandSplitter` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [abGate](https://patchstorage.com/abgate/) | `http://hippie.lt/lv2/gate` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Acceleration2](https://patchstorage.com/acceleration2/) | `https://hannesbraun.net/ns/lv2/airwindows/acceleration2` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [ADClip7](https://patchstorage.com/adclip7/) | `https://hannesbraun.net/ns/lv2/airwindows/adclip7` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Aether](https://patchstorage.com/aether/) | `http://github.com/Dougal-s/Aether` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [AIDA-X](https://patchstorage.com/aida-x/) | `http://aidadsp.cc/plugins/aidadsp-bundle/rt-neural-generic` | PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [ALO Plugin — ALO](https://patchstorage.com/alo-plugin/) | `http://devcurmudgeon.com/alo` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Amp Profiler](https://patchstorage.com/amp-profiler/) | `https://faustlv2.bitbucket.io/profiler` | PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Amplitude Imposer](https://patchstorage.com/amplitude-imposer/) | `http://www.niallmoody.com/ndcplugs/ampimposer.htm` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [amsynth](https://patchstorage.com/amsynth/) | `http://code.google.com/p/amsynth/amsynth` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Arpeggiator](https://patchstorage.com/arpeggiator/) | `http://moddevices.com/plugins/mod-devel/arpeggiator` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [artyfx.lv2 Bundle — Driva [B]](https://patchstorage.com/artyfx-lv2-bundle/) | `http://www.openavproductions.com/artyfx#driva` | Pi64 | WIP | No exact or reviewed equivalent match |
| [artyfx.lv2 Bundle — Filta](https://patchstorage.com/artyfx-lv2-bundle/) | `http://www.openavproductions.com/artyfx#filta` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [artyfx.lv2 Bundle — Kuiza](https://patchstorage.com/artyfx-lv2-bundle/) | `http://www.openavproductions.com/artyfx#kuiza` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [artyfx.lv2 Bundle — Roomy](https://patchstorage.com/artyfx-lv2-bundle/) | `http://www.openavproductions.com/artyfx#roomy` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Attenuverter Booster](https://patchstorage.com/attenuverter-booster/) | `http://moddevices.com/plugins/mod-devel/mod-cv-attenuverter` | x64, PB32, Pi64 | WIP | Related function in Poly (partial) |
| [Audio to CV](https://patchstorage.com/audio-to-cv/) | `http://moddevices.com/plugins/mod-devel/mod-audio-to-cv` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [AudioToCV Pitch](https://patchstorage.com/audiotocv-pitch/) | `https://distrho.kx.studio/plugins/pitchtracking#cv` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Autotune](https://patchstorage.com/autotune-2/) | `http://gareus.org/oss/lv2/fat1` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [avocado](https://patchstorage.com/avocado/) | `http://remaincalm.org/plugins/avocado` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [BandPassFilter](https://patchstorage.com/bandpassfilter/) | `http://moddevices.com/plugins/mod-devel/BandPassFilter` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Baxandall](https://patchstorage.com/baxandall/) | `https://hannesbraun.net/ns/lv2/airwindows/baxandall` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Bitta](https://patchstorage.com/bitta/) | `http://www.openavproductions.com/artyfx#bitta` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Black Pearl 4A](https://patchstorage.com/black-pearl-4a/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_Black_Pearl_4A` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Black Pearl 4B](https://patchstorage.com/black-pearl-4b/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_Black_Pearl_4B` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Black Pearl 5](https://patchstorage.com/black-pearl-5/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_Black_Pearl_5` | Pi64 | Ready | No exact or reviewed equivalent match |
| [bluesbreaker](https://patchstorage.com/bluesbreaker/) | `urn:brummer:bluesbreaker` | PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Bollie Delay](https://patchstorage.com/bollie-delay/) | `https://ca9.eu/lv2/bolliedelay` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Bollie Delay XT](https://patchstorage.com/bollie-delay-xt/) | `https://ca9.eu/lv2/bolliedelayxt` | PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Boreas](https://patchstorage.com/boreas/) | `http://boreas.local/plugins/boreas` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [C* AmpVTS - Tube amp + Tone stack](https://patchstorage.com/c-ampvts-tube-amp-tone-stack/) | `http://moddevices.com/plugins/caps/AmpVTS` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* AutoFilter](https://patchstorage.com/c-autofilter/) | `http://moddevices.com/plugins/caps/AutoFilter` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* CabinetIII - Idealised loudspeaker cabinet emulation](https://patchstorage.com/c-cabinetiii-idealised-loudspeaker-cabinet-emulation/) | `http://moddevices.com/plugins/caps/CabinetIII` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* CabinetIV - Idealised loudspeaker cabinet emulation](https://patchstorage.com/c-cabinetiv-idealised-loudspeaker-cabinet-emulation/) | `http://moddevices.com/plugins/caps/CabinetIV` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* CEO - Chief Executive Oscillator](https://patchstorage.com/c-ceo-chief-executive-oscillator/) | `http://moddevices.com/plugins/caps/CEO` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* ChorusI - Mono chorus/flanger](https://patchstorage.com/c-chorusi-mono-chorus-flanger/) | `http://moddevices.com/plugins/caps/ChorusI` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* Click - Metronome](https://patchstorage.com/c-click-metronome/) | `http://moddevices.com/plugins/caps/Click` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* Compress - Mono compressor](https://patchstorage.com/c-compress-mono-compressor/) | `http://moddevices.com/plugins/caps/Compress` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* CompressX2 - Stereo compressor](https://patchstorage.com/c-compressx2-stereo-compressor/) | `http://moddevices.com/plugins/caps/CompressX2` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* Eq10 - 10-band equalizer](https://patchstorage.com/c-eq10-10-band-equalizer/) | `http://moddevices.com/plugins/caps/Eq10` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* Eq10X2 - 10-band equalizer](https://patchstorage.com/c-eq10x2-10-band-equalizer/) | `http://moddevices.com/plugins/caps/Eq10X2` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* Fractal - Audio stream from deterministic chaos](https://patchstorage.com/c-fractal-audio-stream-from-deterministic-chaos/) | `http://moddevices.com/plugins/caps/Fractal` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* Narrower - Stereo image width reduction](https://patchstorage.com/c-narrower-stereo-image-width-reduction/) | `http://moddevices.com/plugins/caps/Narrower` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* Noisegate - Attenuate noise resident in silence](https://patchstorage.com/c-noisegate-attenuate-noise-resident-in-silence/) | `http://moddevices.com/plugins/caps/Noisegate` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* PhaserII - Mono phaser modulated by a Lorenz fractal](https://patchstorage.com/c-phaserii-mono-phaser-modulated-by-a-lorenz-fractal/) | `http://moddevices.com/plugins/caps/PhaserII` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* Plate - Versatile plate reverb](https://patchstorage.com/c-plate-versatile-plate-reverb/) | `http://moddevices.com/plugins/caps/Plate` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* PlateX2 - Stereo in/out Versatile plate reverb](https://patchstorage.com/c-platex2-stereo-in-out-versatile-plate-reverb/) | `http://moddevices.com/plugins/caps/PlateX2` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* Saturate](https://patchstorage.com/c-saturate/) | `http://moddevices.com/plugins/caps/Saturate` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* Scape - Stereo delay + Filters](https://patchstorage.com/c-scape-stereo-delay-filters/) | `http://moddevices.com/plugins/caps/Scape` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* Sin - Sine wave generator](https://patchstorage.com/c-sin-sine-wave-generator/) | `http://moddevices.com/plugins/caps/Sin` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* Spice](https://patchstorage.com/c-spice/) | `http://moddevices.com/plugins/caps/Spice` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* SpiceX2](https://patchstorage.com/c-spicex2/) | `http://moddevices.com/plugins/caps/SpiceX2` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* ToneStack - Tone stack emulation](https://patchstorage.com/c-tonestack-tone-stack-emulation/) | `http://moddevices.com/plugins/caps/ToneStack` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* White - White noise generator](https://patchstorage.com/c-white-white-noise-generator/) | `http://moddevices.com/plugins/caps/White` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [C* Wider - Stereo image Synthesis](https://patchstorage.com/c-wider-stereo-image-synthesis/) | `http://moddevices.com/plugins/caps/Wider` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Cabinet Loader](https://patchstorage.com/cabinet-loader/) | `https://mod.audio/plugins/CabinetLoader` | PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Analyzer [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Analyzer` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Bass Enhancer [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/BassEnhancer` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Compensation Delay Line [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/CompensationDelay` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Compressor [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Compressor` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Crusher [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Crusher` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Deesser [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Deesser` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Emphasis [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Emphasis` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Envelope Filter](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/EnvelopeFilter` | x64, PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Equalizer 12 Band [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Equalizer12Band` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Equalizer 30 Band [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Equalizer30Band` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Equalizer 5 Band [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Equalizer5Band` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Equalizer 8 Band [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Equalizer8Band` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Exciter [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Exciter` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Filter [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Filter` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Filterclavier [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Filterclavier` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Flanger](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Flanger` | x64, PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Gate [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Gate` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Haas Stereo Enhancer [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/HaasEnhancer` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Limiter [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Limiter` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Mono Compressor [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/MonoCompressor` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Mono Input [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/MonoInput` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Monosynth [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Monosynth` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Multi Chorus [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/MultiChorus` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Multiband Compressor [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/MultibandCompressor` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Multiband Gate [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/MultibandGate` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Multiband Limiter [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/MultibandLimiter` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Organ [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Organ` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Phaser](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Phaser` | x64, PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Pulsator [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Pulsator` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Reverb [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Reverb` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Reverse Delay [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/ReverseDelay` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Ring Modulator [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/RingModulator` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Rotary Speaker [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/RotarySpeaker` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Saturator](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Saturator` | x64, PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Sidechain Compressor [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/SidechainCompressor` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Sidechain Gate [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/SidechainGate` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Sidechain Limiter [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/SidechainLimiter` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Stereo Tools [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/StereoTools` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Tape Simulator [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/TapeSimulator` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Transient Designer [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/TransientDesigner` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Vintage Delay [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/VintageDelay` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf Vocoder [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/Vocoder` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf X-Over 2 Band [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/XOver2Band` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf X-Over 3 Band [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/XOver3Band` | PB32 | WIP | No exact or reviewed equivalent match |
| [calf.lv2 Bundle — Calf X-Over 4 Band [B]](https://patchstorage.com/calf-lv2-bundle/) | `http://calf.sourceforge.net/plugins/XOver4Band` | PB32 | WIP | No exact or reviewed equivalent match |
| [Capacitor](https://patchstorage.com/capacitor/) | `https://hannesbraun.net/ns/lv2/airwindows/capacitor` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Capacitor2](https://patchstorage.com/capacitor2/) | `https://hannesbraun.net/ns/lv2/airwindows/capacitor2` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Capo Plugin — Capo](https://patchstorage.com/capo-plugin/) | `http://moddevices.com/plugins/mod-devel/Capo` | x64, Pi64 | WIP | No exact or reviewed equivalent match |
| [carla-audiogain.lv2 Bundle — Audio Gain (Mono)](https://patchstorage.com/carla-audiogain-lv2-bundle/) | `http://kxstudio.sf.net/carla/plugins/audiogain` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [carla-audiogain.lv2 Bundle — Audio Gain (Stereo)](https://patchstorage.com/carla-audiogain-lv2-bundle/) | `http://kxstudio.sf.net/carla/plugins/audiogain_s` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [carla-files.lv2 Bundle — Audio File](https://patchstorage.com/carla-files-lv2-bundle/) | `http://kxstudio.sf.net/carla/plugins/audiofile` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [carla-files.lv2 Bundle — MIDI File](https://patchstorage.com/carla-files-lv2-bundle/) | `http://kxstudio.sf.net/carla/plugins/midifile` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Channel8](https://patchstorage.com/channel8/) | `https://hannesbraun.net/ns/lv2/airwindows/channel8` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [ChowCentaur](https://patchstorage.com/chowcentaur/) | `https://github.com/jatinchowdhury18/KlonCentaur` | PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [ClipOnly](https://patchstorage.com/cliponly/) | `https://hannesbraun.net/ns/lv2/airwindows/cliponly` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [ClipOnly2](https://patchstorage.com/cliponly2/) | `https://hannesbraun.net/ns/lv2/airwindows/cliponly2` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [CollisionDrive](https://patchstorage.com/collisiondrive/) | `https://github.com/brummer10/CollisionDrive` | PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Compressor](https://patchstorage.com/compressor-2/) | `http://moddevices.com/plugins/mod-devel/System-Compressor` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Compressor Advanced](https://patchstorage.com/compressor-advanced/) | `http://moddevices.com/plugins/mod-devel/Advanced-Compressor` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Console7Buss](https://patchstorage.com/console7buss/) | `https://hannesbraun.net/ns/lv2/airwindows/console7buss` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Console7Cascade](https://patchstorage.com/console7cascade/) | `https://hannesbraun.net/ns/lv2/airwindows/console7cascade` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Console7Channel](https://patchstorage.com/console7channel/) | `https://hannesbraun.net/ns/lv2/airwindows/console7channel` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Console7Crunch](https://patchstorage.com/console7crunch/) | `https://hannesbraun.net/ns/lv2/airwindows/console7crunch` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Control to CV](https://patchstorage.com/control-to-cv/) | `http://moddevices.com/plugins/mod-devel/mod-cv-control` | x64, PB32, Pi64 | WIP | Functional counterpart in Poly |
| [Convolution Loader — MOD Cabinet Loader [B]](https://patchstorage.com/convolution-loader/) | `https://mod.audio/plugins/CabinetLoader` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Convolution Loader — MOD Convolution Loader](https://patchstorage.com/convolution-loader/) | `https://mod.audio/plugins/ConvolutionLoader` | PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [CrossOver 2](https://patchstorage.com/crossover-2/) | `http://moddevices.com/plugins/mod-devel/CrossOver2` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [CrossOver 3](https://patchstorage.com/crossover-3/) | `http://moddevices.com/plugins/mod-devel/CrossOver3` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [CV ABS](https://patchstorage.com/cv-abs/) | `http://moddevices.com/plugins/mod-devel/mod-cv-abs` | x64, PB32, Pi64 | WIP | Functional counterpart in Poly |
| [CV Gate](https://patchstorage.com/cv-gate/) | `http://moddevices.com/plugins/mod-devel/mod-cv-gate` | x64, PB32, Pi64 | WIP | Related function in Poly (partial) |
| [CV meter](https://patchstorage.com/cv-meter/) | `http://moddevices.com/plugins/mod-devel/mod-cv-meter` | x64, PB32, Pi64 | WIP | Exact Beebo reference |
| [CV Parameter Modulation](https://patchstorage.com/cv-parameter-modulation/) | `http://moddevices.com/plugins/mod-devel/mod-cv-parameter-modulation` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [CV Range Divider](https://patchstorage.com/cv-range-divider/) | `http://moddevices.com/plugins/mod-devel/mod-cv-range` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [CV Round](https://patchstorage.com/cv-round/) | `http://moddevices.com/plugins/mod-devel/mod-cv-round` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [DeArmondo610](https://patchstorage.com/dearmondo610/) | `urn:hvcc:DeArmondo610` | x64, Pi64 | Ready | No exact or reviewed equivalent match |
| [DeBess](https://patchstorage.com/debess/) | `https://hannesbraun.net/ns/lv2/airwindows/debess` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Dexed](https://patchstorage.com/dexed/) | `https://github.com/dcoredump/dexed.lv2` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [DIE Fluid Synth](https://patchstorage.com/die-fluid-synth/) | `urn:distrho:a-fluidsynth` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Dimension IV](https://patchstorage.com/dimension-iv/) | `urn:hvcc:Dimension_IV` | x64, Pi64 | Ready | No exact or reviewed equivalent match |
| [Dirty Talk](https://patchstorage.com/dirty-talk/) | `https://github.com/pilali/Dirty-Talk` | x64, Pi64 | Ready | No exact or reviewed equivalent match |
| [Dragonfly Early Reflections](https://patchstorage.com/dragonfly-early-reflections/) | `urn:dragonfly:early` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Dragonfly Hall Reverb](https://patchstorage.com/dragonfly-hall-reverb/) | `https://github.com/michaelwillis/dragonfly-reverb` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Dragonfly Plate Reverb](https://patchstorage.com/dragonfly-plate-reverb/) | `urn:dragonfly:plate` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Dragonfly Room Reverb](https://patchstorage.com/dragonfly-room-reverb/) | `urn:dragonfly:room` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Drop Plugin — Drop](https://patchstorage.com/drop-plugin/) | `http://moddevices.com/plugins/mod-devel/Drop` | x64, Pi64 | WIP | No exact or reviewed equivalent match |
| [DS1 Plugin — DS1](https://patchstorage.com/ds1-plugin/) | `http://moddevices.com/plugins/mod-devel/DS1` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Dyno Plugin — Dyno](https://patchstorage.com/dyno-plugin/) | `https://hannesbraun.net/ns/lv2/airwindows/dyno` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Echomatica](https://patchstorage.com/echomatica/) | `urn:hvcc:Echomatica` | x64, Pi64 | Ready | No exact or reviewed equivalent match |
| [EdIsDim](https://patchstorage.com/edisdim/) | `https://hannesbraun.net/ns/lv2/airwindows/edisdim` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [EveryTrim](https://patchstorage.com/everytrim/) | `https://hannesbraun.net/ns/lv2/airwindows/everytrim` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [FatFrog](https://patchstorage.com/fatfrog/) | `https://github.com/brummer10/FatFrog#_FatFrog_` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [fil4.lv2 Bundle — x42-eq - Parametric Equalizer Mono](https://patchstorage.com/fil4-lv2-bundle/) | `http://gareus.org/oss/lv2/fil4#mono` | x64, PB32, Pi64 | WIP | Exact Beebo reference |
| [fil4.lv2 Bundle — x42-eq - Parametric Equalizer Stereo](https://patchstorage.com/fil4-lv2-bundle/) | `http://gareus.org/oss/lv2/fil4#stereo` | x64, PB32, Pi64 | WIP | Exact Beebo reference |
| [floaty](https://patchstorage.com/floaty/) | `http://remaincalm.org/plugins/floaty` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Fluid Bass](https://patchstorage.com/fluid-bass/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_FluidBass` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Fluid Brass](https://patchstorage.com/fluid-brass/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_FluidBrass` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Fluid Chromatic Percussion](https://patchstorage.com/fluid-chromatic-percussion/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_FluidChromPerc` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Fluid Drums](https://patchstorage.com/fluid-drums/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_FluidDrums` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Fluid Ensemble](https://patchstorage.com/fluid-ensemble/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_FluidEnsemble` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Fluid Ethnic](https://patchstorage.com/fluid-ethnic/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_FluidEthnic` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Fluid Guitars](https://patchstorage.com/fluid-guitars/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_FluidGuitars` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Fluid Organs](https://patchstorage.com/fluid-organs/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_FluidOrgans` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Fluid Percussion](https://patchstorage.com/fluid-percussion/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_FluidPercussion` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Fluid Pianos](https://patchstorage.com/fluid-pianos/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_FluidPianos` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Fluid Pipes](https://patchstorage.com/fluid-pipes/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_FluidPipes` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Fluid Reeds](https://patchstorage.com/fluid-reeds/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_FluidReeds` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Fluid SoundFX](https://patchstorage.com/fluid-soundfx/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_FluidSoundFX` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Fluid Strings](https://patchstorage.com/fluid-strings/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_FluidStrings` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Fluid SynthFX](https://patchstorage.com/fluid-synthfx/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_FluidSynthFX` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Fluid SynthLeads](https://patchstorage.com/fluid-synthleads/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_FluidSynthLeads` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Fluid SynthPads](https://patchstorage.com/fluid-synthpads/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_FluidSynthPads` | Pi64 | Ready | No exact or reviewed equivalent match |
| [fomp.lv2 Bundle — Auto-wah](https://patchstorage.com/fomp-lv2-bundle/) | `http://drobilla.net/plugins/fomp/autowah` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [fomp.lv2 Bundle — CS Chorus 1](https://patchstorage.com/fomp-lv2-bundle/) | `http://drobilla.net/plugins/fomp/cs_chorus1` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [fomp.lv2 Bundle — CS Chorus 2](https://patchstorage.com/fomp-lv2-bundle/) | `http://drobilla.net/plugins/fomp/cs_chorus2` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [fomp.lv2 Bundle — CS Phaser 1 [B]](https://patchstorage.com/fomp-lv2-bundle/) | `http://drobilla.net/plugins/fomp/cs_phaser1` | Pi64 | WIP | No exact or reviewed equivalent match |
| [fomp.lv2 Bundle — CS Phaser 1 with LFO](https://patchstorage.com/fomp-lv2-bundle/) | `http://drobilla.net/plugins/fomp/cs_phaser1_lfo` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [fomp.lv2 Bundle — Moog High-Pass Filter 1 [B]](https://patchstorage.com/fomp-lv2-bundle/) | `http://drobilla.net/plugins/fomp/mvchpf1` | Pi64 | WIP | No exact or reviewed equivalent match |
| [fomp.lv2 Bundle — Moog Low-Pass Filter 1 [B]](https://patchstorage.com/fomp-lv2-bundle/) | `http://drobilla.net/plugins/fomp/mvclpf1` | Pi64 | WIP | Exact Beebo reference |
| [fomp.lv2 Bundle — Moog Low-Pass Filter 2 [B]](https://patchstorage.com/fomp-lv2-bundle/) | `http://drobilla.net/plugins/fomp/mvclpf2` | Pi64 | WIP | No exact or reviewed equivalent match |
| [fomp.lv2 Bundle — Moog Low-Pass Filter 3 [B]](https://patchstorage.com/fomp-lv2-bundle/) | `http://drobilla.net/plugins/fomp/mvclpf3` | Pi64 | WIP | No exact or reviewed equivalent match |
| [fomp.lv2 Bundle — Moog Low-Pass Filter 4 [B]](https://patchstorage.com/fomp-lv2-bundle/) | `http://drobilla.net/plugins/fomp/mvclpf4` | Pi64 | WIP | No exact or reviewed equivalent match |
| [fomp.lv2 Bundle — 4-Band Parametric Filter [B]](https://patchstorage.com/fomp-lv2-bundle/) | `http://drobilla.net/plugins/fomp/parametric1` | Pi64 | WIP | No exact or reviewed equivalent match |
| [fomp.lv2 Bundle — Pulse VCO [B]](https://patchstorage.com/fomp-lv2-bundle/) | `http://drobilla.net/plugins/fomp/pulse_vco` | Pi64 | WIP | No exact or reviewed equivalent match |
| [fomp.lv2 Bundle — Rec VCO [B]](https://patchstorage.com/fomp-lv2-bundle/) | `http://drobilla.net/plugins/fomp/rec_vco` | Pi64 | WIP | No exact or reviewed equivalent match |
| [fomp.lv2 Bundle — reverb](https://patchstorage.com/fomp-lv2-bundle/) | `http://drobilla.net/plugins/fomp/reverb` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [fomp.lv2 Bundle — reverb-amb [B]](https://patchstorage.com/fomp-lv2-bundle/) | `http://drobilla.net/plugins/fomp/reverb_amb` | Pi64 | WIP | No exact or reviewed equivalent match |
| [fomp.lv2 Bundle — Saw VCO [B]](https://patchstorage.com/fomp-lv2-bundle/) | `http://drobilla.net/plugins/fomp/saw_vco` | Pi64 | WIP | No exact or reviewed equivalent match |
| [fomp.lv2 Bundle — Triple chorus](https://patchstorage.com/fomp-lv2-bundle/) | `http://drobilla.net/plugins/fomp/triple_chorus` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Freakclip](https://patchstorage.com/freakclip/) | `http://faust-lv2.googlecode.com/Freakclip` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Freaktail](https://patchstorage.com/freaktail/) | `http://faust-lv2.googlecode.com/Freaktail` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Gain 2x2](https://patchstorage.com/gain-2x2/) | `http://moddevices.com/plugins/mod-devel/Gain2x2` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Gain Plugin — Gain](https://patchstorage.com/gain-plugin/) | `http://moddevices.com/plugins/mod-devel/Gain` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Galactic](https://patchstorage.com/galactic/) | `https://hannesbraun.net/ns/lv2/airwindows/galactic` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Granulator](https://patchstorage.com/granulator-2/) | `http://faust-lv2.googlecode.com/Granulator` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Gx Studio Preamp Stereo](https://patchstorage.com/gx-studio-preamp-stereo/) | `http://guitarix.sourceforge.net/plugins/gx_studiopre_st#studiopre_st` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [gx_redeye.lv2 Bundle — bigchump](https://patchstorage.com/gx_redeye-lv2-bundle/) | `http://guitarix.sourceforge.net/plugins/gx_redeye#bigchump` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [gx_redeye.lv2 Bundle — chump](https://patchstorage.com/gx_redeye-lv2-bundle/) | `http://guitarix.sourceforge.net/plugins/gx_redeye#chump` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [gx_redeye.lv2 Bundle — GxRedeye Vibro Chump](https://patchstorage.com/gx_redeye-lv2-bundle/) | `http://guitarix.sourceforge.net/plugins/gx_redeye#vibrochump` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxAlembic](https://patchstorage.com/gxalembic/) | `http://guitarix.sourceforge.net/plugins/gx_studiopre#studiopre` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxAmplifier-Stereo-X — GxAmplifier Stereo](https://patchstorage.com/gxamplifier-stereo-x/) | `http://guitarix.sourceforge.net/plugins/gx_amp_stereo#GUITARIX_ST` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxAmplifier-X](https://patchstorage.com/gxamplifier-x/) | `http://guitarix.sourceforge.net/plugins/gx_amp#GUITARIX` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxBaJaTubeDriver](https://patchstorage.com/gxbajatubedriver/) | `http://guitarix.sourceforge.net/plugins/gx_bajatubedriver_#_bajatubedriver_` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [GxBarkGraphicEQ](https://patchstorage.com/gxbarkgraphiceq/) | `http://guitarix.sourceforge.net/plugins/gx_barkgraphiceq_#_barkgraphiceq_` | Pi64 | WIP | No exact or reviewed equivalent match |
| [GxBooster](https://patchstorage.com/gxbooster/) | `http://guitarix.sourceforge.net/plugins/gxbooster#booster` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxBottleRocket](https://patchstorage.com/gxbottlerocket/) | `http://guitarix.sourceforge.net/plugins/gx_bottlerocket_#_bottlerocket_` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [GxCabinet](https://patchstorage.com/gxcabinet/) | `http://guitarix.sourceforge.net/plugins/gx_cabinet#CABINET` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxChorus-Stereo](https://patchstorage.com/gxchorus-stereo/) | `http://guitarix.sourceforge.net/plugins/gx_chorus_stereo#_chorus_stereo` | Pi64 | WIP | No exact or reviewed equivalent match |
| [GxColorSound Tonebender — GxColorSoundTonebender](https://patchstorage.com/gxcolorsound-tonebender/) | `http://guitarix.sourceforge.net/plugins/gx_cstb_#_cstb_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxCompressor](https://patchstorage.com/gxcompressor/) | `http://guitarix.sourceforge.net/plugins/gx_compressor#_compressor` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxCrybabyGCB95](https://patchstorage.com/gxcrybabygcb95/) | `http://guitarix.sourceforge.net/plugins/gx_gcb_95_#_gcb_95_` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [GxDelay-Stereo](https://patchstorage.com/gxdelay-stereo/) | `http://guitarix.sourceforge.net/plugins/gx_delay_stereo#_delay_stereo` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Gxdetune](https://patchstorage.com/gxdetune/) | `http://guitarix.sourceforge.net/plugins/gx_detune_#_detune_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Gxdigital_delay](https://patchstorage.com/gxdigital_delay/) | `http://guitarix.sourceforge.net/plugins/gx_digital_delay_#_digital_delay_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Gxdigital_delay_st](https://patchstorage.com/gxdigital_delay_st/) | `http://guitarix.sourceforge.net/plugins/gx_digital_delay_st_#_digital_delay_st_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Gxduck_delay](https://patchstorage.com/gxduck_delay/) | `http://guitarix.sourceforge.net/plugins/gx_duck_delay_#_duck_delay_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Gxduck_delay_st](https://patchstorage.com/gxduck_delay_st/) | `http://guitarix.sourceforge.net/plugins/gx_duck_delay_st_#_duck_delay_st_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxEcho-Stereo](https://patchstorage.com/gxecho-stereo/) | `http://guitarix.sourceforge.net/plugins/gx_echo_stereo#_echo_stereo` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxEchoCat](https://patchstorage.com/gxechocat/) | `http://guitarix.sourceforge.net/plugins/gxechocat#echocat` | Pi64 | WIP | No exact or reviewed equivalent match |
| [GxEpic](https://patchstorage.com/gxepic/) | `http://guitarix.sourceforge.net/plugins/gx_epic_#_epic_` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [GxExpander](https://patchstorage.com/gxexpander/) | `http://guitarix.sourceforge.net/plugins/gx_expander#_expander` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxFlanger](https://patchstorage.com/gxflanger/) | `http://guitarix.sourceforge.net/plugins/gx_flanger#_flanger` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxFuzz](https://patchstorage.com/gxfuzz/) | `http://guitarix.sourceforge.net/plugins/gx_fuzz_#fuzz_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxFuzzFaceFuller — GxFuzzFaceFullerMod](https://patchstorage.com/gxfuzzfacefuller/) | `http://guitarix.sourceforge.net/plugins/gx_fuzzfacefm_#_fuzzfacefm_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxFuzzFaceJH2 — GxFuzzFaceJH-2](https://patchstorage.com/gxfuzzfacejh2/) | `http://guitarix.sourceforge.net/plugins/gx_fuzzface_#_fuzzface_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxFuzzMaster](https://patchstorage.com/gxfuzzmaster/) | `http://guitarix.sourceforge.net/plugins/gx_fumaster_#_fumaster_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxGraphicEQ](https://patchstorage.com/gxgraphiceq/) | `http://guitarix.sourceforge.net/plugins/gx_graphiceq_#_graphiceq_` | Pi64 | WIP | No exact or reviewed equivalent match |
| [GxGuvnor](https://patchstorage.com/gxguvnor/) | `http://guitarix.sourceforge.net/plugins/gx_guvnor_#_guvnor_` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [GxHF_Brightener — GxHighFrequencyBrightener](https://patchstorage.com/gxhf_brightener/) | `http://guitarix.sourceforge.net/plugins/gx_hfb_#_hfb_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxHogsFoot](https://patchstorage.com/gxhogsfoot/) | `http://guitarix.sourceforge.net/plugins/gx_hogsfoot_#_hogsfoot_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxHornet](https://patchstorage.com/gxhornet/) | `http://guitarix.sourceforge.net/plugins/gx_hornet_#_hornet_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxHotBox](https://patchstorage.com/gxhotbox/) | `http://guitarix.sourceforge.net/plugins/gx_hotbox_#_hotbox_` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [GxHyperion](https://patchstorage.com/gxhyperion/) | `http://guitarix.sourceforge.net/plugins/gx_hyperion_#_hyperion_` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Gxjcm800pre — GxJCM800pre](https://patchstorage.com/gxjcm800pre/) | `http://guitarix.sourceforge.net/plugins/gx_jcm800pre_#_jcm800pre_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Gxjcm800preST — GxJCM800pre ST](https://patchstorage.com/gxjcm800prest/) | `http://guitarix.sourceforge.net/plugins/gx_jcm800pre_st#_jcm800pre_st` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxKnightFuzz](https://patchstorage.com/gxknightfuzz/) | `http://guitarix.sourceforge.net/plugins/gx_KnightFuzz_#_KnightFuzz_` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [GxLiquidDrive](https://patchstorage.com/gxliquiddrive/) | `http://guitarix.sourceforge.net/plugins/gx_liquiddrive_#_liquiddrive_` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Gxlivelooper](https://patchstorage.com/gxlivelooper/) | `http://guitarix.sourceforge.net/plugins/gx_livelooper_#_livelooper_` | Pi64 | WIP | No exact or reviewed equivalent match |
| [GxMole](https://patchstorage.com/gxmole/) | `http://guitarix.sourceforge.net/plugins/gx_mole_#_mole_` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [GxMuff](https://patchstorage.com/gxmuff/) | `http://guitarix.sourceforge.net/plugins/gx_muff_#_muff_` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [GxMultiBandCompressor](https://patchstorage.com/gxmultibandcompressor/) | `http://guitarix.sourceforge.net/plugins/gx_mbcompressor_#_mbcompressor_` | Pi64 | WIP | No exact or reviewed equivalent match |
| [GxMultiBandDelay](https://patchstorage.com/gxmultibanddelay/) | `http://guitarix.sourceforge.net/plugins/gx_mbdelay_#_mbdelay_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxMultiBandDistortion](https://patchstorage.com/gxmultibanddistortion/) | `http://guitarix.sourceforge.net/plugins/gx_mbdistortion_#_mbdistortion_` | Pi64 | WIP | No exact or reviewed equivalent match |
| [GxMultiBandEcho](https://patchstorage.com/gxmultibandecho/) | `http://guitarix.sourceforge.net/plugins/gx_mbecho_#_mbecho_` | Pi64 | WIP | No exact or reviewed equivalent match |
| [GxMultiBandReverb](https://patchstorage.com/gxmultibandreverb/) | `http://guitarix.sourceforge.net/plugins/gx_mbreverb_#_mbreverb_` | Pi64 | WIP | No exact or reviewed equivalent match |
| [Gxoc_2 — GxOC-2](https://patchstorage.com/gxoc_2/) | `http://guitarix.sourceforge.net/plugins/gx_oc_2_#_oc_2_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxPhaser](https://patchstorage.com/gxphaser/) | `http://guitarix.sourceforge.net/plugins/gx_phaser#_phaser` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxQuack](https://patchstorage.com/gxquack/) | `http://guitarix.sourceforge.net/plugins/gx_quack_#_quack_` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [GxRangeMaster — GxRangemaster](https://patchstorage.com/gxrangemaster/) | `http://guitarix.sourceforge.net/plugins/gx_rangem_#_rangem_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxReverb-Stereo](https://patchstorage.com/gxreverb-stereo/) | `http://guitarix.sourceforge.net/plugins/gx_reverb_stereo#_reverb_stereo` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Gxroom_simulator](https://patchstorage.com/gxroom_simulator/) | `http://guitarix.sourceforge.net/plugins/gx_room_simulator_#_room_simulator_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxSaturator](https://patchstorage.com/gxsaturator/) | `http://guitarix.sourceforge.net/plugins/gx_saturate_#_saturate_` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [GxScreamingBird](https://patchstorage.com/gxscreamingbird/) | `http://guitarix.sourceforge.net/plugins/gx_scream_#_scream_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxSD1](https://patchstorage.com/gxsd1/) | `http://guitarix.sourceforge.net/plugins/gx_sd1sim_#_sd1sim_` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Gxshimmizita](https://patchstorage.com/gxshimmizita/) | `http://guitarix.sourceforge.net/plugins/gx_shimmizita_#_shimmizita_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxSlowGear](https://patchstorage.com/gxslowgear/) | `http://guitarix.sourceforge.net/plugins/gx_slowgear_#_slowgear_` | x64, PB32, Pi64 | WIP | Exact Beebo reference |
| [GxSustainer](https://patchstorage.com/gxsustainer/) | `http://guitarix.sourceforge.net/plugins/gx_susta_#_susta_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxSVT](https://patchstorage.com/gxsvt/) | `http://guitarix.sourceforge.net/plugins/gx_ampegsvt_#_ampegsvt_` | x64, PB32, Pi64 | Ready | Exact Beebo reference |
| [Gxswitched_tremolo](https://patchstorage.com/gxswitched_tremolo/) | `http://guitarix.sourceforge.net/plugins/gx_switched_tremolo_#_switched_tremolo_` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxSwitchlessWah](https://patchstorage.com/gxswitchlesswah/) | `http://guitarix.sourceforge.net/plugins/gx_switchless_wah#wah` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxTiltTone](https://patchstorage.com/gxtilttone/) | `http://guitarix.sourceforge.net/plugins/gxtilttone#tilttone` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxTremolo](https://patchstorage.com/gxtremolo/) | `http://guitarix.sourceforge.net/plugins/gx_tremolo#_tremolo` | Pi64 | WIP | No exact or reviewed equivalent match |
| [GxTubeDelay](https://patchstorage.com/gxtubedelay/) | `http://guitarix.sourceforge.net/plugins/gxtubedelay#tubedelay` | Pi64 | WIP | No exact or reviewed equivalent match |
| [GxTubeScreamer](https://patchstorage.com/gxtubescreamer/) | `http://guitarix.sourceforge.net/plugins/gxts9#ts9sim` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [GxTubeTremelo](https://patchstorage.com/gxtubetremelo/) | `http://guitarix.sourceforge.net/plugins/gxtubetremelo#tubetremelo` | Pi64 | WIP | No exact or reviewed equivalent match |
| [GxTubeVibrato](https://patchstorage.com/gxtubevibrato/) | `http://guitarix.sourceforge.net/plugins/gxtubevibrato#tubevibrato` | Pi64 | WIP | No exact or reviewed equivalent match |
| [GxUltraCab](https://patchstorage.com/gxultracab/) | `http://guitarix.sourceforge.net/plugins/gx_ultracab_#_ultracab_` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [GxVBassPreAmp](https://patchstorage.com/gxvbasspreamp/) | `http://guitarix.sourceforge.net/plugins/gx_voxbass_#_voxbass_` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [gxwah.lv2 Bundle — GxAutoWah](https://patchstorage.com/gxautowah/) | `http://guitarix.sourceforge.net/plugins/gxautowah#autowah` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [gxwah.lv2 Bundle — GxWah](https://patchstorage.com/gxautowah/) | `http://guitarix.sourceforge.net/plugins/gxautowah#wah` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [GxWahwah](https://patchstorage.com/gxwahwah/) | `http://guitarix.sourceforge.net/plugins/gx_colwah_#_colwah_` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [GxZita_rev1-Stereo](https://patchstorage.com/gxzita_rev1-stereo/) | `http://guitarix.sourceforge.net/plugins/gx_zita_rev1_stereo#_zita_rev1_stereo` | Pi64 | WIP | No exact or reviewed equivalent match |
| [Hardware Bypass](https://patchstorage.com/hardware-bypass/) | `http://moddevices.com/plugins/mod-devel/HardwareBypass` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Harmless](https://patchstorage.com/harmless/) | `https://github.com/ninodewit/SHIRO-Plugins/plugins/harmless` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Harmonic Exciter](https://patchstorage.com/harmonic-exciter/) | `https://github.com/brummer10/HarmonicExciter` | PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Harmonizer](https://patchstorage.com/harmonizer/) | `http://moddevices.com/plugins/mod-devel/Harmonizer` | x64, Pi64 | WIP | No exact or reviewed equivalent match |
| [Harmonizer2](https://patchstorage.com/harmonizer2/) | `http://moddevices.com/plugins/mod-devel/Harmonizer2` | x64, Pi64 | WIP | No exact or reviewed equivalent match |
| [HarmonizerCS](https://patchstorage.com/harmonizercs/) | `http://moddevices.com/plugins/mod-devel/HarmonizerCS` | x64, Pi64 | WIP | No exact or reviewed equivalent match |
| [Hex Chain Amp](https://patchstorage.com/hex-chain/) | `https://rpowell5064.github.io/guitaramp-suite/amp` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Hex Chain Cabinet](https://patchstorage.com/hex-chain-cabinet/) | `https://rpowell5064.github.io/guitaramp-suite/cab` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Hex Chain Compressor](https://patchstorage.com/hex-chain-compressor/) | `https://rpowell5064.github.io/guitaramp-suite/comp` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Hex Chain Delay](https://patchstorage.com/hex-chain-delay/) | `https://rpowell5064.github.io/guitaramp-suite/delay` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Hex Chain Drive](https://patchstorage.com/hex-chain-drive/) | `https://rpowell5064.github.io/guitaramp-suite/drive` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Hex Chain Fuzz](https://patchstorage.com/hex-chain-fuzz/) | `https://rpowell5064.github.io/guitaramp-suite/fuzz` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Hex Chain Input Trim](https://patchstorage.com/hex-chain-input-trim/) | `https://rpowell5064.github.io/guitaramp-suite/utility` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Hex Chain Modulation](https://patchstorage.com/hex-chain-modulation/) | `https://rpowell5064.github.io/guitaramp-suite/modfx` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Hex Chain Nail](https://patchstorage.com/hex-chain-nail/) | `https://rpowell5064.github.io/guitaramp-suite/nail` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Hex Chain Noise Gate](https://patchstorage.com/hex-chain-noise-gate/) | `https://rpowell5064.github.io/guitaramp-suite/gate` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Hex Chain Octave](https://patchstorage.com/hex-chain-octave/) | `https://rpowell5064.github.io/guitaramp-suite/octave` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Hex Chain Reverb](https://patchstorage.com/hex-chain-reverb/) | `https://rpowell5064.github.io/guitaramp-suite/reverb` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Hex Chain Scratch Pad](https://patchstorage.com/hex-chain-scratch-pad/) | `https://rpowell5064.github.io/guitaramp-suite/practice` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Hex Chain Wah](https://patchstorage.com/hex-chain-wah/) | `https://rpowell5064.github.io/guitaramp-suite/wah` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Hex Forge](https://patchstorage.com/hex-forge/) | `https://rpowell5064.github.io/guitaramp-suite/hexforge` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [HighPassFilter](https://patchstorage.com/highpassfilter/) | `http://moddevices.com/plugins/mod-devel/HighPassFilter` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Instrument Tuner](https://patchstorage.com/instrument-tuner/) | `http://gareus.org/oss/lv2/tuna#mod` | x64, PB32, Pi64 | WIP | Exact source + Beebo |
| [IntamixX LR4](https://patchstorage.com/intamixx-lr4-2/) | `https://github.com/intamixx/IntamixX-LR4` | x64 | Ready | No exact or reviewed equivalent match |
| [invada.lv2 Bundle — Invada Compressor (mono)](https://patchstorage.com/invada-lv2-bundle/) | `http://invadarecords.com/plugins/lv2/compressor/mono` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [invada.lv2 Bundle — Invada Compressor (stereo)](https://patchstorage.com/invada-lv2-bundle/) | `http://invadarecords.com/plugins/lv2/compressor/stereo` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [invada.lv2 Bundle — Invada Delay Munge (mono in)](https://patchstorage.com/invada-lv2-bundle/) | `http://invadarecords.com/plugins/lv2/delay/mono` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [invada.lv2 Bundle — Invada Delay Munge (sum L+R in)](https://patchstorage.com/invada-lv2-bundle/) | `http://invadarecords.com/plugins/lv2/delay/sum` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [invada.lv2 Bundle — Invada Early Reflection Reverb (mono in) [B]](https://patchstorage.com/invada-lv2-bundle/) | `http://invadarecords.com/plugins/lv2/erreverb/mono` | Pi64 | WIP | No exact or reviewed equivalent match |
| [invada.lv2 Bundle — Invada Early Reflection Reverb (sum L+R in) [B]](https://patchstorage.com/invada-lv2-bundle/) | `http://invadarecords.com/plugins/lv2/erreverb/sum` | Pi64 | WIP | No exact or reviewed equivalent match |
| [invada.lv2 Bundle — Invada High Pass Filter (mono) [B]](https://patchstorage.com/invada-lv2-bundle/) | `http://invadarecords.com/plugins/lv2/filter/hpf/mono` | Pi64 | WIP | No exact or reviewed equivalent match |
| [invada.lv2 Bundle — Invada High Pass Filter (stereo) [B]](https://patchstorage.com/invada-lv2-bundle/) | `http://invadarecords.com/plugins/lv2/filter/hpf/stereo` | Pi64 | WIP | No exact or reviewed equivalent match |
| [invada.lv2 Bundle — Invada Low Pass Filter (mono) [B]](https://patchstorage.com/invada-lv2-bundle/) | `http://invadarecords.com/plugins/lv2/filter/lpf/mono` | Pi64 | WIP | No exact or reviewed equivalent match |
| [invada.lv2 Bundle — Invada Low Pass Filter (stereo) [B]](https://patchstorage.com/invada-lv2-bundle/) | `http://invadarecords.com/plugins/lv2/filter/lpf/stereo` | Pi64 | WIP | No exact or reviewed equivalent match |
| [invada.lv2 Bundle — Invada Input Module [B]](https://patchstorage.com/invada-lv2-bundle/) | `http://invadarecords.com/plugins/lv2/input` | Pi64 | WIP | No exact or reviewed equivalent match |
| [invada.lv2 Bundle — Invada Meter [B]](https://patchstorage.com/invada-lv2-bundle/) | `http://invadarecords.com/plugins/lv2/meter` | Pi64 | WIP | No exact or reviewed equivalent match |
| [invada.lv2 Bundle — Invada Stereo Phaser (mono in)](https://patchstorage.com/invada-lv2-bundle/) | `http://invadarecords.com/plugins/lv2/phaser/mono` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [invada.lv2 Bundle — Invada Stereo Phaser (stereo in)](https://patchstorage.com/invada-lv2-bundle/) | `http://invadarecords.com/plugins/lv2/phaser/stereo` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [invada.lv2 Bundle — Invada Stereo Phaser (sum L+R in)](https://patchstorage.com/invada-lv2-bundle/) | `http://invadarecords.com/plugins/lv2/phaser/sum` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [invada.lv2 Bundle — Invada Test Tones [B]](https://patchstorage.com/invada-lv2-bundle/) | `http://invadarecords.com/plugins/lv2/testtone` | Pi64 | WIP | No exact or reviewed equivalent match |
| [invada.lv2 Bundle — Invada Tube Distortion (mono) [B]](https://patchstorage.com/invada-lv2-bundle/) | `http://invadarecords.com/plugins/lv2/tube/mono` | Pi64 | WIP | No exact or reviewed equivalent match |
| [invada.lv2 Bundle — Invada Tube Distortion (stereo) [B]](https://patchstorage.com/invada-lv2-bundle/) | `http://invadarecords.com/plugins/lv2/tube/stereo` | Pi64 | WIP | No exact or reviewed equivalent match |
| [IR loader cabsim](https://patchstorage.com/ir-loader-cabsim/) | `http://moddevices.com/plugins/mod-devel/cabsim-IR-loader` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [JTBassMIDI](https://patchstorage.com/jtbassmidi/) | `https://github.com/jtemple967/JTBassMIDI` | Pi64 | Ready | No exact or reviewed equivalent match |
| [JTBassSynth](https://patchstorage.com/jtbasssynth/) | `https://github.com/jtemple967/JTBassSynth` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Kars Plugin — Kars](https://patchstorage.com/kars-plugin/) | `http://distrho.sf.net/plugins/Kars` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Larynx](https://patchstorage.com/larynx/) | `https://github.com/ninodewit/SHIRO-Plugins/plugins/larynx` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Level Meter](https://patchstorage.com/level-meter/) | `http://gareus.org/oss/lv2/modmeter` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [LowPassFilter](https://patchstorage.com/lowpassfilter/) | `http://moddevices.com/plugins/mod-devel/LowPassFilter` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MaBitcrush](https://patchstorage.com/mabitcrush/) | `http://distrho.sf.net/plugins/MaBitcrush` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Mackity](https://patchstorage.com/mackity/) | `https://hannesbraun.net/ns/lv2/airwindows/mackity` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [MaFreeverb](https://patchstorage.com/mafreeverb/) | `http://distrho.sf.net/plugins/MaFreeverb` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MaGigaverb](https://patchstorage.com/magigaverb/) | `http://distrho.sf.net/plugins/MaGigaverb` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MaPitchshift](https://patchstorage.com/mapitchshift/) | `http://distrho.sf.net/plugins/MaPitchshift` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MDA Ambience](https://patchstorage.com/mda-ambience/) | `http://moddevices.com/plugins/mda/Ambience` | PB32 | WIP | No exact or reviewed equivalent match |
| [MDA Bandisto](https://patchstorage.com/mda-bandisto/) | `http://moddevices.com/plugins/mda/Bandisto` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MDA BeatBox](https://patchstorage.com/mda-beatbox/) | `http://moddevices.com/plugins/mda/BeatBox` | PB32 | WIP | No exact or reviewed equivalent match |
| [MDA Combo](https://patchstorage.com/mda-combo/) | `http://moddevices.com/plugins/mda/Combo` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MDA De-ess](https://patchstorage.com/mda-de-ess/) | `http://moddevices.com/plugins/mda/DeEss` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MDA Degrade](https://patchstorage.com/mda-degrade/) | `http://moddevices.com/plugins/mda/Degrade` | PB32 | WIP | No exact or reviewed equivalent match |
| [MDA Delay](https://patchstorage.com/mda-delay/) | `http://moddevices.com/plugins/mda/Delay` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MDA Detune](https://patchstorage.com/mda-detune/) | `http://moddevices.com/plugins/mda/Detune` | PB32 | WIP | No exact or reviewed equivalent match |
| [MDA Dither](https://patchstorage.com/mda-dither/) | `http://moddevices.com/plugins/mda/Dither` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MDA DubDelay](https://patchstorage.com/mda-dubdelay/) | `http://moddevices.com/plugins/mda/DubDelay` | PB32 | WIP | No exact or reviewed equivalent match |
| [MDA DX10](https://patchstorage.com/mda-dx10/) | `http://moddevices.com/plugins/mda/DX10` | PB32 | WIP | No exact or reviewed equivalent match |
| [MDA Dynamics](https://patchstorage.com/mda-dynamics/) | `http://moddevices.com/plugins/mda/Dynamics` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MDA ePiano](https://patchstorage.com/mda-epiano/) | `http://moddevices.com/plugins/mda/EPiano` | PB32 | WIP | No exact or reviewed equivalent match |
| [MDA Image](https://patchstorage.com/mda-image/) | `http://moddevices.com/plugins/mda/Image` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MDA JX10](https://patchstorage.com/mda-jx10/) | `http://moddevices.com/plugins/mda/JX10` | PB32 | WIP | No exact or reviewed equivalent match |
| [MDA Leslie](https://patchstorage.com/mda-leslie/) | `http://moddevices.com/plugins/mda/Leslie` | PB32 | WIP | No exact or reviewed equivalent match |
| [MDA Limiter](https://patchstorage.com/mda-limiter/) | `http://moddevices.com/plugins/mda/Limiter` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MDA Loudness](https://patchstorage.com/mda-loudness/) | `http://moddevices.com/plugins/mda/Loudness` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MDA MultiBand](https://patchstorage.com/mda-multiband/) | `http://moddevices.com/plugins/mda/MultiBand` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MDA Overdrive](https://patchstorage.com/mda-overdrive/) | `http://moddevices.com/plugins/mda/Overdrive` | PB32 | WIP | No exact or reviewed equivalent match |
| [MDA Piano](https://patchstorage.com/mda-piano/) | `http://moddevices.com/plugins/mda/Piano` | PB32 | WIP | No exact or reviewed equivalent match |
| [MDA RePsycho!](https://patchstorage.com/mda-repsycho/) | `http://moddevices.com/plugins/mda/RePsycho` | PB32 | WIP | No exact or reviewed equivalent match |
| [MDA RezFilter](https://patchstorage.com/mda-rezfilter/) | `http://moddevices.com/plugins/mda/RezFilter` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MDA RingMod](https://patchstorage.com/mda-ringmod/) | `http://moddevices.com/plugins/mda/RingMod` | PB32 | WIP | No exact or reviewed equivalent match |
| [MDA RoundPan](https://patchstorage.com/mda-roundpan/) | `http://moddevices.com/plugins/mda/RoundPan` | PB32 | WIP | No exact or reviewed equivalent match |
| [MDA Shepard](https://patchstorage.com/mda-shepard/) | `http://moddevices.com/plugins/mda/Shepard` | PB32 | WIP | No exact or reviewed equivalent match |
| [MDA Splitter](https://patchstorage.com/mda-splitter/) | `http://moddevices.com/plugins/mda/Splitter` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MDA Stereo](https://patchstorage.com/mda-stereo/) | `http://moddevices.com/plugins/mda/Stereo` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MDA SubSynth](https://patchstorage.com/mda-subsynth/) | `http://moddevices.com/plugins/mda/SubSynth` | PB32 | WIP | No exact or reviewed equivalent match |
| [MDA TalkBox](https://patchstorage.com/mda-talkbox/) | `http://moddevices.com/plugins/mda/TalkBox` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MDA TestTone](https://patchstorage.com/mda-testtone/) | `http://moddevices.com/plugins/mda/TestTone` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MDA ThruZero](https://patchstorage.com/mda-thruzero/) | `http://moddevices.com/plugins/mda/ThruZero` | PB32 | WIP | No exact or reviewed equivalent match |
| [MDA Tracker](https://patchstorage.com/mda-tracker/) | `http://moddevices.com/plugins/mda/Tracker` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MDA Transient](https://patchstorage.com/mda-transient/) | `http://moddevices.com/plugins/mda/Transient` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MDA VocInput](https://patchstorage.com/mda-vocinput/) | `http://moddevices.com/plugins/mda/VocInput` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MDA Vocoder](https://patchstorage.com/mda-vocoder/) | `http://moddevices.com/plugins/mda/Vocoder` | PB32 | WIP | No exact or reviewed equivalent match |
| [MetalTone](https://patchstorage.com/metaltone/) | `https://github.com/brummer10/MetalTone` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MIDI Clock Generator](https://patchstorage.com/midi-clock-generator/) | `http://gareus.org/oss/lv2/mclk` | x64, PB32, Pi64 | WIP | Exact Beebo reference |
| [MIDI display](https://patchstorage.com/midi-display/) | `https://github.com/vallsv/midi-display.lv2` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [MIDI Generator](https://patchstorage.com/midi-generator/) | `http://gareus.org/oss/lv2/midigen` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MIDI Step Sequencer16x8](https://patchstorage.com/midi-step-sequencer16x8/) | `http://gareus.org/oss/lv2/stepseq#s16n8` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MIDI Step Sequencer32x8](https://patchstorage.com/midi-step-sequencer32x8/) | `http://gareus.org/oss/lv2/stepseq#s32n8` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MIDI Step Sequencer8x16](https://patchstorage.com/midi-step-sequencer8x16/) | `http://gareus.org/oss/lv2/stepseq#s8n16` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MIDI Step Sequencer8x4](https://patchstorage.com/midi-step-sequencer8x4/) | `http://gareus.org/oss/lv2/stepseq#s8n4` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MIDI Step Sequencer8x8](https://patchstorage.com/midi-step-sequencer8x8/) | `http://gareus.org/oss/lv2/stepseq#s8n8` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MIDI Timecode (MTC) Generator](https://patchstorage.com/midi-timecode-mtc-generator/) | `http://gareus.org/oss/lv2/mtc` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MidSide](https://patchstorage.com/midside/) | `https://hannesbraun.net/ns/lv2/airwindows/midside` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Mixer](https://patchstorage.com/mixer/) | `http://moddevices.com/plugins/mod-devel/mixer` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Mixer Stereo](https://patchstorage.com/mixer-stereo/) | `http://moddevices.com/plugins/mod-devel/mixer-stereo` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [mod-ams.lv2 Bundle — AMS Envelope](https://patchstorage.com/mod-ams-lv2-bundle/) | `http://github.com/blablack/ams-lv2/env` | x64, PB32, Pi64 | Ready | AMS lineage counterpart in Poly |
| [mod-ams.lv2 Bundle — AMS LFO2 - Freq](https://patchstorage.com/mod-ams-lv2-bundle/) | `http://github.com/blablack/ams-lv2/lfo2_freq` | x64, PB32, Pi64 | Ready | AMS lineage counterpart in Poly |
| [mod-ams.lv2 Bundle — AMS VCA Exp](https://patchstorage.com/mod-ams-lv2-bundle/) | `http://github.com/blablack/ams-lv2/vcaexp` | x64, PB32, Pi64 | Ready | AMS lineage counterpart in Poly |
| [mod-ams.lv2 Bundle — AMS VCA Lin](https://patchstorage.com/mod-ams-lv2-bundle/) | `http://github.com/blablack/ams-lv2/vcalin` | x64, PB32, Pi64 | Ready | AMS lineage counterpart in Poly |
| [mod-ams.lv2 Bundle — AMS VCF](https://patchstorage.com/mod-ams-lv2-bundle/) | `http://github.com/blablack/ams-lv2/vcf` | x64, PB32, Pi64 | Ready | AMS lineage counterpart in Poly |
| [mod-ams.lv2 Bundle — AMS VCO3](https://patchstorage.com/mod-ams-lv2-bundle/) | `http://github.com/blablack/ams-lv2/vco3` | x64, PB32, Pi64 | Ready | AMS lineage counterpart in Poly |
| [Modulay](https://patchstorage.com/modulay/) | `https://github.com/ninodewit/SHIRO-Plugins/plugins/modulay` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Mojo Plugin — Mojo](https://patchstorage.com/mojo-plugin/) | `https://hannesbraun.net/ns/lv2/airwindows/mojo` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Molot Lite Mono](https://patchstorage.com/molot-lite-mono/) | `http://github.com/bernhardrusch/molot-lite-mono-lv2` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [mud Plugin — mud](https://patchstorage.com/mud-plugin/) | `http://remaincalm.org/plugins/mud` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Multi Button to CV](https://patchstorage.com/multi-button-to-cv/) | `http://moddevices.com/plugins/mod-devel/mod-button-to-cv` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [MVerb](https://patchstorage.com/mverb/) | `http://distrho.sf.net/plugins/MVerb` | x64, PB32, Pi64 | WIP | Exact Beebo reference |
| [Nekobi](https://patchstorage.com/nekobi/) | `http://distrho.sf.net/plugins/Nekobi` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Neural Amp Modeler](https://patchstorage.com/neural-amp-modeler/) | `http://github.com/mikeoliphant/neural-amp-modeler-lv2` | PB32, Pi64 | Ready | Exact source + Beebo |
| [Neural Record](https://patchstorage.com/neural-record/) | `urn:brummer10:neuralrecord` | PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Nikola](https://patchstorage.com/nikola/) | `https://hannesbraun.net/ns/lv2/airwindows/nikola` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [No Delay Line](https://patchstorage.com/no-delay-line/) | `http://gareus.org/oss/lv2/nodelay` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Noise Gate](https://patchstorage.com/noise-gate/) | `http://moddevices.com/plugins/mod-devel/System-NoiseGate` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Noise Gate Advanced](https://patchstorage.com/noise-gate-advanced/) | `http://moddevices.com/plugins/mod-devel/Advanced-NoiseGate` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Open Big Muff](https://patchstorage.com/open-big-muff/) | `http://moddevices.com/plugins/mod-devel/BigMuffPi` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [paranoia](https://patchstorage.com/paranoia/) | `http://remaincalm.org/plugins/paranoia` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Parkinsound Step Gate](https://patchstorage.com/parkinsound-step-gate/) | `https://github.com/pilali/parkinsound/lv2/stepgate` | x64, Pi64 | Ready | No exact or reviewed equivalent match |
| [Parkinsound Step Gate 4](https://patchstorage.com/parkinsound-step-gate-4/) | `https://github.com/pilali/parkinsound/lv2/stepgate4` | x64, Pi64 | Ready | No exact or reviewed equivalent match |
| [Ping Pong Pan](https://patchstorage.com/ping-pong-pan/) | `http://distrho.sf.net/plugins/PingPongPan` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Pitchotto](https://patchstorage.com/pitchotto/) | `https://github.com/ninodewit/SHIRO-Plugins/plugins/pitchotto` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [PocketVerbs](https://patchstorage.com/pocketverbs/) | `https://hannesbraun.net/ns/lv2/airwindows/pocketverbs` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [portal.lv2 Bundle — Portal Sink](https://patchstorage.com/portal-lv2-bundle/) | `https://falktx.com/plugins/portal#sink` | PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [portal.lv2 Bundle — Portal Source](https://patchstorage.com/portal-lv2-bundle/) | `https://falktx.com/plugins/portal#source` | PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [PowerAmpImpulses](https://patchstorage.com/powerampimpulses/) | `urn:brummer:PowerAmpImpulses` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [PowerAmpTubes](https://patchstorage.com/poweramptubes/) | `urn:brummer:poweramps` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [PreAmpImpulses](https://patchstorage.com/preampimpulses/) | `urn:brummer:PreAmpImpulses` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [PreAmpTubes](https://patchstorage.com/preamptubes/) | `urn:brummer:PreAmps` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Prefreak](https://patchstorage.com/prefreak/) | `http://faust-lv2.googlecode.com/Prefreak` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Pressure5](https://patchstorage.com/pressure5/) | `https://hannesbraun.net/ns/lv2/airwindows/pressure5` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Random Generator](https://patchstorage.com/random-generator/) | `http://moddevices.com/plugins/mod-devel/mod-cv-random` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Red Zeppelin 4](https://patchstorage.com/red-zeppelin-4/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_Red_Zeppelin_4` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Red Zeppelin 5](https://patchstorage.com/red-zeppelin-5/) | `http://kxstudio.linuxaudio.org/plugins/FluidPlug_Red_Zeppelin_5` | Pi64 | Ready | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Arpie [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#Arpie` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Coil Crafter [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#CoilCrafter` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr CompBand [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#CompBand` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr DistBand [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#DistBand` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Dual Flange [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#Dual_Flange` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Echotron [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#Echotron` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Echoverse [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#Echoverse` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Exciter [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#Exciter` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Expander [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#Expander` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Infinity [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#Infinity` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr MuTroMojo [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#MuTroMojo` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr OpticalTrem [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#Otrem` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Reverbtron [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#Reverbtron` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Ring [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#Ring` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Sequence [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#Sequence` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Shelf Boost [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#ShelfBoost` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Shifter [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#Shifter` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Shuffle [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#Shuffle` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr StereoHarmonizer (no midi) [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#StereoHarm_no_mid` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr StompBox [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#StompBox` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — Fuzz](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#StompBox_fuzz` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Sustainer [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#Sustainer` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Synthfilter [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#Synthfilter` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Valve [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#Valve` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr VaryBand [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#VaryBand` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Vibe [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#Vibe` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Vocoder [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#Vocoder` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Analog Phaser [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#aphas` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr AlienWah [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#awha` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Cabinet [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#cabe` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Flanger/Chorus](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#chor` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Compressor [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#comp` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Musical Delay [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#delm` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Derelict [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#dere` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Distortion](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#dist` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Echo [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#eco` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr EQ [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#eql` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Parametric EQ](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#eqp` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Harmonizer [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#har` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Harmonizer (no midi) [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#har_no_mid` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Pan [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#pan` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr Reverb](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#reve` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [rkr.lv2 Bundle — rkr WahWah [B]](https://patchstorage.com/rkr-lv2-bundle/) | `http://rakarrack.sourceforge.net/effects.html#wha` | Pi64 | WIP | No exact or reviewed equivalent match |
| [rubberband.lv2 Bundle — Rubberband (Mono)](https://patchstorage.com/rubberband-lv2-bundle/) | `http://www.breakfastquay.com/rubberband#Mono` | Pi64 | Ready | No exact or reviewed equivalent match |
| [rubberband.lv2 Bundle — Rubberband (Stereo)](https://patchstorage.com/rubberband-lv2-bundle/) | `http://www.breakfastquay.com/rubberband#Stereo` | Pi64 | Ready | No exact or reviewed equivalent match |
| [Rumor](https://patchstorage.com/rumor/) | `https://github.com/brummer10/Rumor` | PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Rézonateur](https://patchstorage.com/rezonateur/) | `http://jpcima.sdf1.org/lv2/rezonateur` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [samplv1](https://patchstorage.com/samplv1/) | `http://samplv1.sourceforge.net/lv2` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [sc_record.lv2 Bundle — Record-Mono](https://patchstorage.com/sc_record-lv2-bundle/) | `https://github.com/brummer10/screcord#mono_record` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [sc_record.lv2 Bundle — Record-Quad](https://patchstorage.com/sc_record-lv2-bundle/) | `https://github.com/brummer10/screcord#quad_record` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [sc_record.lv2 Bundle — Record-Stereo](https://patchstorage.com/sc_record-lv2-bundle/) | `https://github.com/brummer10/screcord#stereo_record` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Schrammel OJD](https://patchstorage.com/schrammel-ojd/) | `https://github.com/JanosGit/Schrammel_OJD` | PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [setBfree DSP Tonewheel Organ](https://patchstorage.com/setbfree-dsp-tonewheel-organ/) | `http://gareus.org/oss/lv2/b_synth` | PB32 | Ready | No exact or reviewed equivalent match |
| [setBfree MIDI controller](https://patchstorage.com/setbfree-midi-controller/) | `https://github.com/vallsv/setbfree-controller` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [setBfree Organ Overdrive](https://patchstorage.com/setbfree-organ-overdrive/) | `http://gareus.org/oss/lv2/b_overdrive` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [setBfree Organ Reverb](https://patchstorage.com/setbfree-organ-reverb/) | `http://gareus.org/oss/lv2/b_reverb` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [setBfree Whirl Speaker (Old version)](https://patchstorage.com/setbfree-whirl-speaker-old-version/) | `http://gareus.org/oss/lv2/b_whirl#simple` | PB32 | Ready | Exact Beebo reference |
| [Shiroverb](https://patchstorage.com/shiroverb/) | `https://github.com/ninodewit/SHIRO-Plugins/plugins/shiroverb` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Sidepass](https://patchstorage.com/sidepass/) | `https://hannesbraun.net/ns/lv2/airwindows/sidepass` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [SimSam](https://patchstorage.com/simsam/) | `https://gitlab.com/edwillys/simsam/` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [SooperLooper](https://patchstorage.com/sooperlooper/) | `http://moddevices.com/plugins/sooperlooper` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Soul Force](https://patchstorage.com/soul-force/) | `http://www.niallmoody.com/ndcplugs/soulforce.htm` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Spectrum Analyzer](https://patchstorage.com/spectrum-analyzer/) | `http://gareus.org/oss/lv2/modspectre` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Spiral](https://patchstorage.com/spiral-2/) | `https://hannesbraun.net/ns/lv2/airwindows/spiral` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [StarChild](https://patchstorage.com/starchild/) | `https://hannesbraun.net/ns/lv2/airwindows/starchild` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Stereo Balance Control](https://patchstorage.com/stereo-balance-control/) | `http://gareus.org/oss/lv2/balance` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [Stereo X-Fade](https://patchstorage.com/stereo-x-fade/) | `http://gareus.org/oss/lv2/xfade` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [stuck.lv2 Bundle — the infamous stuck](https://patchstorage.com/stuck-lv2-bundle/) | `http://ssj71.github.io/infamousPlugins/plugs.html#stuck` | x64, PB32, Pi64 | WIP | Exact Beebo reference |
| [stuck.lv2 Bundle — the infamous stuck stacker](https://patchstorage.com/stuck-lv2-bundle/) | `http://ssj71.github.io/infamousPlugins/plugs.html#stuckstacker` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Super Capo](https://patchstorage.com/super-capo/) | `http://moddevices.com/plugins/mod-devel/SuperCapo` | x64, Pi64 | WIP | No exact or reviewed equivalent match |
| [Super Whammy](https://patchstorage.com/super-whammy/) | `http://moddevices.com/plugins/mod-devel/SuperWhammy` | x64, Pi64 | WIP | No exact or reviewed equivalent match |
| [Switchbox 1-2](https://patchstorage.com/switchbox-1-2/) | `http://moddevices.com/plugins/mod-devel/SwitchBox2` | PB32 | Ready | No exact or reviewed equivalent match |
| [SwitchTrigger4](https://patchstorage.com/switchtrigger4/) | `http://moddevices.com/plugins/mod-devel/SwitchTrigger4` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [TAP AutoPanner](https://patchstorage.com/tap-autopanner/) | `http://moddevices.com/plugins/tap/autopan` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [TAP Chorus/Flanger](https://patchstorage.com/tap-chorus-flanger/) | `http://moddevices.com/plugins/tap/chorusflanger` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [TAP DeEsser](https://patchstorage.com/tap-deesser/) | `http://moddevices.com/plugins/tap/deesser` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [TAP Equalizer](https://patchstorage.com/tap-equalizer/) | `http://moddevices.com/plugins/tap/eq` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [TAP Equalizer/BW](https://patchstorage.com/tap-equalizer-bw/) | `http://moddevices.com/plugins/tap/eqbw` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [TAP Fractal Doubler](https://patchstorage.com/tap-fractal-doubler/) | `http://moddevices.com/plugins/tap/doubler` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [TAP Mono Dynamics](https://patchstorage.com/tap-mono-dynamics/) | `http://moddevices.com/plugins/tap/dynamics` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [TAP Pink/Fractal Noise](https://patchstorage.com/tap-pink-fractal-noise/) | `http://moddevices.com/plugins/tap/pinknoise` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [TAP Pitch Shifter](https://patchstorage.com/tap-pitch-shifter/) | `http://moddevices.com/plugins/tap/pitch` | x64, PB32, Pi64 | WIP | Exact Beebo reference |
| [TAP Reflector](https://patchstorage.com/tap-reflector/) | `http://moddevices.com/plugins/tap/reflector` | x64, PB32, Pi64 | WIP | Exact Beebo reference |
| [TAP Reverberator](https://patchstorage.com/tap-reverberator/) | `http://moddevices.com/plugins/tap/reverb` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [TAP Rotary Speaker](https://patchstorage.com/tap-rotary-speaker/) | `http://moddevices.com/plugins/tap/rotspeak` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [TAP Scaling Limiter](https://patchstorage.com/tap-scaling-limiter/) | `http://moddevices.com/plugins/tap/limiter` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [TAP Sigmoid Booster](https://patchstorage.com/tap-sigmoid-booster/) | `http://moddevices.com/plugins/tap/sigmoid` | x64, PB32, Pi64 | WIP | Exact Beebo reference |
| [TAP Stereo Dynamics](https://patchstorage.com/tap-stereo-dynamics/) | `http://moddevices.com/plugins/tap/dynamics-st` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [TAP Stereo Echo](https://patchstorage.com/tap-stereo-echo/) | `http://moddevices.com/plugins/tap/echo` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [TAP Tremolo](https://patchstorage.com/tap-tremolo/) | `http://moddevices.com/plugins/tap/tremolo` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [TAP Tubewarmth](https://patchstorage.com/tap-tubewarmth/) | `http://moddevices.com/plugins/tap/tubewarmth` | x64, PB32, Pi64 | WIP | Exact Beebo reference |
| [TAP Vibrato](https://patchstorage.com/tap-vibrato/) | `http://moddevices.com/plugins/tap/vibrato` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [the infamous ewham](https://patchstorage.com/the-infamous-ewham/) | `http://ssj71.github.io/infamousPlugins/plugs.html#ewham` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [the infamous Hip2B](https://patchstorage.com/the-infamous-hip2b/) | `http://ssj71.github.io/infamousPlugins/plugs.html#hip2b` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [the infamous mindi](https://patchstorage.com/the-infamous-mindi/) | `http://ssj71.github.io/infamousPlugins/plugs.html#mindi` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [the infamous power cut](https://patchstorage.com/the-infamous-power-cut/) | `http://ssj71.github.io/infamousPlugins/plugs.html#powercut` | x64, PB32, Pi64 | WIP | Exact Beebo reference |
| [tinygain.lv2 Bundle — TinyGain Mono](https://patchstorage.com/tinygain-lv2-bundle/) | `http://gareus.org/oss/lv2/tinygain#mono` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [tinygain.lv2 Bundle — TinyGain Stereo](https://patchstorage.com/tinygain-lv2-bundle/) | `http://gareus.org/oss/lv2/tinygain#stereo` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [ToggleSwitch](https://patchstorage.com/toggleswitch/) | `http://moddevices.com/plugins/mod-devel/ToggleSwitch4` | x64, PB32, Pi64 | WIP | No exact or reviewed equivalent match |
| [Triceratops](https://patchstorage.com/triceratops/) | `http://nickbailey.co.nr/triceratops` | Pi64 | WIP | No exact or reviewed equivalent match |
| [Vibrato](https://patchstorage.com/vibrato-2/) | `https://hannesbraun.net/ns/lv2/airwindows/vibrato` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [x42 MIDI Filter Collection — MIDI CC to Note](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#cctonote` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Channel Filter](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#channelfilter` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Channel Map](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#channelmap` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Enforce Scale](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#enforcescale` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Event Filter](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#eventblocker` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Key-Range Filter](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#keyrange` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Keysplit](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#keysplit` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI CC Map](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#mapcc` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Note/Channel Map](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#mapkeychannel` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Note Transpose](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#mapkeyscale` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Chord](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#midichord` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Delayline](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#mididelay` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Channel Unisono](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#mididup` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Strum](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#midistrum` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Chromatic Transpose](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#miditranspose` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Monophonic Legato](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#monolegato` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Remove Active Sensing](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#noactivesensing` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Duplicate Blocker](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#nodup` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Note to CC](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#notetocc` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Note Toggle](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#notetoggle` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI N-Tap Delay](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#ntapdelay` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Simple Channel Filter](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#onechannelfilter` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Quantization](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#quantize` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Velocity Randomization](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#randvelocity` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Scale CC Value](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#scalecc` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Sostenuto](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#sostenuto` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Velocity-Range Filter](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#velocityrange` | Pi64 | Ready | Exact Beebo reference |
| [x42 MIDI Filter Collection — MIDI Velocity Adjust](https://patchstorage.com/midifilter-lv2-bundle/) | `http://gareus.org/oss/lv2/midifilter#velocityscale` | Pi64 | Ready | Exact Beebo reference |
| [ZamAutoSat](https://patchstorage.com/zamautosat/) | `urn:zamaudio:ZamAutoSat` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [ZaMaximX2](https://patchstorage.com/zamaximx2/) | `urn:zamaudio:ZaMaximX2` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [ZamComp](https://patchstorage.com/zamcomp/) | `urn:zamaudio:ZamComp` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [ZamCompX2](https://patchstorage.com/zamcompx2/) | `urn:zamaudio:ZamCompX2` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [ZamDelay](https://patchstorage.com/zamdelay/) | `urn:zamaudio:ZamDelay` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [ZamEQ2](https://patchstorage.com/zameq2/) | `urn:zamaudio:ZamEQ2` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [ZamGate](https://patchstorage.com/zamgate/) | `urn:zamaudio:ZamGate` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [ZamGateX2](https://patchstorage.com/zamgatex2/) | `urn:zamaudio:ZamGateX2` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [ZamGEQ31](https://patchstorage.com/zamgeq31/) | `urn:zamaudio:ZamGEQ31` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [ZamHeadX2](https://patchstorage.com/zamheadx2/) | `urn:zamaudio:ZamHeadX2` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [ZamTube](https://patchstorage.com/zamtube/) | `urn:zamaudio:ZamTube` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [ZaMultiComp](https://patchstorage.com/zamulticomp/) | `urn:zamaudio:ZaMultiComp` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [ZaMultiCompX2](https://patchstorage.com/zamulticompx2/) | `urn:zamaudio:ZaMultiCompX2` | x64, PB32, Pi64 | Ready | No exact or reviewed equivalent match |
| [zeroconvo.lv2 Bundle — x42 - IR Convolver Mono](https://patchstorage.com/zeroconvo-lv2-bundle/) | `http://gareus.org/oss/lv2/zeroconvolv#CfgMono` | x64, PB32, Pi64 | WIP | Exact Beebo reference |
| [zeroconvo.lv2 Bundle — x42 - IR Convolver Stereo](https://patchstorage.com/zeroconvo-lv2-bundle/) | `http://gareus.org/oss/lv2/zeroconvolv#CfgStereo` | x64, PB32, Pi64 | WIP | Exact Beebo reference |

## Named bundle inspection

The archive inventory comes from one selected target for each of all 385 entries. This table summarizes the 18 entries that explicitly advertise multiple IDs. Website ID lists and archive manifests sometimes disagree. Extra members are included above and their target is limited to the inspected build.

| Bundle | Advertised IDs | IDs in inspected archive | Additional IDs | Inspected build |
| --- | --- | --- | --- | --- |
| [artyfx.lv2 Bundle](https://patchstorage.com/artyfx-lv2-bundle/) | 3 | 4 | 1 | Pi64 |
| [calf.lv2 Bundle](https://patchstorage.com/calf-lv2-bundle/) | 4 | 45 | 41 | PB32 |
| [carla-audiogain.lv2 Bundle](https://patchstorage.com/carla-audiogain-lv2-bundle/) | 2 | 2 | 0 | Pi64 |
| [carla-files.lv2 Bundle](https://patchstorage.com/carla-files-lv2-bundle/) | 2 | 2 | 0 | Pi64 |
| [fil4.lv2 Bundle](https://patchstorage.com/fil4-lv2-bundle/) | 2 | 2 | 0 | Pi64 |
| [fomp.lv2 Bundle](https://patchstorage.com/fomp-lv2-bundle/) | 6 | 17 | 11 | Pi64 |
| [gx_redeye.lv2 Bundle](https://patchstorage.com/gx_redeye-lv2-bundle/) | 3 | 1 | 0 | Pi64 |
| [gxwah.lv2 Bundle](https://patchstorage.com/gxautowah/) | 2 | 2 | 0 | Pi64 |
| [invada.lv2 Bundle](https://patchstorage.com/invada-lv2-bundle/) | 7 | 18 | 11 | Pi64 |
| [mod-ams.lv2 Bundle](https://patchstorage.com/mod-ams-lv2-bundle/) | 6 | 6 | 0 | Pi64 |
| [portal.lv2 Bundle](https://patchstorage.com/portal-lv2-bundle/) | 2 | 2 | 0 | Pi64 |
| [rkr.lv2 Bundle](https://patchstorage.com/rkr-lv2-bundle/) | 5 | 43 | 38 | Pi64 |
| [rubberband.lv2 Bundle](https://patchstorage.com/rubberband-lv2-bundle/) | 2 | 2 | 0 | Pi64 |
| [sc_record.lv2 Bundle](https://patchstorage.com/sc_record-lv2-bundle/) | 3 | 3 | 0 | Pi64 |
| [stuck.lv2 Bundle](https://patchstorage.com/stuck-lv2-bundle/) | 2 | 2 | 0 | Pi64 |
| [tinygain.lv2 Bundle](https://patchstorage.com/tinygain-lv2-bundle/) | 2 | 2 | 0 | Pi64 |
| [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) | 28 | 28 | 0 | Pi64 |
| [zeroconvo.lv2 Bundle](https://patchstorage.com/zeroconvo-lv2-bundle/) | 2 | 2 | 0 | Pi64 |

## Complete Poly implementation catalogue — 123 IDs

This section includes metadata declarations and build-generated identities. Availability shows exact IDs, verified variants, functional counterparts and partial relationships. “Equivalent not established” means no verified counterpart has been identified, rather than proven absence. Names are preserved from upstream metadata, with explanatory labels for known duplicated names. Source links point to the inspected commit.

### avwlv2 — 46 IDs

| Plugin | Exact ID | Source evidence | Patchstorage counterpart / Beebo mapping |
| --- | --- | --- | --- |
| Amplitude Modifier | `http://avwlv2.sourceforge.net/plugins/avw/amplitude` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/amplitude.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| AMS Advanced Envelope | `http://avwlv2.sourceforge.net/plugins/avw/advenv` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/advenv.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| AMS Amp | `http://avwlv2.sourceforge.net/plugins/avw/amp` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/amp.ttl) | Partial functional overlap: [Attenuverter Booster — Attenuverter Booster](https://patchstorage.com/attenuverter-booster/); No exact Beebo mapping; related family mapping: `attenuverter`, `boost`, `product` |
| AMS Analog Driver | `http://avwlv2.sourceforge.net/plugins/avw/ad` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/ad.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| AMS CV Source | `http://avwlv2.sourceforge.net/plugins/avw/cvs` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/cvs.ttl) | Partial functional overlap: [Control to CV — Control to CV](https://patchstorage.com/control-to-cv/); No exact Beebo mapping |
| AMS Delay | `http://avwlv2.sourceforge.net/plugins/avw/delay` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/delay.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| AMS Envelope | `http://avwlv2.sourceforge.net/plugins/avw/env` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/env.ttl) | AMS lineage variant: [mod-ams.lv2 Bundle — AMS Envelope](https://patchstorage.com/mod-ams-lv2-bundle/); No exact Beebo mapping |
| AMS Inv | `http://avwlv2.sourceforge.net/plugins/avw/inv` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/inv.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| AMS LFO - Freq | `http://avwlv2.sourceforge.net/plugins/avw/lfo_freq` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/lfo_freq.ttl) | AMS lineage variant: [mod-ams.lv2 Bundle — AMS LFO2 - Freq](https://patchstorage.com/mod-ams-lv2-bundle/); No exact Beebo mapping; related family mapping: `lfo` |
| AMS LFO - Tempo | `http://avwlv2.sourceforge.net/plugins/avw/lfo_tempo` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/lfo_tempo.ttl) | Related variant: [mod-ams.lv2 Bundle — AMS LFO2 - Freq](https://patchstorage.com/mod-ams-lv2-bundle/); Exact Beebo mapping: `lfo` |
| AMS Mixer 2 channels - Audio | `http://avwlv2.sourceforge.net/plugins/avw/mixer_2ch_audio` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/mixer_2ch_audio.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| AMS Mixer 2 channels - CV | `http://avwlv2.sourceforge.net/plugins/avw/mixer_2ch_cv` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/mixer_2ch_cv.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| AMS Mixer 4 channels - Audio | `http://avwlv2.sourceforge.net/plugins/avw/mixer_4ch_audio` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/mixer_4ch_audio.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| AMS Mixer 4 channels - CV | `http://avwlv2.sourceforge.net/plugins/avw/mixer_4ch_cv` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/mixer_4ch_cv.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| AMS Noise2 - Audio | `http://avwlv2.sourceforge.net/plugins/avw/noise2_audio` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/noise2_audio.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| AMS Noise2 - CV | `http://avwlv2.sourceforge.net/plugins/avw/noise2_cv` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/noise2_cv.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| AMS Percussive Envelope | `http://avwlv2.sourceforge.net/plugins/avw/percussiveenv` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/percussiveenv.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| AMS Ring Modulator - Audio | `http://avwlv2.sourceforge.net/plugins/avw/ringmodulator_audio` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/ringmodulator_audio.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| AMS Ring Modulator - CV | `http://avwlv2.sourceforge.net/plugins/avw/ringmodulator_cv` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/ringmodulator_cv.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| AMS Sample&Hold | `http://avwlv2.sourceforge.net/plugins/avw/samplehold` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/samplehold.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `sample_hold` |
| AMS Slew Limiter | `http://avwlv2.sourceforge.net/plugins/avw/slew` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/slew.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `slew_limiter` |
| AMS VC Panning | `http://avwlv2.sourceforge.net/plugins/avw/vcpanning` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/vcpanning.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `pan` |
| AMS VC Switch - Audio | `http://avwlv2.sourceforge.net/plugins/avw/vcswitch_audio` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/vcswitch_audio.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| AMS VC Switch - CV | `http://avwlv2.sourceforge.net/plugins/avw/vcswitch_cv` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/vcswitch_cv.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| AMS VCA Exp - Audio | `http://avwlv2.sourceforge.net/plugins/avw/vcaexp_audio` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/vcaexp_audio.ttl) | AMS lineage variant: [mod-ams.lv2 Bundle — AMS VCA Exp](https://patchstorage.com/mod-ams-lv2-bundle/); Exact Beebo mapping: `mix_vca` |
| AMS VCA Exp - CV | `http://avwlv2.sourceforge.net/plugins/avw/vcaexp_cv` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/vcaexp_cv.ttl) | AMS lineage variant: [mod-ams.lv2 Bundle — AMS VCA Exp](https://patchstorage.com/mod-ams-lv2-bundle/); No exact Beebo mapping; related family mapping: `mix_vca` |
| AMS VCA Lin - Audio | `http://avwlv2.sourceforge.net/plugins/avw/vcalin_audio` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/vcalin_audio.ttl) | AMS lineage variant: [mod-ams.lv2 Bundle — AMS VCA Lin](https://patchstorage.com/mod-ams-lv2-bundle/); No exact Beebo mapping |
| AMS VCA Lin - CV | `http://avwlv2.sourceforge.net/plugins/avw/vcalin_cv` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/vcalin_cv.ttl) | AMS lineage variant: [mod-ams.lv2 Bundle — AMS VCA Lin](https://patchstorage.com/mod-ams-lv2-bundle/); No exact Beebo mapping |
| AMS VCF | `http://avwlv2.sourceforge.net/plugins/avw/vcf` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/vcf.ttl) | AMS lineage variant: [mod-ams.lv2 Bundle — AMS VCF](https://patchstorage.com/mod-ams-lv2-bundle/); No exact Beebo mapping |
| AMS VCO2 - Audio | `http://avwlv2.sourceforge.net/plugins/avw/vco2_audio` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/vco2_audio.ttl) | AMS lineage variant: [mod-ams.lv2 Bundle — AMS VCO3](https://patchstorage.com/mod-ams-lv2-bundle/); No exact Beebo mapping |
| AMS VCO2 - CV | `http://avwlv2.sourceforge.net/plugins/avw/vco2_cv` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/vco2_cv.ttl) | AMS lineage variant: [mod-ams.lv2 Bundle — AMS VCO3](https://patchstorage.com/mod-ams-lv2-bundle/); No exact Beebo mapping |
| Beat Repeater - Mono | `http://avwlv2.sourceforge.net/plugins/avw/beatrepeater_mono` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/beatrepeater_mono.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| Beat Repeater - Stereo | `http://avwlv2.sourceforge.net/plugins/avw/beatrepeater_stereo` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/beatrepeater_stereo.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| Beat Slicer - Mono | `http://avwlv2.sourceforge.net/plugins/avw/beatslicer_mono` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/beatslicer_mono.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| Beat Slicer - Stereo | `http://avwlv2.sourceforge.net/plugins/avw/beatslicer_stereo` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/beatslicer_stereo.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| Control to CV | `http://avwlv2.sourceforge.net/plugins/avw/controltocv` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/controltocv.ttl) | Functional counterpart: [Control to CV — Control to CV](https://patchstorage.com/control-to-cv/); No exact Beebo mapping |
| CV to Control | `http://avwlv2.sourceforge.net/plugins/avw/cvtocontrol` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/cvtocontrol.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| Downsampler - Mono | `http://avwlv2.sourceforge.net/plugins/avw/downsampler_mono` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/downsampler_mono.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| Downsampler - Stereo | `http://avwlv2.sourceforge.net/plugins/avw/downsampler_stereo` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/downsampler_stereo.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| Granulator - Mono | `http://avwlv2.sourceforge.net/plugins/avw/granulator_mono` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/granulator_mono.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| Granulator - Stereo | `http://avwlv2.sourceforge.net/plugins/avw/granulator_stereo` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/granulator_stereo.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| Hz to VC | `http://avwlv2.sourceforge.net/plugins/avw/hztovc` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/hztovc.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| Moog LPF | `http://avwlv2.sourceforge.net/plugins/avw/mooglpf` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/mooglpf.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| Scope | `http://avwlv2.sourceforge.net/plugins/avw/scope` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/scope.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| Signal Absolute Value | `http://avwlv2.sourceforge.net/plugins/avw/absolute` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/absolute.ttl) | Functional counterpart: [CV ABS — CV ABS](https://patchstorage.com/cv-abs/); No exact Beebo mapping; related family mapping: `rectify_value` |
| VC to Hz | `http://avwlv2.sourceforge.net/plugins/avw/vctohz` | [metadata](https://github.com/polyeffects/avwlv2/blob/bfb2e1f1851bdc6192e0401789a5d62045156d20/avw.lv2/vctohz.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |

### convoLV2 — 3 IDs

| Plugin | Exact ID | Source evidence | Patchstorage counterpart / Beebo mapping |
| --- | --- | --- | --- |
| LV2 Convolution Mono | `http://gareus.org/oss/lv2/convoLV2#Mono` | [metadata](https://github.com/polyeffects/convoLV2/blob/8ddddc69e3a0a4dd1e0fa220144b8c343252409d/lv2ttl/convoLV2.ttl.in) | No exact ID match; equivalent not established; No exact Beebo mapping |
| LV2 Convolution Mono=>Stereo | `http://gareus.org/oss/lv2/convoLV2#MonoToStereo` | [metadata](https://github.com/polyeffects/convoLV2/blob/8ddddc69e3a0a4dd1e0fa220144b8c343252409d/lv2ttl/convoLV2.ttl.in) | No exact ID match; equivalent not established; No exact Beebo mapping |
| LV2 Convolution Stereo | `http://gareus.org/oss/lv2/convoLV2#Stereo` | [metadata](https://github.com/polyeffects/convoLV2/blob/8ddddc69e3a0a4dd1e0fa220144b8c343252409d/lv2ttl/convoLV2.ttl.in) | No exact ID match; equivalent not established; No exact Beebo mapping |

### cv_to_cc.lv2 — 1 IDs

| Plugin | Exact ID | Source evidence | Patchstorage counterpart / Beebo mapping |
| --- | --- | --- | --- |
| CV to MIDI Note | `http://polyeffects.com/lv2/cv_to_note` | [metadata](https://github.com/polyeffects/cv_to_cc.lv2/blob/23152a8bf6dcdf2496df0d696dfd4c1a61f67e54/cv_to_note.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `cv_to_note` |

### digit_carla — 2 IDs

| Plugin | Exact ID | Source evidence | Patchstorage counterpart / Beebo mapping |
| --- | --- | --- | --- |
| digit_delay | `http://polyeffects.com/lv2/digit_delay` | [metadata](https://github.com/polyeffects/digit_carla/blob/75609d7dd365628fca044f64a7141686853e7945/LV2/digit_delay/digit_delay.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `delay` |
| Poly LFO | `http://polyeffects.com/lv2/polylfo` | [metadata](https://github.com/polyeffects/digit_carla/blob/75609d7dd365628fca044f64a7141686853e7945/LV2/PolyLFO/polylfo.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |

### lenticular_lv2 — 21 IDs

| Plugin | Exact ID | Source evidence | Patchstorage counterpart / Beebo mapping |
| --- | --- | --- | --- |
| Poly Beat Repeat | `http://polyeffects.com/lv2/polyclouds#beat_repeat` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polyclouds.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| Poly Looping Envelope Generator | `http://polyeffects.com/lv2/polytides` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polytides.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `looping_envelope` |
| Poly Macro Osc | `http://polyeffects.com/lv2/polyplaits` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polyplaits.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `macro_osc` |
| Poly Mist Granular | `http://polyeffects.com/lv2/polyclouds#granular` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polyclouds.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `granular` |
| Poly Mist Looping Delay | `http://polyeffects.com/lv2/polyclouds#looping_delay` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polyclouds.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `granular_looping` |
| Poly Mist Oliverb | `http://polyeffects.com/lv2/polyclouds#oliverb` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polyclouds.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `pitch_verb` |
| Poly Mist Resonestor | `http://polyeffects.com/lv2/polyclouds#resonestor` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polyclouds.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `resonestor` |
| Poly Mist Spectral | `http://polyeffects.com/lv2/polyclouds#spectral` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polyclouds.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| Poly Mist Stretch | `http://polyeffects.com/lv2/polyclouds#stretch` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polyclouds.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `time_stretch` |
| Poly Random CV Sampler | `http://polyeffects.com/lv2/polymarbles` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polymarbles.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `chaos_controller` |
| Poly Rings / multi-resonator (metadata: Poly Random CV Sampler) | `http://polyeffects.com/lv2/polyrings` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polyrings.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `multi_resonator` |
| Poly Topological Sequencer | `http://polyeffects.com/lv2/polygrids` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polygrids.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `drum_patterns` |
| Poly Twist Binaural Doppler Panner | `http://polyeffects.com/lv2/polywarps#doppler` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polywarps.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `doppler_panner` |
| Poly Twist Bitmangler | `http://polyeffects.com/lv2/polywarps#bitcrusher` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polywarps.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `bitmangle` |
| Poly Twist Chebyschev | `http://polyeffects.com/lv2/polywarps#chebyschev` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polywarps.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| Poly Twist Comparator | `http://polyeffects.com/lv2/polywarps#comparator` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polywarps.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| Poly Twist Delay | `http://polyeffects.com/lv2/polywarps#delay` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polywarps.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `twist_delay` |
| Poly Twist frequency shifter | `http://polyeffects.com/lv2/polywarps#frequency_shifter` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polywarps.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| Poly Twist Meta | `http://polyeffects.com/lv2/polywarps#meta` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polywarps.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `meta_modulation` |
| Poly Twist Vocoder | `http://polyeffects.com/lv2/polywarps#vocoder` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polywarps.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| Poly Twist Wavefolder | `http://polyeffects.com/lv2/polywarps#fold` | [metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polywarps.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `wavefolder` |

### neural-amp-modeler-lv2 — 1 IDs

| Plugin | Exact ID | Source evidence | Patchstorage counterpart / Beebo mapping |
| --- | --- | --- | --- |
| Neural Amp Modeler | `http://github.com/mikeoliphant/neural-amp-modeler-lv2` | [metadata](https://github.com/polyeffects/neural-amp-modeler-lv2/blob/a3a6520ea519513a6ca1e82d350d47e3c7891732/resources/neural_amp_modeler.ttl.in) | Exact ID: [Neural Amp Modeler](https://patchstorage.com/neural-amp-modeler/); Exact Beebo mapping: `amp_nam` |

### poly_filters — 6 IDs

| Plugin | Exact ID | Source evidence | Patchstorage counterpart / Beebo mapping |
| --- | --- | --- | --- |
| diodeLadder | `http://polyeffects.com/lv2/polyfilter#diode_ladder` | [metadata](https://github.com/polyeffects/poly_filters/blob/ebf8fd1cea20e5d167f0c04ded12da13bd70829f/poly_filter.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `diode_ladder_lpf` |
| Korg HPF | `http://polyeffects.com/lv2/polyfilter#korg_hpf` | [metadata](https://github.com/polyeffects/poly_filters/blob/ebf8fd1cea20e5d167f0c04ded12da13bd70829f/poly_filter.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `k_org_hpf` |
| korg lpf | `http://polyeffects.com/lv2/polyfilter#korg_lpf` | [metadata](https://github.com/polyeffects/poly_filters/blob/ebf8fd1cea20e5d167f0c04ded12da13bd70829f/poly_filter.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `k_org_lpf` |
| moog half ladder | `http://polyeffects.com/lv2/polyfilter#moog_half_ladder` | [metadata](https://github.com/polyeffects/poly_filters/blob/ebf8fd1cea20e5d167f0c04ded12da13bd70829f/poly_filter.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `oog_half_lpf` |
| moog ladder | `http://polyeffects.com/lv2/polyfilter#moog_ladder` | [metadata](https://github.com/polyeffects/poly_filters/blob/ebf8fd1cea20e5d167f0c04ded12da13bd70829f/poly_filter.ttl) | No exact ID match; equivalent not established; No exact Beebo mapping |
| oberheim filter | `http://polyeffects.com/lv2/polyfilter#oberheim` | [metadata](https://github.com/polyeffects/poly_filters/blob/ebf8fd1cea20e5d167f0c04ded12da13bd70829f/poly_filter.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `filter_uberheim` |

### polyconvo.lv2 — 6 IDs

| Plugin | Exact ID | Source evidence | Patchstorage counterpart / Beebo mapping |
| --- | --- | --- | --- |
| Poly Convolver Cab Mono | `http://polyeffects.com/lv2/polyconvo#CabMono` | [metadata](https://github.com/polyeffects/polyconvo.lv2/blob/fe8edd2da2e348ecc383c93c5d4e7961e7d8dc5b/lv2ttl/polyconvo.ttl.in) | No exact ID match; equivalent not established; Exact Beebo mapping: `short_mono_cab` |
| Poly Convolver Cab Mono => Stereo | `http://polyeffects.com/lv2/polyconvo#CabMonoToStereo` | [metadata](https://github.com/polyeffects/polyconvo.lv2/blob/fe8edd2da2e348ecc383c93c5d4e7961e7d8dc5b/lv2ttl/polyconvo.ttl.in) | No exact ID match; equivalent not established; Exact Beebo mapping: `stereo_cab` |
| Poly Convolver Cab Stereo | `http://polyeffects.com/lv2/polyconvo#CabStereo` | [metadata](https://github.com/polyeffects/polyconvo.lv2/blob/fe8edd2da2e348ecc383c93c5d4e7961e7d8dc5b/lv2ttl/polyconvo.ttl.in) | No exact ID match; equivalent not established; Exact Beebo mapping: `quad_ir_cab` |
| Poly Convolver Mono | `http://polyeffects.com/lv2/polyconvo#Mono` | [metadata](https://github.com/polyeffects/polyconvo.lv2/blob/fe8edd2da2e348ecc383c93c5d4e7961e7d8dc5b/lv2ttl/polyconvo.ttl.in) | No exact ID match; equivalent not established; Exact Beebo mapping: `mono_reverb` |
| Poly Convolver Mono => Stereo | `http://polyeffects.com/lv2/polyconvo#MonoToStereo` | [metadata](https://github.com/polyeffects/polyconvo.lv2/blob/fe8edd2da2e348ecc383c93c5d4e7961e7d8dc5b/lv2ttl/polyconvo.ttl.in) | No exact ID match; equivalent not established; Exact Beebo mapping: `short_stereo_reverb` |
| Poly Convolver Stereo | `http://polyeffects.com/lv2/polyconvo#Stereo` | [metadata](https://github.com/polyeffects/polyconvo.lv2/blob/fe8edd2da2e348ecc383c93c5d4e7961e7d8dc5b/lv2ttl/polyconvo.ttl.in) | No exact ID match; equivalent not established; Exact Beebo mapping: `short_quad_ir_reverb` |

### PolyLV2 — 33 IDs

| Plugin | Exact ID | Source evidence | Patchstorage counterpart / Beebo mapping |
| --- | --- | --- | --- |
| 16 Step Note Sequencer | `http://polyeffects.com/lv2/note_sequencer` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/poly_note/note_sequencer.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `note_sequencer_ext` |
| 16 Step Note Sequencer BPM | `http://polyeffects.com/lv2/note_sequencer_bpm` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/poly_note_bpm/note_sequencer_bpm.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `note_sequencer` |
| 16 Step Sequencer | `http://polyeffects.com/lv2/step_sequencer` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/poly_step/step_sequencer.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `step_sequencer_ext` |
| 16 Step Sequencer BPM | `http://polyeffects.com/lv2/step_sequencer_bpm` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/poly_step_bpm/step_sequencer_bpm.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `step_sequencer` |
| abs | `http://polyeffects.com/lv2/basic_modular#abs` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/basic_modular.lv2/difference.ttl) | Functional counterpart: [CV ABS — CV ABS](https://patchstorage.com/cv-abs/); Exact Beebo mapping: `rectify_value` |
| bitcrushed_delay | `http://polyeffects.com/lv2/bitcrushed_delay` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/bitcrushed_delay/bitcrushed_delay.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `bitcrushed_delay` |
| boost | `http://polyeffects.com/lv2/basic_modular#boost` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/basic_modular.lv2/amp.ttl) | Partial functional overlap: [Attenuverter Booster — Attenuverter Booster](https://patchstorage.com/attenuverter-booster/); Exact Beebo mapping: `boost` |
| chorus | `http://polyeffects.com/lv2/chorus` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/poly_chorus/chorus.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `chorus_d` |
| chorus_ext | `http://polyeffects.com/lv2/chorus_ext` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/poly_chorus_ext/chorus_ext.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `chorus_d_ext` |
| clock_divider | `http://polyeffects.com/lv2/basic_modular#clock_divider` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/basic_modular.lv2/difference.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `clock_divider` |
| CV to MIDI CC | `http://polyeffects.com/lv2/cv_to_cc` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/cv_to_midi/cv_to_cc.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `cv_to_midi_cc` |
| difference | `http://polyeffects.com/lv2/basic_modular#difference` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/basic_modular.lv2/difference.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `difference` |
| flanger | `http://polyeffects.com/lv2/flanger` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/poly_flange/flanger.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `flanger` |
| flanger_ext | `http://polyeffects.com/lv2/flanger_ext` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/poly_flange_ext/flanger_ext.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `flanger_ext` |
| harm_trem | `http://polyeffects.com/lv2/harm_trem` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/poly_harm_trem/harm_trem.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `harmonic_tremolo` |
| harm_trem_ext | `http://polyeffects.com/lv2/harm_trem_ext` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/poly_harm_trem_ext/harm_trem_ext.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `harmonic_trem_ext` |
| max | `http://polyeffects.com/lv2/basic_modular#max` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/basic_modular.lv2/difference.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `max` |
| min | `http://polyeffects.com/lv2/basic_modular#min` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/basic_modular.lv2/difference.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `min` |
| Pitch calibration output (metadata: pitch_cal_in) | `http://polyeffects.com/lv2/pitch_cal#out` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/poly_cal.lv2/cal.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `pitch_cal_out` |
| pitch_cal_in | `http://polyeffects.com/lv2/pitch_cal#in` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/poly_cal.lv2/cal.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `pitch_cal_in` |
| Poly Euclidean Sequencer | `http://polyeffects.com/lv2/euclidean` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/poly_euclidean/poly_euclidean.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `euclidean` |
| product | `http://polyeffects.com/lv2/basic_modular#product` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/basic_modular.lv2/difference.ttl) | Partial functional overlap: [Attenuverter Booster — Attenuverter Booster](https://patchstorage.com/attenuverter-booster/); Exact Beebo mapping: `product` |
| quantizer | `http://polyeffects.com/lv2/basic_quantizer` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/basic_quantizer/basic_quantizer.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `quantizer` |
| ratio | `http://polyeffects.com/lv2/basic_modular#ratio` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/basic_modular.lv2/difference.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `ratio` |
| schmitt | `http://polyeffects.com/lv2/basic_modular#schmitt` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/basic_modular.lv2/difference.ttl) | Partial functional overlap: [CV Gate — CV Gate](https://patchstorage.com/cv-gate/); Exact Beebo mapping: `cv_to_trigger` |
| strum | `http://polyeffects.com/lv2/strum` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/poly_strum/strum.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `strum` |
| sum | `http://polyeffects.com/lv2/basic_modular#sum` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/basic_modular.lv2/difference.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `sum` |
| tempo_ratio | `http://polyeffects.com/lv2/basic_modular#tempo_ratio` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/basic_modular.lv2/difference.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `tempo_ratio` |
| toggle | `http://polyeffects.com/lv2/basic_modular#toggle` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/basic_modular.lv2/difference.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `toggle` |
| trigger_to_gate | `http://polyeffects.com/lv2/basic_modular#trigger_to_gate` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/basic_modular.lv2/difference.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `trigger_to_gate` |
| VCA | `http://polyeffects.com/lv2/basic_modular#amp` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/basic_modular.lv2/amp.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `vca` |
| vibrato | `http://polyeffects.com/lv2/vibrato` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/poly_vibrato/vibrato.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `vibrato` |
| vibrato_ext | `http://polyeffects.com/lv2/vibrato_ext` | [metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/poly_vibrato_ext/vibrato_ext.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `vibrato_ext` |

### PolyMclkIn.lv2 — 1 IDs

| Plugin | Exact ID | Source evidence | Patchstorage counterpart / Beebo mapping |
| --- | --- | --- | --- |
| Poly MIDI Clock in | `http://polyeffects.com/lv2/mclk_in` | [metadata](https://github.com/polyeffects/PolyMclkIn.lv2/blob/64acb88d6c574b4efc743b770d1d22444391ba02/mclk_in.ttl) | No exact ID match; equivalent not established; Exact Beebo mapping: `midi_clock_in` |

### tuna.lv2 — 3 IDs

| Plugin | Exact ID | Source evidence | Patchstorage counterpart / Beebo mapping |
| --- | --- | --- | --- |
| x42 Instrument Tuner | `http://gareus.org/oss/lv2/tuna#one` | [metadata](https://github.com/polyeffects/tuna.lv2/blob/cfdfb6aeb065b278f58b6c6d4c23b5d82ae34279/lv2ttl/tuna.lv2.ttl.in) | No exact ID match; equivalent not established; No exact Beebo mapping |
| x42 Instrument Tuner (MOD build) | `http://gareus.org/oss/lv2/tuna#mod` | [metadata](https://github.com/polyeffects/tuna.lv2/blob/cfdfb6aeb065b278f58b6c6d4c23b5d82ae34279/lv2ttl/tuna.lv2.ttl.in) | Exact ID: [Instrument Tuner](https://patchstorage.com/instrument-tuner/); Exact Beebo mapping: `tuner` |
| x42 Instrument Tuner [Spectrum] | `http://gareus.org/oss/lv2/tuna#two` | [metadata](https://github.com/polyeffects/tuna.lv2/blob/cfdfb6aeb065b278f58b6c6d4c23b5d82ae34279/lv2ttl/tuna.lv2.ttl.in) | No exact ID match; equivalent not established; No exact Beebo mapping |

## Complete Beebo plugin map — 157 mappings, 153 IDs

These are the exact mapping keys in `digit_ui/module_info.py`; labels below are readable expansions of those keys. Five footswitch mappings share one URI. The `mix_vca` mapping is present but hidden by `show_widget.py`’s browser filter. Four extra prototype definitions (`input`, `output`, `midi_input`, `midi_output`) are interface endpoints and have no entry in the plugin URI map.

| Beebo mapping | Exact ID | Published implementation | Patchstorage |
| --- | --- | --- | --- |
| `ad_env_level` | `http://drobilla.net/plugins/omins/adenv_lvl` | Beebo reference only | No exact ID match; equivalent not established |
| `ad_envelope` | `http://drobilla.net/plugins/omins/adenv` | Beebo reference only | No exact ID match; equivalent not established |
| `adsr` | `http://drobilla.net/plugins/blop/adsr` | Beebo reference only | No exact ID match; equivalent not established |
| `amp_bass` | `http://guitarix.sourceforge.net/plugins/gx_ampegsvt_#_ampegsvt_` | Beebo reference only | Exact ID: [GxSVT](https://patchstorage.com/gxsvt/) |
| `amp_nam` | `http://github.com/mikeoliphant/neural-amp-modeler-lv2` | neural-amp-modeler-lv2 | Exact ID: [Neural Amp Modeler](https://patchstorage.com/neural-amp-modeler/) |
| `attenuverter` | `http://drobilla.net/plugins/blop/product` | Beebo reference only | Partial functional overlap: [Attenuverter Booster — Attenuverter Booster](https://patchstorage.com/attenuverter-booster/) |
| `auto_swell` | `http://guitarix.sourceforge.net/plugins/gx_slowgear_#_slowgear_` | Beebo reference only | Exact ID: [GxSlowGear](https://patchstorage.com/gxslowgear/) |
| `basic_reverb` | `http://distrho.sf.net/plugins/MVerb` | Beebo reference only | Exact ID: [MVerb](https://patchstorage.com/mverb/) |
| `bitcrushed_delay` | `http://polyeffects.com/lv2/bitcrushed_delay` | PolyLV2 | No exact ID match; equivalent not established |
| `bitcrusher` | `http://polyeffects.com/lv2/bitcrusher` | Beebo reference only | No exact ID match; equivalent not established |
| `bitmangle` | `http://polyeffects.com/lv2/polywarps#bitcrusher` | lenticular_lv2 | No exact ID match; equivalent not established |
| `boost` | `http://polyeffects.com/lv2/basic_modular#boost` | PolyLV2 | Partial functional overlap: [Attenuverter Booster — Attenuverter Booster](https://patchstorage.com/attenuverter-booster/) |
| `chaos_controller` | `http://polyeffects.com/lv2/polymarbles` | lenticular_lv2 | No exact ID match; equivalent not established |
| `chorus_d` | `http://polyeffects.com/lv2/chorus` | PolyLV2 | No exact ID match; equivalent not established |
| `chorus_d_ext` | `http://polyeffects.com/lv2/chorus_ext` | PolyLV2 | No exact ID match; equivalent not established |
| `chorus_j` | `https://chrisarndt.de/plugins/ykchorus` | Beebo reference only | No exact ID match; equivalent not established |
| `clock_divider` | `http://polyeffects.com/lv2/basic_modular#clock_divider` | PolyLV2 | No exact ID match; equivalent not established |
| `cv_meter` | `http://moddevices.com/plugins/mod-devel/mod-cv-meter` | Beebo reference only | Exact ID: [CV meter](https://patchstorage.com/cv-meter/) |
| `cv_to_midi_cc` | `http://polyeffects.com/lv2/cv_to_cc` | PolyLV2 | No exact ID match; equivalent not established |
| `cv_to_note` | `http://polyeffects.com/lv2/cv_to_note` | PolyLV2, cv_to_cc.lv2 | No exact ID match; equivalent not established |
| `cv_to_trigger` | `http://polyeffects.com/lv2/basic_modular#schmitt` | PolyLV2 | Partial functional overlap: [CV Gate — CV Gate](https://patchstorage.com/cv-gate/) |
| `dahdsr` | `http://drobilla.net/plugins/blop/dahdsr` | Beebo reference only | No exact ID match; equivalent not established |
| `delay` | `http://polyeffects.com/lv2/digit_delay` | digit_carla | No exact ID match; equivalent not established |
| `difference` | `http://polyeffects.com/lv2/basic_modular#difference` | PolyLV2 | No exact ID match; equivalent not established |
| `diode_ladder_lpf` | `http://polyeffects.com/lv2/polyfilter#diode_ladder` | poly_filters | No exact ID match; equivalent not established |
| `doppler_panner` | `http://polyeffects.com/lv2/polywarps#doppler` | lenticular_lv2 | No exact ID match; equivalent not established |
| `drum_patterns` | `http://polyeffects.com/lv2/polygrids` | lenticular_lv2 | No exact ID match; equivalent not established |
| `env_follower` | `http://ssj71.github.io/infamousPlugins/plugs.html#envfollowerCV` | Beebo reference only | No exact ID match; equivalent not established |
| `euclidean` | `http://polyeffects.com/lv2/euclidean` | PolyLV2 | No exact ID match; equivalent not established |
| `filter` | `http://drobilla.net/plugins/fomp/mvclpf1` | Beebo reference only | Exact ID: [fomp.lv2 Bundle — bundle extra](https://patchstorage.com/fomp-lv2-bundle/) |
| `filter_uberheim` | `http://polyeffects.com/lv2/polyfilter#oberheim` | poly_filters | No exact ID match; equivalent not established |
| `flanger` | `http://polyeffects.com/lv2/flanger` | PolyLV2 | No exact ID match; equivalent not established |
| `flanger_ext` | `http://polyeffects.com/lv2/flanger_ext` | PolyLV2 | No exact ID match; equivalent not established |
| `foot_switch_a` | `http://polyeffects.com/lv2/polyfoot` | Beebo reference only | No exact ID match; equivalent not established |
| `foot_switch_b` | `http://polyeffects.com/lv2/polyfoot` | Beebo reference only | No exact ID match; equivalent not established |
| `foot_switch_c` | `http://polyeffects.com/lv2/polyfoot` | Beebo reference only | No exact ID match; equivalent not established |
| `foot_switch_d` | `http://polyeffects.com/lv2/polyfoot` | Beebo reference only | No exact ID match; equivalent not established |
| `foot_switch_e` | `http://polyeffects.com/lv2/polyfoot` | Beebo reference only | No exact ID match; equivalent not established |
| `freeze` | `http://ssj71.github.io/infamousPlugins/plugs.html#stuck` | Beebo reference only | Exact ID: [stuck.lv2 Bundle](https://patchstorage.com/stuck-lv2-bundle/) |
| `granular` | `http://polyeffects.com/lv2/polyclouds#granular` | lenticular_lv2 | No exact ID match; equivalent not established |
| `granular_looping` | `http://polyeffects.com/lv2/polyclouds#looping_delay` | lenticular_lv2 | No exact ID match; equivalent not established |
| `harmonic_trem_ext` | `http://polyeffects.com/lv2/harm_trem_ext` | PolyLV2 | No exact ID match; equivalent not established |
| `harmonic_tremolo` | `http://polyeffects.com/lv2/harm_trem` | PolyLV2 | No exact ID match; equivalent not established |
| `k_org_hpf` | `http://polyeffects.com/lv2/polyfilter#korg_hpf` | poly_filters | No exact ID match; equivalent not established |
| `k_org_lpf` | `http://polyeffects.com/lv2/polyfilter#korg_lpf` | poly_filters | No exact ID match; equivalent not established |
| `lfo` | `http://avwlv2.sourceforge.net/plugins/avw/lfo_tempo` | avwlv2 | Related variant: [mod-ams.lv2 Bundle — AMS LFO2 - Freq](https://patchstorage.com/mod-ams-lv2-bundle/) |
| `loop_common_in` | `http://polyeffects.com/lv2/loop_common_in` | Beebo reference only | No exact ID match; equivalent not established |
| `loop_common_out` | `http://polyeffects.com/lv2/loop_common_out` | Beebo reference only | No exact ID match; equivalent not established |
| `loop_extra_midi` | `http://polyeffects.com/lv2/loop_extra_midi` | Beebo reference only | No exact ID match; equivalent not established |
| `loop_midi_out` | `http://polyeffects.com/lv2/loop_midi_out` | Beebo reference only | No exact ID match; equivalent not established |
| `looping_envelope` | `http://polyeffects.com/lv2/polytides` | lenticular_lv2 | No exact ID match; equivalent not established |
| `macro_osc` | `http://polyeffects.com/lv2/polyplaits` | lenticular_lv2 | No exact ID match; equivalent not established |
| `matrix_mixer` | `http://gareus.org/oss/lv2/matrixmixer#i4o3` | Beebo reference only | No exact ID match; equivalent not established |
| `max` | `http://polyeffects.com/lv2/basic_modular#max` | PolyLV2 | No exact ID match; equivalent not established |
| `meta_modulation` | `http://polyeffects.com/lv2/polywarps#meta` | lenticular_lv2 | No exact ID match; equivalent not established |
| `midi_cc` | `http://github.com/blablack/midimsg-lv2/controller` | Beebo reference only | No exact ID match; equivalent not established |
| `midi_cc_to_note` | `http://gareus.org/oss/lv2/midifilter#cctonote` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_channel_filter` | `http://gareus.org/oss/lv2/midifilter#channelfilter` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_channel_map` | `http://gareus.org/oss/lv2/midifilter#channelmap` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_choke_filter` | `http://gareus.org/oss/lv2/midifilter#chokefilter` | Beebo reference only | No exact ID match; equivalent not established |
| `midi_chord` | `http://gareus.org/oss/lv2/midifilter#midichord` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_clock_in` | `http://polyeffects.com/lv2/mclk_in` | PolyMclkIn.lv2 | No exact ID match; equivalent not established |
| `midi_clock_out` | `http://gareus.org/oss/lv2/mclk` | Beebo reference only | Exact ID: [MIDI Clock Generator](https://patchstorage.com/midi-clock-generator/) |
| `midi_delay` | `http://gareus.org/oss/lv2/midifilter#mididelay` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_dup` | `http://gareus.org/oss/lv2/midifilter#mididup` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_enforce_scale` | `http://gareus.org/oss/lv2/midifilter#enforcescale` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_event_blocker` | `http://gareus.org/oss/lv2/midifilter#eventblocker` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_keyrange` | `http://gareus.org/oss/lv2/midifilter#keyrange` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_keysplit` | `http://gareus.org/oss/lv2/midifilter#keysplit` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_map_key_channel` | `http://gareus.org/oss/lv2/midifilter#mapkeychannel` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_map_key_scale` | `http://gareus.org/oss/lv2/midifilter#mapkeyscale` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_mapcc` | `http://gareus.org/oss/lv2/midifilter#mapcc` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_mono_legato` | `http://gareus.org/oss/lv2/midifilter#monolegato` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_no_active_sensing` | `http://gareus.org/oss/lv2/midifilter#noactivesensing` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_nodup` | `http://gareus.org/oss/lv2/midifilter#nodup` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_note_to_cc` | `http://gareus.org/oss/lv2/midifilter#notetocc` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_note_to_cv` | `http://polyeffects.com/lv2/midi_to_cv_mono` | Beebo reference only | No exact ID match; equivalent not established |
| `midi_note_to_pgm` | `http://gareus.org/oss/lv2/midifilter#notetopgm` | Beebo reference only | No exact ID match; equivalent not established |
| `midi_note_toggle` | `http://gareus.org/oss/lv2/midifilter#notetoggle` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_ntap_delay` | `http://gareus.org/oss/lv2/midifilter#ntapdelay` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_one_channel_filter` | `http://gareus.org/oss/lv2/midifilter#onechannelfilter` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_pc` | `http://polyeffects.com/lv2/midi_pc` | Beebo reference only | No exact ID match; equivalent not established |
| `midi_quantize` | `http://gareus.org/oss/lv2/midifilter#quantize` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_rand_velocity` | `http://gareus.org/oss/lv2/midifilter#randvelocity` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_scale_cc` | `http://gareus.org/oss/lv2/midifilter#scalecc` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_sostenuto` | `http://gareus.org/oss/lv2/midifilter#sostenuto` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_strum` | `http://gareus.org/oss/lv2/midifilter#midistrum` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_tonal_pedal` | `http://gareus.org/oss/lv2/midifilter#tonalpedal` | Beebo reference only | No exact ID match; equivalent not established |
| `midi_transpose` | `http://gareus.org/oss/lv2/midifilter#miditranspose` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_velocity_gamma` | `http://gareus.org/oss/lv2/midifilter#velocitygamma` | Beebo reference only | No exact ID match; equivalent not established |
| `midi_velocity_range` | `http://gareus.org/oss/lv2/midifilter#velocityrange` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `midi_velocity_scale` | `http://gareus.org/oss/lv2/midifilter#velocityscale` | Beebo reference only | Exact ID: [x42 MIDI Filter Collection](https://patchstorage.com/midifilter-lv2-bundle/) |
| `min` | `http://polyeffects.com/lv2/basic_modular#min` | PolyLV2 | No exact ID match; equivalent not established |
| `mix_vca` | `http://avwlv2.sourceforge.net/plugins/avw/vcaexp_audio` | avwlv2 | AMS lineage variant: [mod-ams.lv2 Bundle — AMS VCA Exp](https://patchstorage.com/mod-ams-lv2-bundle/) |
| `mono_EQ` | `http://gareus.org/oss/lv2/fil4#mono` | Beebo reference only | Exact ID: [fil4.lv2 Bundle](https://patchstorage.com/fil4-lv2-bundle/) |
| `mono_cab` | `http://gareus.org/oss/lv2/zeroconvolv#CfgMono` | Beebo reference only | Exact ID: [zeroconvo.lv2 Bundle](https://patchstorage.com/zeroconvo-lv2-bundle/) |
| `mono_compressor` | `http://gareus.org/oss/lv2/darc#mono` | Beebo reference only | No exact ID match; equivalent not established |
| `mono_reverb` | `http://polyeffects.com/lv2/polyconvo#Mono` | polyconvo.lv2 | No exact ID match; equivalent not established |
| `multi_resonator` | `http://polyeffects.com/lv2/polyrings` | lenticular_lv2 | No exact ID match; equivalent not established |
| `note_sequencer` | `http://polyeffects.com/lv2/note_sequencer_bpm` | PolyLV2 | No exact ID match; equivalent not established |
| `note_sequencer_ext` | `http://polyeffects.com/lv2/note_sequencer` | PolyLV2 | No exact ID match; equivalent not established |
| `onset_detect` | `http://polyeffects.com/lv2/detect#onset` | Beebo reference only | No exact ID match; equivalent not established |
| `oog_half_lpf` | `http://polyeffects.com/lv2/polyfilter#moog_half_ladder` | poly_filters | No exact ID match; equivalent not established |
| `pan` | `http://avwlv2.sourceforge.net/plugins/avw/vcpanning` | avwlv2 | No exact ID match; equivalent not established |
| `phaser` | `http://jpcima.sdf1.org/lv2/stone-phaser` | Beebo reference only | No exact ID match; equivalent not established |
| `phaser_ext` | `http://polyeffects.com/lv2/stone-phaser-ext` | Beebo reference only | No exact ID match; equivalent not established |
| `phaser_stereo_ext` | `http://polyeffects.com/lv2/stone-phaser-stereo-ext` | Beebo reference only | No exact ID match; equivalent not established |
| `pitch_cal_in` | `http://polyeffects.com/lv2/pitch_cal#in` | PolyLV2 | No exact ID match; equivalent not established |
| `pitch_cal_out` | `http://polyeffects.com/lv2/pitch_cal#out` | PolyLV2 | No exact ID match; equivalent not established |
| `pitch_detect` | `http://polyeffects.com/lv2/detect#pitch` | Beebo reference only | No exact ID match; equivalent not established |
| `pitch_shift` | `http://moddevices.com/plugins/tap/pitch` | Beebo reference only | Exact ID: [TAP Pitch Shifter](https://patchstorage.com/tap-pitch-shifter/) |
| `pitch_verb` | `http://polyeffects.com/lv2/polyclouds#oliverb` | lenticular_lv2 | No exact ID match; equivalent not established |
| `poly_note_to_cv` | `http://polyeffects.com/lv2/midi_to_cv_poly` | Beebo reference only | No exact ID match; equivalent not established |
| `power_amp_cream` | `http://guitarix.sourceforge.net/plugins/gx_CreamMachine_#_CreamMachine_` | Beebo reference only | No exact ID match; equivalent not established |
| `power_amp_super` | `http://guitarix.sourceforge.net/plugins/gx_supersonic_#_supersonic_` | Beebo reference only | No exact ID match; equivalent not established |
| `product` | `http://polyeffects.com/lv2/basic_modular#product` | PolyLV2 | Partial functional overlap: [Attenuverter Booster — Attenuverter Booster](https://patchstorage.com/attenuverter-booster/) |
| `quad_ir_cab` | `http://polyeffects.com/lv2/polyconvo#CabStereo` | polyconvo.lv2 | No exact ID match; equivalent not established |
| `quad_ir_reverb` | `http://gareus.org/oss/lv2/zeroconvolv#CfgStereo` | Beebo reference only | Exact ID: [zeroconvo.lv2 Bundle](https://patchstorage.com/zeroconvo-lv2-bundle/) |
| `quantizer` | `http://polyeffects.com/lv2/basic_quantizer` | PolyLV2 | No exact ID match; equivalent not established |
| `ratio` | `http://polyeffects.com/lv2/basic_modular#ratio` | PolyLV2 | No exact ID match; equivalent not established |
| `rectify_value` | `http://polyeffects.com/lv2/basic_modular#abs` | PolyLV2 | Functional counterpart: [CV ABS — CV ABS](https://patchstorage.com/cv-abs/) |
| `resonestor` | `http://polyeffects.com/lv2/polyclouds#resonestor` | lenticular_lv2 | No exact ID match; equivalent not established |
| `reverse` | `http://moddevices.com/plugins/tap/reflector` | Beebo reference only | Exact ID: [TAP Reflector](https://patchstorage.com/tap-reflector/) |
| `rotary` | `http://gareus.org/oss/lv2/b_whirl#simple` | Beebo reference only | Exact ID: [setBfree Whirl Speaker (Old version)](https://patchstorage.com/setbfree-whirl-speaker-old-version/) |
| `rotary_advanced` | `http://gareus.org/oss/lv2/b_whirl#extended` | Beebo reference only | No exact ID match; equivalent not established |
| `sample_hold` | `http://avwlv2.sourceforge.net/plugins/avw/samplehold` | avwlv2 | No exact ID match; equivalent not established |
| `saturator` | `http://moddevices.com/plugins/tap/sigmoid` | Beebo reference only | Exact ID: [TAP Sigmoid Booster](https://patchstorage.com/tap-sigmoid-booster/) |
| `short_mono_cab` | `http://polyeffects.com/lv2/polyconvo#CabMono` | polyconvo.lv2 | No exact ID match; equivalent not established |
| `short_quad_ir_reverb` | `http://polyeffects.com/lv2/polyconvo#Stereo` | polyconvo.lv2 | No exact ID match; equivalent not established |
| `short_stereo_reverb` | `http://polyeffects.com/lv2/polyconvo#MonoToStereo` | polyconvo.lv2 | No exact ID match; equivalent not established |
| `slew_limiter` | `http://avwlv2.sourceforge.net/plugins/avw/slew` | avwlv2 | No exact ID match; equivalent not established |
| `step_sequencer` | `http://polyeffects.com/lv2/step_sequencer_bpm` | PolyLV2 | No exact ID match; equivalent not established |
| `step_sequencer_ext` | `http://polyeffects.com/lv2/step_sequencer` | PolyLV2 | No exact ID match; equivalent not established |
| `stereo_EQ` | `http://gareus.org/oss/lv2/fil4#stereo` | Beebo reference only | Exact ID: [fil4.lv2 Bundle](https://patchstorage.com/fil4-lv2-bundle/) |
| `stereo_cab` | `http://polyeffects.com/lv2/polyconvo#CabMonoToStereo` | polyconvo.lv2 | No exact ID match; equivalent not established |
| `stereo_compress` | `http://gareus.org/oss/lv2/darc#stereo` | Beebo reference only | No exact ID match; equivalent not established |
| `stereo_phaser` | `http://jpcima.sdf1.org/lv2/stone-phaser-stereo` | Beebo reference only | No exact ID match; equivalent not established |
| `stereo_reverb` | `http://gareus.org/oss/lv2/zeroconvolv#CfgMonoToStereo` | Beebo reference only | No exact ID match; equivalent not established |
| `strum` | `http://polyeffects.com/lv2/strum` | PolyLV2 | No exact ID match; equivalent not established |
| `sum` | `http://polyeffects.com/lv2/basic_modular#sum` | PolyLV2 | No exact ID match; equivalent not established |
| `tempo_ratio` | `http://polyeffects.com/lv2/basic_modular#tempo_ratio` | PolyLV2 | No exact ID match; equivalent not established |
| `thruzero_flange` | `http://drobilla.net/plugins/mda/ThruZero` | Beebo reference only | No exact ID match; equivalent not established |
| `time_stretch` | `http://polyeffects.com/lv2/polyclouds#stretch` | lenticular_lv2 | No exact ID match; equivalent not established |
| `toggle` | `http://polyeffects.com/lv2/basic_modular#toggle` | PolyLV2 | No exact ID match; equivalent not established |
| `tonestack` | `http://polyeffects.com/lv2/poly_3_band_eq` | Beebo reference only | No exact ID match; equivalent not established |
| `trigger_to_gate` | `http://polyeffects.com/lv2/basic_modular#trigger_to_gate` | PolyLV2 | No exact ID match; equivalent not established |
| `tuner` | `http://gareus.org/oss/lv2/tuna#mod` | tuna.lv2 | Exact ID: [Instrument Tuner](https://patchstorage.com/instrument-tuner/) |
| `turntable_stop` | `http://ssj71.github.io/infamousPlugins/plugs.html#powercut` | Beebo reference only | Exact ID: [the infamous power cut](https://patchstorage.com/the-infamous-power-cut/) |
| `twist_delay` | `http://polyeffects.com/lv2/polywarps#delay` | lenticular_lv2 | No exact ID match; equivalent not established |
| `vca` | `http://polyeffects.com/lv2/basic_modular#amp` | PolyLV2 | No exact ID match; equivalent not established |
| `vibrato` | `http://polyeffects.com/lv2/vibrato` | PolyLV2 | No exact ID match; equivalent not established |
| `vibrato_ext` | `http://polyeffects.com/lv2/vibrato_ext` | PolyLV2 | No exact ID match; equivalent not established |
| `vinyl` | `http://calf.sourceforge.net/plugins/Vinyl` | Beebo reference only | No exact ID match; equivalent not established |
| `warmth` | `http://moddevices.com/plugins/tap/tubewarmth` | Beebo reference only | Exact ID: [TAP Tubewarmth](https://patchstorage.com/tap-tubewarmth/) |
| `wavefolder` | `http://polyeffects.com/lv2/polywarps#fold` | lenticular_lv2 | No exact ID match; equivalent not established |
| `wet_dry` | `http://polyeffects.com/lv2/wet_dry` | Beebo reference only | No exact ID match; equivalent not established |
| `wet_dry_stereo` | `http://polyeffects.com/lv2/wet_dry_stereo` | Beebo reference only | No exact ID match; equivalent not established |

## Poly IDs without an exact Patchstorage URI match — 160 IDs

This is the strict URI difference, not a list of unavailable plugin functions. Rows with verified AMS or CV counterparts remain here because their IDs differ; their counterpart is explicitly shown. The Patchstorage table likewise labels exact matches, verified counterparts, partial relationships and unestablished equivalents independently.

| Plugin / mapping | Evidence in Poly | Poly ID | Patchstorage counterpart review |
| --- | --- | --- | --- |
| 16 Step Note Sequencer | PolyLV2 | `http://polyeffects.com/lv2/note_sequencer` | No exact ID match; equivalent not established |
| 16 Step Note Sequencer BPM | PolyLV2 | `http://polyeffects.com/lv2/note_sequencer_bpm` | No exact ID match; equivalent not established |
| 16 Step Sequencer | PolyLV2 | `http://polyeffects.com/lv2/step_sequencer` | No exact ID match; equivalent not established |
| 16 Step Sequencer BPM | PolyLV2 | `http://polyeffects.com/lv2/step_sequencer_bpm` | No exact ID match; equivalent not established |
| abs | PolyLV2 | `http://polyeffects.com/lv2/basic_modular#abs` | Functional counterpart: [CV ABS — CV ABS](https://patchstorage.com/cv-abs/) |
| ad env level | Beebo reference only | `http://drobilla.net/plugins/omins/adenv_lvl` | No exact ID match; equivalent not established |
| ad envelope | Beebo reference only | `http://drobilla.net/plugins/omins/adenv` | No exact ID match; equivalent not established |
| adsr | Beebo reference only | `http://drobilla.net/plugins/blop/adsr` | No exact ID match; equivalent not established |
| Amplitude Modifier | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/amplitude` | No exact ID match; equivalent not established |
| AMS Advanced Envelope | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/advenv` | No exact ID match; equivalent not established |
| AMS Amp | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/amp` | Partial functional overlap: [Attenuverter Booster — Attenuverter Booster](https://patchstorage.com/attenuverter-booster/) |
| AMS Analog Driver | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/ad` | No exact ID match; equivalent not established |
| AMS CV Source | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/cvs` | Partial functional overlap: [Control to CV — Control to CV](https://patchstorage.com/control-to-cv/) |
| AMS Delay | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/delay` | No exact ID match; equivalent not established |
| AMS Envelope | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/env` | AMS lineage variant: [mod-ams.lv2 Bundle — AMS Envelope](https://patchstorage.com/mod-ams-lv2-bundle/) |
| AMS Inv | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/inv` | No exact ID match; equivalent not established |
| AMS LFO - Freq | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/lfo_freq` | AMS lineage variant: [mod-ams.lv2 Bundle — AMS LFO2 - Freq](https://patchstorage.com/mod-ams-lv2-bundle/) |
| AMS LFO - Tempo | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/lfo_tempo` | Related variant: [mod-ams.lv2 Bundle — AMS LFO2 - Freq](https://patchstorage.com/mod-ams-lv2-bundle/) |
| AMS Mixer 2 channels - Audio | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/mixer_2ch_audio` | No exact ID match; equivalent not established |
| AMS Mixer 2 channels - CV | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/mixer_2ch_cv` | No exact ID match; equivalent not established |
| AMS Mixer 4 channels - Audio | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/mixer_4ch_audio` | No exact ID match; equivalent not established |
| AMS Mixer 4 channels - CV | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/mixer_4ch_cv` | No exact ID match; equivalent not established |
| AMS Noise2 - Audio | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/noise2_audio` | No exact ID match; equivalent not established |
| AMS Noise2 - CV | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/noise2_cv` | No exact ID match; equivalent not established |
| AMS Percussive Envelope | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/percussiveenv` | No exact ID match; equivalent not established |
| AMS Ring Modulator - Audio | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/ringmodulator_audio` | No exact ID match; equivalent not established |
| AMS Ring Modulator - CV | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/ringmodulator_cv` | No exact ID match; equivalent not established |
| AMS Sample&Hold | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/samplehold` | No exact ID match; equivalent not established |
| AMS Slew Limiter | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/slew` | No exact ID match; equivalent not established |
| AMS VC Panning | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/vcpanning` | No exact ID match; equivalent not established |
| AMS VC Switch - Audio | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/vcswitch_audio` | No exact ID match; equivalent not established |
| AMS VC Switch - CV | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/vcswitch_cv` | No exact ID match; equivalent not established |
| AMS VCA Exp - Audio | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/vcaexp_audio` | AMS lineage variant: [mod-ams.lv2 Bundle — AMS VCA Exp](https://patchstorage.com/mod-ams-lv2-bundle/) |
| AMS VCA Exp - CV | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/vcaexp_cv` | AMS lineage variant: [mod-ams.lv2 Bundle — AMS VCA Exp](https://patchstorage.com/mod-ams-lv2-bundle/) |
| AMS VCA Lin - Audio | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/vcalin_audio` | AMS lineage variant: [mod-ams.lv2 Bundle — AMS VCA Lin](https://patchstorage.com/mod-ams-lv2-bundle/) |
| AMS VCA Lin - CV | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/vcalin_cv` | AMS lineage variant: [mod-ams.lv2 Bundle — AMS VCA Lin](https://patchstorage.com/mod-ams-lv2-bundle/) |
| AMS VCF | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/vcf` | AMS lineage variant: [mod-ams.lv2 Bundle — AMS VCF](https://patchstorage.com/mod-ams-lv2-bundle/) |
| AMS VCO2 - Audio | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/vco2_audio` | AMS lineage variant: [mod-ams.lv2 Bundle — AMS VCO3](https://patchstorage.com/mod-ams-lv2-bundle/) |
| AMS VCO2 - CV | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/vco2_cv` | AMS lineage variant: [mod-ams.lv2 Bundle — AMS VCO3](https://patchstorage.com/mod-ams-lv2-bundle/) |
| attenuverter | Beebo reference only | `http://drobilla.net/plugins/blop/product` | Partial functional overlap: [Attenuverter Booster — Attenuverter Booster](https://patchstorage.com/attenuverter-booster/) |
| Beat Repeater - Mono | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/beatrepeater_mono` | No exact ID match; equivalent not established |
| Beat Repeater - Stereo | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/beatrepeater_stereo` | No exact ID match; equivalent not established |
| Beat Slicer - Mono | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/beatslicer_mono` | No exact ID match; equivalent not established |
| Beat Slicer - Stereo | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/beatslicer_stereo` | No exact ID match; equivalent not established |
| bitcrushed_delay | PolyLV2 | `http://polyeffects.com/lv2/bitcrushed_delay` | No exact ID match; equivalent not established |
| bitcrusher | Beebo reference only | `http://polyeffects.com/lv2/bitcrusher` | No exact ID match; equivalent not established |
| boost | PolyLV2 | `http://polyeffects.com/lv2/basic_modular#boost` | Partial functional overlap: [Attenuverter Booster — Attenuverter Booster](https://patchstorage.com/attenuverter-booster/) |
| chorus | PolyLV2 | `http://polyeffects.com/lv2/chorus` | No exact ID match; equivalent not established |
| chorus j | Beebo reference only | `https://chrisarndt.de/plugins/ykchorus` | No exact ID match; equivalent not established |
| chorus_ext | PolyLV2 | `http://polyeffects.com/lv2/chorus_ext` | No exact ID match; equivalent not established |
| clock_divider | PolyLV2 | `http://polyeffects.com/lv2/basic_modular#clock_divider` | No exact ID match; equivalent not established |
| Control to CV | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/controltocv` | Functional counterpart: [Control to CV — Control to CV](https://patchstorage.com/control-to-cv/) |
| CV to Control | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/cvtocontrol` | No exact ID match; equivalent not established |
| CV to MIDI CC | PolyLV2 | `http://polyeffects.com/lv2/cv_to_cc` | No exact ID match; equivalent not established |
| CV to MIDI Note | PolyLV2, cv_to_cc.lv2 | `http://polyeffects.com/lv2/cv_to_note` | No exact ID match; equivalent not established |
| dahdsr | Beebo reference only | `http://drobilla.net/plugins/blop/dahdsr` | No exact ID match; equivalent not established |
| difference | PolyLV2 | `http://polyeffects.com/lv2/basic_modular#difference` | No exact ID match; equivalent not established |
| digit_delay | digit_carla | `http://polyeffects.com/lv2/digit_delay` | No exact ID match; equivalent not established |
| diodeLadder | poly_filters | `http://polyeffects.com/lv2/polyfilter#diode_ladder` | No exact ID match; equivalent not established |
| Downsampler - Mono | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/downsampler_mono` | No exact ID match; equivalent not established |
| Downsampler - Stereo | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/downsampler_stereo` | No exact ID match; equivalent not established |
| env follower | Beebo reference only | `http://ssj71.github.io/infamousPlugins/plugs.html#envfollowerCV` | No exact ID match; equivalent not established |
| flanger | PolyLV2 | `http://polyeffects.com/lv2/flanger` | No exact ID match; equivalent not established |
| flanger_ext | PolyLV2 | `http://polyeffects.com/lv2/flanger_ext` | No exact ID match; equivalent not established |
| foot switch a, foot switch b, foot switch c, foot switch d, foot switch e | Beebo reference only | `http://polyeffects.com/lv2/polyfoot` | No exact ID match; equivalent not established |
| Granulator - Mono | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/granulator_mono` | No exact ID match; equivalent not established |
| Granulator - Stereo | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/granulator_stereo` | No exact ID match; equivalent not established |
| harm_trem | PolyLV2 | `http://polyeffects.com/lv2/harm_trem` | No exact ID match; equivalent not established |
| harm_trem_ext | PolyLV2 | `http://polyeffects.com/lv2/harm_trem_ext` | No exact ID match; equivalent not established |
| Hz to VC | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/hztovc` | No exact ID match; equivalent not established |
| Korg HPF | poly_filters | `http://polyeffects.com/lv2/polyfilter#korg_hpf` | No exact ID match; equivalent not established |
| korg lpf | poly_filters | `http://polyeffects.com/lv2/polyfilter#korg_lpf` | No exact ID match; equivalent not established |
| loop common in | Beebo reference only | `http://polyeffects.com/lv2/loop_common_in` | No exact ID match; equivalent not established |
| loop common out | Beebo reference only | `http://polyeffects.com/lv2/loop_common_out` | No exact ID match; equivalent not established |
| loop extra midi | Beebo reference only | `http://polyeffects.com/lv2/loop_extra_midi` | No exact ID match; equivalent not established |
| loop midi out | Beebo reference only | `http://polyeffects.com/lv2/loop_midi_out` | No exact ID match; equivalent not established |
| LV2 Convolution Mono | convoLV2 | `http://gareus.org/oss/lv2/convoLV2#Mono` | No exact ID match; equivalent not established |
| LV2 Convolution Mono=>Stereo | convoLV2 | `http://gareus.org/oss/lv2/convoLV2#MonoToStereo` | No exact ID match; equivalent not established |
| LV2 Convolution Stereo | convoLV2 | `http://gareus.org/oss/lv2/convoLV2#Stereo` | No exact ID match; equivalent not established |
| matrix mixer | Beebo reference only | `http://gareus.org/oss/lv2/matrixmixer#i4o3` | No exact ID match; equivalent not established |
| max | PolyLV2 | `http://polyeffects.com/lv2/basic_modular#max` | No exact ID match; equivalent not established |
| midi cc | Beebo reference only | `http://github.com/blablack/midimsg-lv2/controller` | No exact ID match; equivalent not established |
| midi choke filter | Beebo reference only | `http://gareus.org/oss/lv2/midifilter#chokefilter` | No exact ID match; equivalent not established |
| midi note to cv | Beebo reference only | `http://polyeffects.com/lv2/midi_to_cv_mono` | No exact ID match; equivalent not established |
| midi note to pgm | Beebo reference only | `http://gareus.org/oss/lv2/midifilter#notetopgm` | No exact ID match; equivalent not established |
| midi pc | Beebo reference only | `http://polyeffects.com/lv2/midi_pc` | No exact ID match; equivalent not established |
| midi tonal pedal | Beebo reference only | `http://gareus.org/oss/lv2/midifilter#tonalpedal` | No exact ID match; equivalent not established |
| midi velocity gamma | Beebo reference only | `http://gareus.org/oss/lv2/midifilter#velocitygamma` | No exact ID match; equivalent not established |
| min | PolyLV2 | `http://polyeffects.com/lv2/basic_modular#min` | No exact ID match; equivalent not established |
| mono compressor | Beebo reference only | `http://gareus.org/oss/lv2/darc#mono` | No exact ID match; equivalent not established |
| moog half ladder | poly_filters | `http://polyeffects.com/lv2/polyfilter#moog_half_ladder` | No exact ID match; equivalent not established |
| moog ladder | poly_filters | `http://polyeffects.com/lv2/polyfilter#moog_ladder` | No exact ID match; equivalent not established |
| Moog LPF | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/mooglpf` | No exact ID match; equivalent not established |
| oberheim filter | poly_filters | `http://polyeffects.com/lv2/polyfilter#oberheim` | No exact ID match; equivalent not established |
| onset detect | Beebo reference only | `http://polyeffects.com/lv2/detect#onset` | No exact ID match; equivalent not established |
| phaser | Beebo reference only | `http://jpcima.sdf1.org/lv2/stone-phaser` | No exact ID match; equivalent not established |
| phaser ext | Beebo reference only | `http://polyeffects.com/lv2/stone-phaser-ext` | No exact ID match; equivalent not established |
| phaser stereo ext | Beebo reference only | `http://polyeffects.com/lv2/stone-phaser-stereo-ext` | No exact ID match; equivalent not established |
| Pitch calibration output (metadata: pitch_cal_in) | PolyLV2 | `http://polyeffects.com/lv2/pitch_cal#out` | No exact ID match; equivalent not established |
| pitch detect | Beebo reference only | `http://polyeffects.com/lv2/detect#pitch` | No exact ID match; equivalent not established |
| pitch_cal_in | PolyLV2 | `http://polyeffects.com/lv2/pitch_cal#in` | No exact ID match; equivalent not established |
| Poly Beat Repeat | lenticular_lv2 | `http://polyeffects.com/lv2/polyclouds#beat_repeat` | No exact ID match; equivalent not established |
| Poly Convolver Cab Mono | polyconvo.lv2 | `http://polyeffects.com/lv2/polyconvo#CabMono` | No exact ID match; equivalent not established |
| Poly Convolver Cab Mono => Stereo | polyconvo.lv2 | `http://polyeffects.com/lv2/polyconvo#CabMonoToStereo` | No exact ID match; equivalent not established |
| Poly Convolver Cab Stereo | polyconvo.lv2 | `http://polyeffects.com/lv2/polyconvo#CabStereo` | No exact ID match; equivalent not established |
| Poly Convolver Mono | polyconvo.lv2 | `http://polyeffects.com/lv2/polyconvo#Mono` | No exact ID match; equivalent not established |
| Poly Convolver Mono => Stereo | polyconvo.lv2 | `http://polyeffects.com/lv2/polyconvo#MonoToStereo` | No exact ID match; equivalent not established |
| Poly Convolver Stereo | polyconvo.lv2 | `http://polyeffects.com/lv2/polyconvo#Stereo` | No exact ID match; equivalent not established |
| Poly Euclidean Sequencer | PolyLV2 | `http://polyeffects.com/lv2/euclidean` | No exact ID match; equivalent not established |
| Poly LFO | digit_carla | `http://polyeffects.com/lv2/polylfo` | No exact ID match; equivalent not established |
| Poly Looping Envelope Generator | lenticular_lv2 | `http://polyeffects.com/lv2/polytides` | No exact ID match; equivalent not established |
| Poly Macro Osc | lenticular_lv2 | `http://polyeffects.com/lv2/polyplaits` | No exact ID match; equivalent not established |
| Poly MIDI Clock in | PolyMclkIn.lv2 | `http://polyeffects.com/lv2/mclk_in` | No exact ID match; equivalent not established |
| Poly Mist Granular | lenticular_lv2 | `http://polyeffects.com/lv2/polyclouds#granular` | No exact ID match; equivalent not established |
| Poly Mist Looping Delay | lenticular_lv2 | `http://polyeffects.com/lv2/polyclouds#looping_delay` | No exact ID match; equivalent not established |
| Poly Mist Oliverb | lenticular_lv2 | `http://polyeffects.com/lv2/polyclouds#oliverb` | No exact ID match; equivalent not established |
| Poly Mist Resonestor | lenticular_lv2 | `http://polyeffects.com/lv2/polyclouds#resonestor` | No exact ID match; equivalent not established |
| Poly Mist Spectral | lenticular_lv2 | `http://polyeffects.com/lv2/polyclouds#spectral` | No exact ID match; equivalent not established |
| Poly Mist Stretch | lenticular_lv2 | `http://polyeffects.com/lv2/polyclouds#stretch` | No exact ID match; equivalent not established |
| poly note to cv | Beebo reference only | `http://polyeffects.com/lv2/midi_to_cv_poly` | No exact ID match; equivalent not established |
| Poly Random CV Sampler | lenticular_lv2 | `http://polyeffects.com/lv2/polymarbles` | No exact ID match; equivalent not established |
| Poly Rings / multi-resonator (metadata: Poly Random CV Sampler) | lenticular_lv2 | `http://polyeffects.com/lv2/polyrings` | No exact ID match; equivalent not established |
| Poly Topological Sequencer | lenticular_lv2 | `http://polyeffects.com/lv2/polygrids` | No exact ID match; equivalent not established |
| Poly Twist Binaural Doppler Panner | lenticular_lv2 | `http://polyeffects.com/lv2/polywarps#doppler` | No exact ID match; equivalent not established |
| Poly Twist Bitmangler | lenticular_lv2 | `http://polyeffects.com/lv2/polywarps#bitcrusher` | No exact ID match; equivalent not established |
| Poly Twist Chebyschev | lenticular_lv2 | `http://polyeffects.com/lv2/polywarps#chebyschev` | No exact ID match; equivalent not established |
| Poly Twist Comparator | lenticular_lv2 | `http://polyeffects.com/lv2/polywarps#comparator` | No exact ID match; equivalent not established |
| Poly Twist Delay | lenticular_lv2 | `http://polyeffects.com/lv2/polywarps#delay` | No exact ID match; equivalent not established |
| Poly Twist frequency shifter | lenticular_lv2 | `http://polyeffects.com/lv2/polywarps#frequency_shifter` | No exact ID match; equivalent not established |
| Poly Twist Meta | lenticular_lv2 | `http://polyeffects.com/lv2/polywarps#meta` | No exact ID match; equivalent not established |
| Poly Twist Vocoder | lenticular_lv2 | `http://polyeffects.com/lv2/polywarps#vocoder` | No exact ID match; equivalent not established |
| Poly Twist Wavefolder | lenticular_lv2 | `http://polyeffects.com/lv2/polywarps#fold` | No exact ID match; equivalent not established |
| power amp cream | Beebo reference only | `http://guitarix.sourceforge.net/plugins/gx_CreamMachine_#_CreamMachine_` | No exact ID match; equivalent not established |
| power amp super | Beebo reference only | `http://guitarix.sourceforge.net/plugins/gx_supersonic_#_supersonic_` | No exact ID match; equivalent not established |
| product | PolyLV2 | `http://polyeffects.com/lv2/basic_modular#product` | Partial functional overlap: [Attenuverter Booster — Attenuverter Booster](https://patchstorage.com/attenuverter-booster/) |
| quantizer | PolyLV2 | `http://polyeffects.com/lv2/basic_quantizer` | No exact ID match; equivalent not established |
| ratio | PolyLV2 | `http://polyeffects.com/lv2/basic_modular#ratio` | No exact ID match; equivalent not established |
| rotary advanced | Beebo reference only | `http://gareus.org/oss/lv2/b_whirl#extended` | No exact ID match; equivalent not established |
| schmitt | PolyLV2 | `http://polyeffects.com/lv2/basic_modular#schmitt` | Partial functional overlap: [CV Gate — CV Gate](https://patchstorage.com/cv-gate/) |
| Scope | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/scope` | No exact ID match; equivalent not established |
| Signal Absolute Value | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/absolute` | Functional counterpart: [CV ABS — CV ABS](https://patchstorage.com/cv-abs/) |
| stereo compress | Beebo reference only | `http://gareus.org/oss/lv2/darc#stereo` | No exact ID match; equivalent not established |
| stereo phaser | Beebo reference only | `http://jpcima.sdf1.org/lv2/stone-phaser-stereo` | No exact ID match; equivalent not established |
| stereo reverb | Beebo reference only | `http://gareus.org/oss/lv2/zeroconvolv#CfgMonoToStereo` | No exact ID match; equivalent not established |
| strum | PolyLV2 | `http://polyeffects.com/lv2/strum` | No exact ID match; equivalent not established |
| sum | PolyLV2 | `http://polyeffects.com/lv2/basic_modular#sum` | No exact ID match; equivalent not established |
| tempo_ratio | PolyLV2 | `http://polyeffects.com/lv2/basic_modular#tempo_ratio` | No exact ID match; equivalent not established |
| thruzero flange | Beebo reference only | `http://drobilla.net/plugins/mda/ThruZero` | No exact ID match; equivalent not established |
| toggle | PolyLV2 | `http://polyeffects.com/lv2/basic_modular#toggle` | No exact ID match; equivalent not established |
| tonestack | Beebo reference only | `http://polyeffects.com/lv2/poly_3_band_eq` | No exact ID match; equivalent not established |
| trigger_to_gate | PolyLV2 | `http://polyeffects.com/lv2/basic_modular#trigger_to_gate` | No exact ID match; equivalent not established |
| VC to Hz | avwlv2 | `http://avwlv2.sourceforge.net/plugins/avw/vctohz` | No exact ID match; equivalent not established |
| VCA | PolyLV2 | `http://polyeffects.com/lv2/basic_modular#amp` | No exact ID match; equivalent not established |
| vibrato | PolyLV2 | `http://polyeffects.com/lv2/vibrato` | No exact ID match; equivalent not established |
| vibrato_ext | PolyLV2 | `http://polyeffects.com/lv2/vibrato_ext` | No exact ID match; equivalent not established |
| vinyl | Beebo reference only | `http://calf.sourceforge.net/plugins/Vinyl` | No exact ID match; equivalent not established |
| wet dry | Beebo reference only | `http://polyeffects.com/lv2/wet_dry` | No exact ID match; equivalent not established |
| wet dry stereo | Beebo reference only | `http://polyeffects.com/lv2/wet_dry_stereo` | No exact ID match; equivalent not established |
| x42 Instrument Tuner | tuna.lv2 | `http://gareus.org/oss/lv2/tuna#one` | No exact ID match; equivalent not established |
| x42 Instrument Tuner [Spectrum] | tuna.lv2 | `http://gareus.org/oss/lv2/tuna#two` | No exact ID match; equivalent not established |

## Naming, packaging and coverage findings

- **Bundle ID lists understate package contents.** For example, `calf.lv2 Bundle` advertises 4 IDs but its inspected PB32 archive declares 45; `rkr.lv2 Bundle` advertises 5 but its inspected Pi64 archive declares 43; `fomp.lv2 Bundle` advertises 6 but its inspected Pi64 archive declares 17. All additional identities are listed above.

- **Gx Redeye has a discrepancy.** The website advertises `#bigchump`, `#chump` and `#vibrochump`, while the inspected Pi64 archive’s plugin manifest declares only `#vibrochump`. The other two remain in the website-advertised inventory; their presence in that download is unconfirmed. [Entry](https://patchstorage.com/gx_redeye-lv2-bundle/)

- **Two entries share a downloadable identity.** `https://mod.audio/plugins/CabinetLoader` appears in both Cabinet Loader and Convolution Loader downloads. It is listed under both entries but counted once, which gives 553 catalogue rows for 552 distinct IDs.

- **Poly Rings has a misleading metadata name.** `polyrings.ttl` calls its plugin “Poly Random CV Sampler”, the same name as Marbles; Beebo maps that Rings URI to `multi_resonator`. These are distinct IDs and are listed separately. [Rings metadata](https://github.com/polyeffects/lenticular_lv2/blob/141fdc89e250987bae421664132ddaedae735107/polyrings.ttl)

- **Pitch calibration repeats a name.** Both `pitch_cal#in` and `pitch_cal#out` use `pitch_cal_in` in their metadata; the exact IDs distinguish input and output. [Metadata](https://github.com/polyeffects/PolyLV2/blob/6f3ae80f6b4eccb8daebfc2b641fde1d48224250/poly_cal.lv2/cal.ttl)

- **The repository named `cv_to_cc.lv2` currently implements CV to MIDI Note.** Its metadata and C header identify `http://polyeffects.com/lv2/cv_to_note`. The CV-to-CC identity used by Beebo is implemented separately inside `PolyLV2/cv_to_midi`. [CV-to-note source](https://github.com/polyeffects/cv_to_cc.lv2/tree/23152a8bf6dcdf2496df0d696dfd4c1a61f67e54)

- **Tuner count includes build identities.** Default Tuna builds use `#one`, MOD builds use `#mod`, and the spectrum variant uses `#two`. These are three IDs representing two variants plus a build-specific identity; a single build does not install all three. [Makefile](https://github.com/polyeffects/tuna.lv2/blob/cfdfb6aeb065b278f58b6c6d4c23b5d82ae34279/Makefile)

- **Beebo references do not ensure public implementation source exists.** Examples include Poly footswitch, wet/dry, detector, extended phaser, looper-routing, MIDI program-change and tone-stack identities. Every reference is retained in the map above, with source presence recorded independently.

- **SooperLooper is also available in Poly’s repositories, outside the LV2 count.** The source declares a LADSPA `SooperLooper` plugin, unique ID `1601`, and includes a macOS Audio Unit implementation. These are recorded here because the request also asks for every plugin in Poly’s repositories; they have no LV2 URI to compare with this platform. [LADSPA source](https://github.com/polyeffects/sooperlooper/blob/0cf3001ed295011714c059fa91d987835811a0f1/src/plugin.cc), [Audio Unit source](https://github.com/polyeffects/sooperlooper/tree/0cf3001ed295011714c059fa91d987835811a0f1/mac/SooperLooperAU)

- **Ingen is a host and graph-as-plugin runtime.** Its user-defined graphs can become LV2 plugins, but are not a fixed additional effect catalogue. Internal host facilities, graph examples, GUI resources, build dependencies and presets are not counted as independent DSP plugins.

## Repository audit — all 21 public Poly repositories

| Repository | Inspected default-branch revision | Inventory treatment |
| --- | --- | --- |
| [avwlv2](https://github.com/polyeffects/avwlv2) | [bfb2e1f1851b](https://github.com/polyeffects/avwlv2/commit/bfb2e1f1851bdc6192e0401789a5d62045156d20) | 46 LV2 IDs |
| [convoLV2](https://github.com/polyeffects/convoLV2) | [8ddddc69e3a0](https://github.com/polyeffects/convoLV2/commit/8ddddc69e3a0a4dd1e0fa220144b8c343252409d) | 3 LV2 IDs |
| [cv_to_cc.lv2](https://github.com/polyeffects/cv_to_cc.lv2) | [23152a8bf6dc](https://github.com/polyeffects/cv_to_cc.lv2/commit/23152a8bf6dcdf2496df0d696dfd4c1a61f67e54) | 1 LV2 IDs |
| [digit_carla](https://github.com/polyeffects/digit_carla) | [75609d7dd365](https://github.com/polyeffects/digit_carla/commit/75609d7dd365628fca044f64a7141686853e7945) | 2 LV2 IDs (archived historical Digit source) |
| [digit_ui](https://github.com/polyeffects/digit_ui) | [feb4df451067](https://github.com/polyeffects/digit_ui/commit/feb4df451067850770ddd13af9f4e2828234dabc) | Beebo plugin references: 157 mappings / 153 IDs |
| [ingen](https://github.com/polyeffects/ingen) | [8dae740425f0](https://github.com/polyeffects/ingen/commit/8dae740425f0f1468198e0cdf1e1c61b3fc6ec5e) | LV2 host / graph runtime; no fixed DSP catalogue counted |
| [lenticular_lv2](https://github.com/polyeffects/lenticular_lv2) | [141fdc89e250](https://github.com/polyeffects/lenticular_lv2/commit/141fdc89e250987bae421664132ddaedae735107) | 21 LV2 IDs |
| [linux](https://github.com/polyeffects/linux) | [6602da91cde9](https://github.com/polyeffects/linux/commit/6602da91cde9a8834361793b70cee90e3a64d4a5) | Hardware kernel source |
| [lv2_to_dict](https://github.com/polyeffects/lv2_to_dict) | [d062ea0151ef](https://github.com/polyeffects/lv2_to_dict/commit/d062ea0151ef7a8877361bf8c2ce0c5078e8ccad) | LV2 metadata conversion utility |
| [manual](https://github.com/polyeffects/manual) | [6d9e199333c2](https://github.com/polyeffects/manual/commit/6d9e199333c2162b80dfa922ef4853c0119bbd80) | Manual documentation |
| [neural-amp-modeler-lv2](https://github.com/polyeffects/neural-amp-modeler-lv2) | [a3a6520ea519](https://github.com/polyeffects/neural-amp-modeler-lv2/commit/a3a6520ea519513a6ca1e82d350d47e3c7891732) | 1 LV2 IDs |
| [poly_capture](https://github.com/polyeffects/poly_capture) | [f1ad646dcdfe](https://github.com/polyeffects/poly_capture/commit/f1ad646dcdfe906d51e27d45e78125bf7565ed8b) | Amp and impulse-response capture scripts |
| [poly_erp](https://github.com/polyeffects/poly_erp) | [1a70425c8f17](https://github.com/polyeffects/poly_erp/commit/1a70425c8f17fc715992c9a0591b2dedefbbc94f) | Inventory/manufacturing integration |
| [poly_filters](https://github.com/polyeffects/poly_filters) | [ebf8fd1cea20](https://github.com/polyeffects/poly_filters/commit/ebf8fd1cea20e5d167f0c04ded12da13bd70829f) | 6 LV2 IDs |
| [polyconvo.lv2](https://github.com/polyeffects/polyconvo.lv2) | [fe8edd2da2e3](https://github.com/polyeffects/polyconvo.lv2/commit/fe8edd2da2e348ecc383c93c5d4e7961e7d8dc5b) | 6 LV2 IDs |
| [PolyLV2](https://github.com/polyeffects/PolyLV2) | [6f3ae80f6b4e](https://github.com/polyeffects/PolyLV2/commit/6f3ae80f6b4eccb8daebfc2b641fde1d48224250) | 33 LV2 IDs |
| [PolyMclkIn.lv2](https://github.com/polyeffects/PolyMclkIn.lv2) | [64acb88d6c57](https://github.com/polyeffects/PolyMclkIn.lv2/commit/64acb88d6c574b4efc743b770d1d22444391ba02) | 1 LV2 IDs |
| [serd](https://github.com/polyeffects/serd) | [ebe0b8697bca](https://github.com/polyeffects/serd/commit/ebe0b8697bca69abb7e7be1c731da4d54b10e462) | RDF parsing library |
| [sooperlooper](https://github.com/polyeffects/sooperlooper) | [0cf3001ed295](https://github.com/polyeffects/sooperlooper/commit/0cf3001ed295011714c059fa91d987835811a0f1) | SooperLooper LADSPA and Audio Unit; outside LV2 counts |
| [tuna.lv2](https://github.com/polyeffects/tuna.lv2) | [cfdfb6aeb065](https://github.com/polyeffects/tuna.lv2/commit/cfdfb6aeb065b278f58b6c6d4c23b5d82ae34279) | 3 LV2 IDs |
| [xmos_lib_si5351](https://github.com/polyeffects/xmos_lib_si5351) | [9c333896a18b](https://github.com/polyeffects/xmos_lib_si5351/commit/9c333896a18b37575e73d68a28ffa0be3a65ec8a) | Hardware clock library |

## Verification and limits

All four Patchstorage API pages were fetched (100 + 100 + 100 + 85), and all 385 entry IDs are unique. All 450 advertised plugin IDs are unique. Bundle expansion yields 552 unique IDs across 553 catalogue rows. The matching/difference counts reconcile to 47 + 505 = 552 and 47 + 160 = 207.

All 21 public Poly repositories were enumerated live. The local default-branch revisions of all 11 plugin-source repositories and `digit_ui` match GitHub. Source metadata was checked with an RDF parser: 120 static/template identities agree with an independent extraction; the three Tuna build identities were resolved from its Makefile. Build placeholders were expanded for inspection. Missing namespace declarations in some upstream metadata were supplied only to the inspection parser; upstream files were not edited.

The 385 Patchstorage entries include **161 Ready to Go** and **224 Work in Progress** entries. Advertised entry-level targets are **362 Pi64**, **329 PB32**, and **309 x64**; these counts overlap because an entry can offer several targets. Additional bundled IDs carry only the inspected build’s target.

AMS/CV counterpart annotations were checked against source code, port symbols/types and algorithms at the recorded MOD revisions. Similar names alone were not treated as proof. This is a catalogue and source comparison. Plugins were not compiled, installed, or audio-tested. All 385 entries had one selected downloadable archive inspected for plugin metadata. Other architecture downloads may differ; the document records extra IDs only for the target actually inspected. Source-only metadata declarations and website-only IDs remain included even when an actual working binary has not been verified. Other branches, historical tags, private firmware components and third-party source repositories outside Poly’s public organization are outside this inventory.

Primary machine-readable sources: [Patchstorage page 1](https://patchstorage.com/api/beta/patches/?platforms=8046&per_page=100&page=1), [page 2](https://patchstorage.com/api/beta/patches/?platforms=8046&per_page=100&page=2), [page 3](https://patchstorage.com/api/beta/patches/?platforms=8046&per_page=100&page=3), [page 4](https://patchstorage.com/api/beta/patches/?platforms=8046&per_page=100&page=4), [Poly repository list](https://api.github.com/orgs/polyeffects/repos?per_page=100). Entry and commit-specific source links are embedded throughout the lists.
