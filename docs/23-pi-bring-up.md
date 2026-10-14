# Pi bring-up: profile and first sound

**Recorded:** 14 October 2026, on the Pi itself, over SSH from the Windows PC. **Scope:** the Phase 0 profile (P0.1–P0.4, P0.6), and the first run of the Pedal PI package on the Pi (P3.3, the first part of P3.4, P4.5 and P4.7). **Raw output:** [evidence/pi-bring-up-results.txt](evidence/pi-bring-up-results.txt).

**Result.** Guitar → Tascam US-100 → Ingen → Tascam, edited by touch on the Pi's own screen in the Pedal PI interface. The user drew two cables, IN to OUT, and heard the guitar. This is the first pass of Checkpoint B on the real hardware.

## The Pi as found (P0.1–P0.4)

| | Found |
| --- | --- |
| Board | Raspberry Pi 5 Model B Rev 1.0, 2 GB RAM, active fan (`pwm-fan`) |
| System | Patchbox OS on Debian 12 (bookworm), arm64 userland, 64-bit kernel `6.6.74+rpt-rpi-2712` (PREEMPT, not real-time), glibc 2.36 |
| Storage | 32 GB microSD, 6.5 GB used |
| CPU | `ondemand` governor, 2.4 GHz maximum |
| Power | **Undervoltage.** 4.89 V on the 5 V rail at idle; the kernel reported undervoltage twice within two hours, and the firmware flags (`throttled=0x50000`) say undervoltage and throttling occurred. The supply is not adequate for a Pi 5. |
| Network | Wi-Fi only in use (the new PC has no Ethernet port); weak signal, about 225 ms round trip with occasional loss. SSH as `patch` with a key; passwordless sudo. |
| Display | HDMI panel, connector `HDMI-A-1`, **800 × 480** mode, DRM/KMS (`vc4-kms-v3d`). A Wayfire desktop ran on it under LightDM. |
| Touch | USB `QDtech MPI5001` (0484:5750), recognized as a touch screen. |
| Audio interface | **Tascam US-100**: a TI PCM2900B (08bb:29b0), ALSA name `CODEC`. USB 1.1 full speed, 2 in / 2 out, 16 bit; playback 32/44.1/48 kHz, stereo capture at 48 or 44.1 kHz. |
| Controls | Not connected yet (P0.5 open). |

**Services found (P0.2).** MODEP 1.13 (`jack`, `modep-mod-host`, `modep-mod-ui` enabled), PipeWire and WirePlumber in the user session, Pisound software (no Pisound board fitted), 48 LV2 bundles in `/var/modep/lv2`. MODEP's JACK was configured for `hw:CODEC` at 44.1 kHz, 128 × 2. It had failed at boot because the interface was unplugged, so MODEP was not running. WirePlumber holds only the cards' mixer controls, not their audio streams, so JACK can open the Tascam.

## Preservation (P0.6)

The user decided against a backup. The card holds only earlier experiments. The Pedal PI installation is separate in any case:

- programs in `/opt/pedalpi`, settings in `/etc/pedalpi`, patches in `/var/lib/pedalpi`, a `pedalpi` system account;
- `mda-lv2` and `lv2-examples` installed from the distribution;
- nothing of MODEP removed or disabled. For the test, LightDM was stopped (not disabled) so the interface could own the screen.

**Initial recovery policy (superseded):** MODEP and LightDM were initially left enabled. The user's later direct-boot request is now implemented: Pedal PI starts at boot and those competitors are masked. To return to the saved earlier boot policy, use `sudo /opt/pedalpi/bin/pedalpi-instrument-boot --restore`, then reboot. See [direct instrument boot](24-instrument-boot.md).

## First run of the package

The arm64 package built on the PC under emulation on 14 October installed and ran unchanged. Only `/etc/pedalpi/pedalpi.conf` was changed:

```
JACK_DRIVER_OPTIONS=-d hw:CODEC -r 48000 -p 128 -n 3
```

| Check | Result |
| --- | --- |
| JACK | Real-time priority 70, 48 kHz, 128 frames (2.7 ms) × 3 periods, 16-bit both ways |
| Engine | Ingen on `/run/pedalpi/ingen.sock`, TCP disabled; 72 plugins loaded. Lilv rejects Ingen's own `MonoInstrument` and `StereoInstrument` descriptions (missing ports), which the instrument does not use. |
| Routing | `pedalpi-route` wired `ingen:in_l/in_r` to `system:capture_1/2` and `ingen:out_l/out_r` to `system:playback_1/2` as soon as the interface created them |
| Interface | `--display drm` on the HDMI panel, touch found automatically; the editor appeared upright, and touch landed where the user pressed |
| Sound | The user connected IN to OUT on the screen and heard the guitar |
| Load (idle patch) | jackd 2.6–3.7 % CPU, Ingen 0.6 %, interface 0.7 %; 480 MB of 2 GB memory used in total; 49 °C |
| Xruns | 2 since JACK started, both "client = ingen was not finished", during the editing session; not yet tied to a cause |

## Today's package brought to the Pi

