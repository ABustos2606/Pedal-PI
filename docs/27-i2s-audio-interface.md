# I2S audio interface

**Assessment, 15 October 2026.** A Pi 5, the screen and a USB audio interface make the enclosure too large. Gemini suggested replacing the USB interface with generic I2S converter modules that are easy to buy in Argentina: a **PCM5102A** DAC, a **PCM1808** ADC and a small op-amp guitar buffer. The approach is sound and would let Peepo fit something near a Hammond 1590XX (about 145 × 121 mm) or 1590DD (about 188 × 120 mm). Gemini's write-up, though, misses the hardest part and gets some details wrong.

Nothing here is built or measured yet. The Tascam US-100 remains the verified audio path ([Pi bring-up](23-pi-bring-up.md)).

## Where Gemini is right

- **PCM5102A DAC.** One of the most common DIY Pi DACs. It wires to the Pi 5's I2S pins and runs on the stock `hifiberry-dac` overlay. It needs no master clock (MCLK): with the module's SCK pin tied to GND, its internal PLL derives the clock from BCLK. Output is about 2.1 Vrms line level, fine into an amp input or effects return. It cannot drive headphones.
- **PCM1808 ADC.** 24-bit stereo, cheap, and good enough for guitar.
- **A buffer before the ADC.** Necessary. A passive pickup loaded by the module's low input impedance loses its treble.
- **Size and latency.** The converter boards and the buffer together take about the space of a matchbox. I2S has no USB buffering, so latency should be equal to or lower than the US-100's.

## Where to push back

1. **The PCM1808 needs a master clock, and the Pi does not output one.** This is the main omission. The PCM1808 needs a system clock on SCKI at 256, 384 or 512 × fs, e.g. 12.288 MHz for 48 kHz. The Pi's I2S has no MCLK pin. The fix:
    - Add a 12.288 MHz (or 24.576 MHz) crystal oscillator module to the PCM1808's SCKI.
    - Set the PCM1808 as **clock master** with its MD0/MD1 pins, so it generates BCLK and LRCLK.
    - The Pi and the PCM5102A are both clock slaves on that BCLK/LRCLK.

   One clock source then drives everything: no drift between input and output, no periodic clicks. Deriving SCKI from a Pi general-purpose clock (GPCLK) instead is not locked to the I2S clocks and is not recommended.
2. **`hifiberry-dac` is playback only.** It creates an output-only card. Full duplex with the Pi as clock slave needs a small custom device-tree overlay, likely `simple-audio-card` with generic codecs. JACK then points at that card instead of `hw:CODEC`; Ingen is unaffected.
3. **"0 to 3.3 V signal, bias to 1.65 V" is wrong for this chip.** The PCM1808 biases its inputs internally and the modules have coupling capacitors, so the buffer output is simply AC-coupled in. Full scale is about 3 Vpp with the analog supply at 5 V. A guitar gives roughly 100 mV to 1 V peak, so the buffer should add some gain (about 2–4×, ideally switchable) to use the ADC's resolution.
4. **A TL072 on the Pi's 5 V rail is a poor choice.** The TL072 (and NE5532) is not rail-to-rail and barely works at 5 V. Either:
    - run it from **9 V with a 4.5 V virtual ground**, the usual pedal approach. That implies a 9–12 V pedal supply with a buck converter giving the Pi 5 its 5 V at up to 5 A, replacing the official 27 W USB-C unit; or
    - use a rail-to-rail op-amp on 5 V, such as an MCP6002 or TLV2372.

   The input should present about 1 MΩ to the guitar.
5. **Noise.** The Pi 5 is digitally noisy. Give the PCM1808's analog 5 V its own LDO or LC filter, keep analog wiring short, use a single ground point, and route the input jack wiring away from the Pi.

## Summary

| Part | Usable | Catch |
| --- | --- | --- |
| PCM5102A | Yes, stock overlay | Tie SCK to GND; check the XSMT (unmute) jumper |
| PCM1808 | Yes | Needs its own oscillator and must be clock master |
| Pi overlay | Yes | Custom full-duplex overlay with the Pi as slave, not only `hifiberry-dac` |
| Op-amp buffer | Yes | TL072 from 9 V with 4.5 V bias, or a rail-to-rail op-amp on 5 V; add gain |

## Wiring outline

Pi 5 I2S pins (header pin numbers in brackets):

| Signal | Pi GPIO | PCM1808 | PCM5102A |
| --- | --- | --- | --- |
| BCLK | GPIO18 (12), input | BCK, output | BCK, input |
| LRCLK | GPIO19 (35), input | LRC, output | LCK, input |
| Capture data | GPIO20 (38), input | OUT | — |
| Playback data | GPIO21 (40), output | — | DIN |
| System clock | — | SCKI ← 12.288 MHz oscillator | SCK → GND |

The [physical controls](18-physical-controls.md) plan already leaves GPIO 18–21 free for I²S, so the encoders and stomps are unaffected. Stomp 1 uses GPIO 4 (GPCLK0), which is fine because this design does not use a Pi clock output.

## Next steps

- [ ] Breadboard the PCM5102A alone with `hifiberry-dac` and play audio through JACK.
- [ ] Add the PCM1808 as clock master with the oscillator; write and load the full-duplex overlay.
- [ ] Move JACK to the new card at 48 kHz / 128 × 3 and check for xruns and drift over a long run.
- [ ] Build the buffer (1 MΩ input, switchable gain) and compare noise and tone against the US-100.
- [ ] Measure round-trip latency against the US-100.
- [ ] Decide the power design (9–12 V input and buck converter, or 5 V only) and then the enclosure.
