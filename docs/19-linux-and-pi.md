# Linux build, Pi packaging and services

**Implemented:** 14 October 2026, on the Windows PC, without the Pi. **Scope:**
- the Linux build of the interface (P1.3) and the code side of its Pi display backend (P4.5);
- a reproducible arm64 build route and dependency baseline (P3.1, P3.2);
- the software side of the controls' hardware adapter (P7.3);
- installation and a restart-only-the-interface loop (P4.8);
- separate services with a deliberate restart policy (P10.1).

**Verified with:** the native Linux build and its tests under WSL2; the arm64 build and its tests under emulation; the installed services running with JACK's dummy driver; and evdev events from virtual devices. **Original PC record excludes:** the Pi itself — its screen, touch, GPIO, audio interface and timing. Their first run on the Pi and the later guitar/UI restart check are recorded in [Pi bring-up](23-pi-bring-up.md).

## What was verified

| Check | Result |
| --- | --- |
| Linux interface | Builds without warnings with DRM/KMS, framebuffer and SDL. All 8 Linux test suites pass, including the full LVGL workflow checks. The program renders offscreen and in an SDL window under WSLg. |
| Controls on Linux | Bindings and event translation are unit-tested. A virtual encoder and footswitch made through `uinput` are read through the real evdev path, and a virtual touch screen is found and left alone. |
| arm64 build | Engine, plugins and interface built in a Debian 12 arm64 root under qemu (engine and plugins 22 min the first time). All 8 test suites pass on the arm64 binaries, the offscreen render matches the x86 one, and the catalogue export finds 65 plugins (59 offered). The live adapter checks pass against the arm64 engine over the UNIX socket and TCP. They include a patch saved and loaded from disk, an engine restart, and the level through a patch built by the adapter (0.25 in; 0.5000 out through the amplifier, 0.2500 direct). The package is 24 MB. Emulated timing (6.4 ms median edit round trip) says nothing about the Pi. |
| Services | Installed from the package with `install.sh` and started with JACK's dummy driver. Audio passes through a patch built in the installed engine (0.25 in, 0.5000 out, all 750 periods). Killing the interface during playback left every one of 750 measured periods at 0.5000, restarted only the interface, and it reconnected. An engine restart left the interface running; it reconnected and the routing was redone. The engine listens on no TCP port. Stopping the instrument stops every part. |
| Rollback | `install.sh --remove` then `--rollback` restored the previous installation. |

Recorded output: [evidence/linux-and-pi-results.txt](evidence/linux-and-pi-results.txt).

## What the arm64 build found

1. **Debian 12's lilv is too old.** This Ingen needs lilv 0.24.21 or later; Debian 12 has 0.24.14. The build now makes lilv 0.24.24, with the zix 0.4.2 it needs, inside `/opt/pedalpi` when the system's is older.
2. **The engine writes relative paths with Debian 12's serd.** Built against serd 0.30 it sends `</main/m3>` where the WSL engine (Debian 13) sends `<ingen:/main/m3>`, both relative to its base `<ingen:/>`. The adapter's Turtle reader took them literally and never saw cables or blocks, so a new patch's cables were invisible and re-creating them was refused. The reader now resolves relative references against `<ingen:/>`, as RFC 3986 describes; a stand-in engine that writes this way is part of the adapter tests, together with the recorded message.
3. **JACK's example tools moved.** `jack_lsp` and `jack_connect` are their own package since Debian 13 and part of `jackd2` before; the scripts install whichever exists.

## Build route (P3.1, P3.2)

**One script builds everything into `/opt/pedalpi`.** `tools/pi/build.sh` builds the engine, the plugins and the interface, runs the tests and makes a package. It runs on Debian or Raspberry Pi OS:

- **natively on the Pi** (the route once the Pi is unpacked);
- **on the PC**, in a Debian arm64 root under emulation: `tools/pi/arm64-root.sh` creates the root with debootstrap and qemu, and `tools/pi/build-arm64.sh` runs `build.sh` inside it.

**Emulation over cross-compiling.**
- It needs no cross toolchain or sysroot.
- It uses the very packages the Pi's distribution has.
- The tests run on the arm64 binaries themselves.
- The price is speed: the engine takes most of an hour the first time, later builds are incremental. Emulated timing says nothing about the Pi's.