The rebuilt `pedalpi-arm64-20261014-c91d0b9-dirty.tar.gz` is installed. It includes engine patch 0005, Clouds, both isolated NAM candidate builds (only the selected current implementation is on the engine's plugin path), and all buildable components of the selected catalogue. The package is 50,856,981 bytes. All eight arm64 test suites and the 800 x 480 offscreen smoke check passed before installation.

The installer regenerated the catalogue **on the Pi itself**: 226 inventory entries, 186 available and 40 withheld before the presentation overlay. All installed binary and plugin libraries resolved on the Pi. The existing Tascam configuration and patch directory were preserved; the previous program installation is in `/opt/pedalpi.previous`.

Native checks passed for the six offered Clouds modes: ports, controls, default switches and bypass. Beat repeat remains withheld for its known upstream limitation. The selected NAM plugin also loaded and matched its four connectors and three controls; no model or NAM performance benchmark was tested here, and its file selector remains pending.

Clouds processed a generated stereo signal through the real engine at 48 kHz: 0.04646 / 0.04546 RMS out for a 0.17678 RMS sine in, with finite samples. The test signal was disconnected afterwards. The instrument target, JACK, engine, routing and touchscreen services are active. The final graph was read back after the interface started: **stereo IN -> Clouds -> stereo OUT**, named **Clouds guitar**, blend 0.35, pitch 0, freeze off. At the end of deployment this patch was unsaved; the subsequent guitar listening and persistence results are recorded below.

**Guitar listening confirmed, 14 October 2026.** After the update, the user played guitar through Clouds on the Pi and reported: "it's working!" This is the first musical milestone: guitar -> Tascam US-100 -> Clouds in Ingen -> Tascam, with the Pi touchscreen interface. The earlier generated-signal check is now complemented by actual listening. This confirms the live path; it does not claim a listening test of every mode, all controls, or combined-load performance.

Recorded output: [Pi update results](evidence/pi-update-results.txt). The Pi-generated catalogue is also retained locally in `build/pi/catalogue-pi-20261014.json`. Known malformed upstream metadata and catalogue exclusions remain as documented in the plugin catalogue; this update is not a full listening or compatibility test of every plugin on the Pi.

## Patch save/load and interface restart with guitar

**Verified on the Pi, 14 October 2026.** The current Clouds patch was saved through the application's Ingen adapter and `PatchStore`, published on the card, deliberately changed and loaded again. Its stored identity, name, module, four stereo cables, layout and all eleven visible controls came back; numerical controls were compared to 1e-6. No physical-control assignments were present.

The patch is **Clouds guitar**, identity `p20261014-222121-f9e0`, in `/var/lib/pedalpi/patches/p20261014-222121-f9e0/`:

- `r000001.ingen` preserves the user's original settings, including Freeze on.
- `r000002.ingen` keeps those settings with Freeze off for live guitar input. This is the current saved and loaded revision; there are no unsaved edits.

The original save took about 10 ms; the live-input save took 12 ms, and its reload took 60 ms in this run. Reloading a patch rebuilds its DSP. **Clouds does not save its captured/frozen buffer**, so restored controls do not restore a frozen sound. Revision 1 preserves the controls, not that audio buffer.

While the user played guitar, only `pedalpi-ui` was restarted. JACK stayed PID `108820`, Ingen stayed PID `108838`, and the interface changed from PID `108894` to `126356`. A passive JACK client monitored the actual capture and processed output ports for 22 seconds: **8,251 blocks, 1,056,128 frames, zero callback frame gaps, zero reported xruns, no silent output blocks and finite audio throughout**. Minimum per-block output RMS was 0.00673 left / 0.00769 right. It injected no test signal and saved no guitar recording.

The restarted interface reconnected. A new adapter client read back the same patch identity, name, module, cables, controls, positions and clean saved status. The user independently confirmed **"No interruption"**. All five instrument units remained active. This establishes audio continuity for this run, not a long-duration reliability or performance budget.

Evidence: [patch persistence and UI restart results](evidence/pi-persistence-results.txt). The repeatable check lives in `tests/pi_patch_live.cpp`, `tools/pi/check-ui-restart.sh` and `tools/pi/monitor-ui-restart.py`; the native tools are retained under `/opt/pedalpi/libexec/pedalpi/`.

## Open points

- **Power supply first.** No latency, xrun or load measurement is meaningful while the Pi throttles for undervoltage. A 5.1 V / 5 A USB-C supply (the official 27 W one) is needed.
- **The xruns.** Find out whether they come from graph edits, plugin instantiation or the undervoltage throttling, once the supply is fixed.
- **Latency and periods (P0.3, P3.4).** Measure round-trip latency and xruns at 128 × 3, 64 × 3 and 128 × 2. The US-100 is USB 1.1 and 16-bit; it is fine for development, its guitar input level and noise are still to judge.
- **Ownership of the screen and audio at boot: implemented.** Pedal PI is enabled, LightDM/MODEP and the automatic console login are masked, and a reboot restored Clouds with all audio connections. The user confirmed the screen and touch. Offline and repeated physical cold boots remain Phase 10 acceptance; see [direct instrument boot](24-instrument-boot.md).
- **Not done on the Pi yet:** controls (P0.5, P7.3), the MODEP baseline (P0.7; MODEP was not running), file-based plugin-state/asset persistence, and power-loss recovery tests. Guitar through Clouds, patch save/load and an interface-only restart during playing are now verified above.
