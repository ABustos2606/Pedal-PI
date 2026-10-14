# Linux build, Pi packaging and services

**Implemented:** 14 October 2026, on the Windows PC, without the Pi. **Scope:**
- the Linux build of the interface (P1.3) and the code side of its Pi display backend (P4.5);
- a reproducible arm64 build route and dependency baseline (P3.1, P3.2);
- the software side of the controls' hardware adapter (P7.3);
- installation and a restart-only-the-interface loop (P4.8);
- separate services with a deliberate restart policy (P10.1).

**Verified with:** the native Linux build and its tests under WSL2; the arm64 build and its tests under emulation; the installed services running with JACK's dummy driver; and evdev events from virtual devices. **Not covered:** anything on the Pi itself — its screen, touch, GPIO, audio interface and timing. Those wait for Phase 0.

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

- the prefix (engine, plugins, interface, catalogue exported from this build's plugins);
- the configuration in `config/pi`;
- `install.sh` and `check-services.sh`;
- the licenses.

On the Pi:

```bash
tar xzf pedalpi-arm64-….tar.gz && cd pedalpi
sudo ./install.sh                 # keeps the previous installation for --rollback
sudo systemctl start pedalpi.target
sudo bash check-services.sh       # optional: the same service checks, on JACK's dummy driver
sudo systemctl restart pedalpi-ui # after copying a new interface: audio keeps playing
```

The installer:

- checks the package's architecture against the machine's;
- installs the libraries the programs were built against;
- creates a `pedalpi` system account in the audio, video, input and GPIO groups that exist;
- moves any previous installation to `/opt/pedalpi.previous`;
- installs the settings `/etc/pedalpi/pedalpi.conf` and `controls.conf` once only, never overwriting them;
- installs the units.

It **never** edits `/boot/firmware/config.txt` and never stops MODEP, PipeWire or another JACK; it names any that is running, since they would compete for the audio interface (P0.6). `--remove` takes the services away and keeps patches and settings; `--rollback` restores the previous installation.

**Development loop.** Build on the PC (`build-arm64.sh --skip-engine` after the first time), copy the package over SSH, install, then restart only the interface. A Windows interface can also drive the Pi's engine over the network only if the engine is started with a TCP port. The installed service deliberately does not open one; a verified, firewalled transport is a separate decision.

## Services (P10.1)

| Unit | Runs | Restart policy |
| --- | --- | --- |
| `pedalpi-jack` | `jackd` with the driver and options of `pedalpi.conf` | On failure. A JACK restart restarts the engine (its client). |
| `pedalpi-engine` | Ingen on `/run/pedalpi/ingen.sock`, no TCP | On failure. Bound to JACK; independent of the interface. |
| `pedalpi-route` | Keeps the engine's IN and OUT wired to the ports named in `pedalpi.conf` | Always. It checks every two seconds, so new ports after any restart are wired again. |
| `pedalpi-ui` | The interface on DRM/KMS with the touch screen and controls | Always, after one second. It only *wants* the engine: either can restart without the other, and it reconnects by itself. |
| `pedalpi.target` | All of the above | `systemctl enable pedalpi.target` starts the instrument at boot. |

**Accounts and limits.** All run as `pedalpi`, with real-time limits granted to JACK and the engine (`LimitRTPRIO`, `LimitMEMLOCK`). The CPU governor, IRQ priorities and the rest of P10's readiness and boot-time work are not done.

## The interface on Linux (P1.3, P4.5)

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

- **Nothing here ran on a Pi.** DRM/KMS was compiled but not exercised (WSL has no DRM device), and no real Pi screen, touch controller or GPIO was involved.
- **Bench checks.** The overlays' names and steps per detent, and whether the panel offers 800 × 480, are to check on the bench. If it does not, a `video=` kernel argument may be needed.
- **`pedalpi.conf` placeholders.** `JACK_DRIVER_OPTIONS=-d hw:USB -r 48000 -p 128 -n 3` is a placeholder until the audio interface is chosen and measured (P0.3).
- **The startup patch.** It is loaded by the interface, so audio at boot waits for the interface (P10.2, P10.4).
- **Power loss.** An abrupt power loss during a save is still untested (P5.5/P10.5).
- **TCP after a quick restart.** An engine restarted with a TCP port immediately after a kill can fail to open it ([patch storage](17-patch-storage.md)). The installed engine has no TCP port.