**The suite must match the Pi's.**
- The root is Debian 12 (bookworm) by default, the base of Raspberry Pi OS bookworm. Binaries need the C library they were built against, or newer.
- Once Phase 0 has identified the Pi's release, set `PEDALPI_SUITE` to match.
- Patchbox OS was based on older releases; if it is still what the Pi runs, the plan's choice (P0.6) is between building for it and installing a current Raspberry Pi OS on a separate card.

**Dependency baseline**, recorded by the build:

| Component | Source | Version |
| --- | --- | --- |
| Ingen | Upstream `13a045d0` + Poly's MIDI-learn fix + our four patches | as [the engine evidence](14-ingen-engine-evidence.md) |
| lilv | Built into the prefix when the system's is older than 0.24.21 (Debian 12 has 0.24.14) | v0.24.24 |
| zix | Needed by that lilv | v0.4.2 |
| raul | Not packaged | v2.0.0 |
| serd, sord, sratom, suil, lv2, JACK, Boost | Debian | the suite's |
| Mutable-derived plugins | Lenticular at the pinned commit | built with `-mcpu=cortex-a76` instead of the makefile's SSE flags |
| Interface | This repository | LVGL v9.5.0, pinned |

**Engine patch 0004.** The engine's TCP port listens on every interface without authentication, and Ingen had no way to turn it off. With this patch, `--engine-port 0` disables it. The Pi's service uses that; the UNIX socket is all the instrument needs. The patch also makes the listener poll its sockets in fixed slots: it read an uninitialized slot when only one socket was open.

## The package and installation (P4.8)

`build/pi/pedalpi-<arch>-<date>-<version>.tar.gz` holds:

- the prefix (patched engine, Mutable plugins including Clouds, the selected current NAM plugin, the full selected plugin inventory, interface and catalogue);
- the configuration in `config/pi`;
- `install.sh`, `instrument-boot.sh`, `export-catalogue.sh` and `check-services.sh`;
- the licenses.

On the Pi:

```bash
tar xzf pedalpi-arm64-….tar.gz && cd pedalpi
sudo ./install.sh                 # keeps the previous installation for --rollback
sudo ./install.sh --instrument    # direct boot; saves and replaces desktop/MODEP boot policy
sudo systemctl start pedalpi.target
sudo bash check-services.sh       # optional: the same service checks, on JACK's dummy driver
sudo systemctl restart pedalpi-ui # after copying a new interface: audio keeps playing
```

The installer:

- checks the package's architecture against the machine's;
- installs the libraries the programs were built against;
- installs the distribution plugins from the selected inventory and regenerates the catalogue on the Pi with the same plugin paths the engine uses;
- creates a `pedalpi` system account in the audio, video, input and GPIO groups that exist;
- moves any previous installation to `/opt/pedalpi.previous`;
- installs the settings `/etc/pedalpi/pedalpi.conf` and `controls.conf` once only, never overwriting them;
- installs the units.

It **never** edits `/boot/firmware/config.txt`. Ordinary installation reports competing audio services. Explicit `--instrument` invokes the boot utility to disable/mask LightDM, MODEP and its JACK and enable direct instrument startup, with the previous policy saved for restoration. `--remove` restores that saved policy if present, takes the services away and keeps patches and settings; `--rollback` restores the previous program installation. See [direct boot and recovery](24-instrument-boot.md).

**Development loop.** Build on the PC (`build-arm64.sh --skip-engine` after the first time), copy the package over SSH, install, then restart only the interface. A Windows interface can also drive the Pi's engine over the network only if the engine is started with a TCP port. The installed service deliberately does not open one; a verified, firewalled transport is a separate decision.

`--skip-engine` reuses all installed engine and plugin binaries; omit it when plugin sources or engine patches change. A full build now runs `02-plugins.sh`, `12-nam.sh` and `14-more-plugins.sh`, and refuses a failed catalogue component. The installed `sudo /opt/pedalpi/bin/pedalpi-export-catalogue` can refresh the catalogue after plugin changes; restart `pedalpi-ui` to read it.

## Services (P10.1)

| Unit | Runs | Restart policy |
| --- | --- | --- |
| `pedalpi-jack` | `jackd` with the driver and options of `pedalpi.conf` | On failure, without exhausting a start limit. Wants the engine, so automatic JACK recovery also starts its client again. |
| `pedalpi-engine` | Ingen on `/run/pedalpi/ingen.sock`, no TCP | On failure. Bound to JACK; independent of the interface. |
| `pedalpi-route` | Keeps the engine's IN and OUT wired to the ports named in `pedalpi.conf` | Always. It checks every two seconds, so new ports after any restart are wired again. |
| `pedalpi-ui` | The interface on DRM/KMS with the touch screen and controls | Always, after one second. It only *wants* the engine: either can restart without the other, and it reconnects by itself. |
| `pedalpi.target` | All of the above | `systemctl enable pedalpi.target` starts the instrument at boot. |

