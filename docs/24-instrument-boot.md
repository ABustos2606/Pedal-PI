# Direct instrument boot

**Verified on the Raspberry Pi, 14 October 2026.** The Pi now boots directly into Pedal PI on its 800 x 480 touchscreen, restoring the saved **Clouds guitar** patch through the Tascam US-100. The user confirmed after reboot: "Yes, Pedal PI appears and touch works". No desktop login or MODEP launch is required.

## Installed boot policy

- Default target: `multi-user.target`; `pedalpi.target` enabled.
- LightDM, MODEP's JACK, every installed `modep-*` service and its update path are disabled and masked. Masking prevents dependency activation as well as ordinary boot activation.
- The automatic login on `tty1` is disabled and masked, so it cannot write a console over the instrument. Serial console and SSH remain available.
- UI uses `--display drm`; no desktop or browser is involved. No firmware configuration or kernel command line was changed.
- Programs, catalogue, plugins and patches remain on the card. No network-ready target is required by the instrument services. A boot with networking physically unavailable remains untested.
- JACK, Ingen, routing and UI remain separate services. Start limits are disabled for their retry policies, so late devices do not exhaust the retry allowance. JACK wants the engine so its automatic restart also brings back the engine stopped by `BindsTo`.

The selected startup identity is `p20261014-222121-f9e0`, currently revision `r000002.ingen`, with Freeze off. The UI restores the latest saved revision of this identity when a fresh engine is ready. Restarting only the UI retains a live patch and its unsaved edits. Use **Patches > Startup** to change the startup choice. Save changes that should survive an engine restart or power cycle.

Before reboot, the newer unsaved **wendy** patch was saved as `p20261014-224640-727c/r000001.ingen`. Its Tubewarmth, Chorus and Harmonic tremolo modules, five cables, controls and layout were verified against the saved bundle; saving did not rebuild its audio graph. It remains available in the patch manager alongside Clouds.

## Repeatable setup and recovery

Packages include `instrument-boot.sh`, installed as `/opt/pedalpi/bin/pedalpi-instrument-boot`. An ordinary installation preserves existing boot policy; `sudo ./install.sh --instrument` explicitly selects instrument mode. For an existing installation:

```bash
sudo /opt/pedalpi/bin/pedalpi-instrument-boot --dry-run
sudo /opt/pedalpi/bin/pedalpi-instrument-boot --startup p20261014-222121-f9e0
sudo reboot
```

The script saves the previous default target and each affected unit's enable/mask policy in `/etc/pedalpi/instrument-boot-state` before changing them. Repeating the command preserves that original snapshot. It never deletes MODEP, a saved patch, settings or plugins. A startup identity is checked before any boot-policy mutation; the UI performs its existing bundle/plugin/asset validation at restoration.

To return to the previous desktop/MODEP boot policy:

```bash
sudo /opt/pedalpi/bin/pedalpi-instrument-boot --restore --dry-run
sudo /opt/pedalpi/bin/pedalpi-instrument-boot --restore
sudo reboot
```

Restoring stops Pedal PI and restores the original unit policy and default target. Patches and the startup choice remain. Package removal also restores the saved boot policy before deleting Pedal PI's service units. The original Pedal PI unit files from this deployment are retained separately in `/etc/pedalpi/instrument-unit-backup`.

## Verification and limits

One software reboot was verified with a changed Linux boot identity. All five instrument units were active, the saved Clouds bundle matched the engine's identity, modules, four cables, controls and layout, and all four capture/playback connections were present. LightDM, MODEP and the `tty1` login were masked; no services were failed. The user independently checked the physical display and touch.

Recorded boot events, in the journal's monotonic seconds since kernel startup:

| Event | Seconds |
| --- | ---: |
| JACK started / responsive | 7.09 / 8.03 |
| Ingen service ready | 9.37 |
| UI process started | 9.40 |
| Saved Clouds bundle loading began | 10.33 |
| All four external audio connections established | 11.92 |

These are service and routing events, not measured power-on-to-audible-sound or power-on-to-responsive-touch times. Firmware time is excluded. `systemd-analyze` reported 16.00 seconds for kernel plus userspace overall; it includes other system services.

A separate deliberate JACK `SIGKILL` recovered automatically: JACK PID 736 became 2296, Ingen 894 became 2311, UI stayed PID 978. Within the eight-second observation window the saved Clouds patch matched the bundle again and all four Tascam connections were restored. This is audio-service recovery with an interruption, distinct from the earlier uninterrupted UI-only restart test.

A passive 22-second post-boot monitor observed 8,250 blocks with no frame gaps or xruns and finite samples throughout. It also recorded 96 left and 86 right exactly silent blocks, so this run is **not** a sustained-playing/no-silence acceptance test. A subsequent audit saw a dirty graph during the touch session; the later recovery audit verified the saved revision. No audio was injected or recorded by the monitor.

Phase 10 remains partial: physical offline boot, five cold power cycles, late physical USB availability, native UI-crash/controller recovery, missing-plugin/asset startup cases and shutdown/power-loss storage acceptance remain open. Existing undervoltage history (`throttled=0x50000`) remains a performance limitation.

Evidence: [instrument boot results](evidence/pi-instrument-boot-results.txt). The boot utility and service changes were deployed without rebuilding or replacing the engine, plugins or touchscreen application; the patch-preservation CLI was compiled natively against the already deployed core library. Bash syntax and systemd unit verification passed on the Pi.