**Accounts and limits.** All run as `pedalpi`, with real-time limits granted to JACK and the engine (`LimitRTPRIO`, `LimitMEMLOCK`). The CPU governor, IRQ priorities and the rest of P10's readiness and boot-time work are not done.

**Direct boot verified on the Pi, 14 October 2026.** The target is now enabled with `multi-user.target` as default and desktop/MODEP/automatic-console competitors masked. A software reboot restored Clouds and all four Tascam connections; the user confirmed the display and touch. Deliberate JACK failure recovered both audio services and the saved patch while the UI stayed running. Repeated cold boots, physical offline startup and the rest of Phase 10 remain open. [Boot record and recovery](24-instrument-boot.md).

## The interface on Linux (P1.3, P4.5)

**Real guitar/UI restart verified on the Pi, 14 October 2026:** a saved and reloaded Clouds patch kept playing while `systemctl restart pedalpi-ui` replaced only the interface process. JACK and Ingen kept their PIDs, the interface reconnected to the same saved graph, and the user heard no interruption. A passive 22-second monitor recorded 8,251 continuous blocks without xruns or silent output blocks. See [Pi persistence evidence](23-pi-bring-up.md#patch-saveload-and-interface-restart-with-guitar).

For a deliberate repeat with a player sustaining guitar, the installed native tools are:

```bash
sudo -u pedalpi /opt/pedalpi/libexec/pedalpi/pedal_pi_patch_live save-load
sudo bash /opt/pedalpi/libexec/pedalpi/check-ui-restart.sh
```

The first command saves a new copy of the current patch and reloads it, leaving it playing. For frozen Clouds it retains revision 1 and saves a Freeze-off revision 2 for live input; the captured buffer is not persistent. The second monitors the existing audio ports, restarts only the interface, and compares the graph with the recorded baseline. It stores aggregate measurements, not audio. The checker is built from `tests/pi_patch_live.cpp` and uses the application's real adapter and patch store.

`pedal_pi` takes `--display drm|fbdev|sdl|offscreen`. Without it: SDL when a desktop is present, else DRM, else the framebuffer.

**On the Pi's screen**
- **Display.** DRM/KMS picks the first card with a connected display, and an exact 800 × 480 mode when the panel offers one; otherwise the panel's preferred mode. `--rotate` turns the display.
- **Touch.** The first input device that reports touch positions is used as the touch screen (or `--touch PATH`), calibrated from the device's own ranges.

**Physical controls**
- **Reading.** They are read from the kernel's input devices according to `/etc/pedalpi/controls.conf` ([physical controls](18-physical-controls.md)).
- **The proposed wiring.** The kernel's `rotary-encoder` and `gpio-key` overlays give each encoder its own relative axis and each push or footswitch its own key code (`config/pi/config.txt`). Bindings therefore never depend on device numbers.
- **Hotplug.** A device that disappears is dropped and found again within two seconds.
- **Desktops and WSL.** Under SDL the same keys stand in for the controls as on Windows.

## Limits and open points

- **On the Pi.** The package, DRM/KMS output and touch were first run on the Pi on 14 October 2026 ([Pi bring-up](23-pi-bring-up.md)). GPIO controls are still untried.
- **Bench checks.** The overlays' names and steps per detent, and whether the panel offers 800 × 480, are to check on the bench. If it does not, a `video=` kernel argument may be needed.
- **`pedalpi.conf` placeholders.** `JACK_DRIVER_OPTIONS=-d hw:USB -r 48000 -p 128 -n 3` is a placeholder until the audio interface is chosen and measured (P0.3). The Pi currently uses `-d hw:CODEC` for the Tascam US-100.
- **The startup patch.** Clouds guitar is selected and restored on boot by the interface, so audio still waits for the interface. Missing-plugin/asset startup validation on Pi remains P10.4.
- **Power loss.** An abrupt power loss during a save is still untested (P5.5/P10.5).
- **TCP after a quick restart.** An engine restarted with a TCP port immediately after a kill can fail to open it ([patch storage](17-patch-storage.md)). The installed engine has no TCP port.
