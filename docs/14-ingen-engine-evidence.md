# Ingen engine evidence

**Run:** 13 October 2026, on the Windows PC. **Environment:** Debian 13 (trixie) under WSL2, x86-64, JACK 1.9.22 dummy backend at 48 kHz / 128 frames. **Scope:** the hardware-independent parts of Phase 3 (P3.1, P3.5–P3.8) and the format question of P5.2. **Not covered:** the Pi, ARM builds, a real audio interface, latency, xruns under load, and every plugin other than the LV2 example amplifier.

Everything here is reproducible with `bash tools/ingen-spike/run-all.sh` as root inside that Debian. The recorded output is in [evidence/ingen-spike-results.txt](evidence/ingen-spike-results.txt): 34 checks passed, none failed.

## What was built

| Component | Version | Source |
| --- | --- | --- |
| Ingen | upstream `13a045d0` (6 June 2025) + Poly's MIDI-learn fix `8dae7404` + three project patches | cloned fresh; `reference/` is untouched |
| Raul | 2.0.0 | built from source, not packaged in Debian |
| Serd / Sord / Sratom / Suil / LV2 / Lilv | 0.32.4 / 0.16.18 / 0.6.18 / 0.10.22 / 1.18.10 / 0.24.26 | stock Debian packages |

**Poly's published Ingen `main` does not compile.** It is upstream Ingen plus three commits. Two of them (September 2025) switch the client socket from Turtle to JSON using `SERD_JSONTURTLE`, a serd syntax that exists in no published serd: Poly's public serd `master` is plain 0.32.3 and its `hextuples` branch adds only `SERD_HEXTUPLES`. The third commit is a two-line MIDI-learn fix.

Our interface is new and has no need for Poly's JSON, so the engine built here is the last upstream commit, which speaks Turtle with stock libraries, plus that MIDI-learn fix. **"Poly's Ingen fork" (D15) is therefore, in practice, upstream Ingen.** The serd snapshot under `reference/` is not needed.

## Findings

| Question | Result |
| --- | --- |
| Protocol (P3.5) | Turtle `patch:Get/Put/Set/Delete/Copy` messages over a Unix socket. Replies are NUL-terminated and carry the request's `patch:sequenceNumber` and an integer status (0 = success). The engine's own notifications reuse that predicate with unrelated numbers, so correlate on `patch:Response` blocks only. Nothing is sent on connect; the client asks with `Get`. |
| Add / remove / connect / set | All work and are audible. An unknown plugin URI is refused with status 22. |
| Control latency | A control change is heard after a median 5.6 ms (max 7.7 ms). |
| Fan-out | One output to several inputs works and leaves existing branches unchanged. |
| Fan-in (P3.7) | Several cables into one input are **summed at unity, with no compensation**: two 0.25 branches measured 0.5000, +6 dB. The interface needs its own merge rule (attenuate, or insert a mixer). |
| Feedback | The engine **accepts** a cable from a block's output to its own input. Our graph validation must keep refusing cycles; the engine will not. |
| Value checking | **None.** A string sent to a float control returns success and is stored; a gain of 9999 dB returns success and the output becomes `inf`. The client must type-check and clamp every value. |
| Reconnect (P3.8, R13) | Twenty orderly reconnects, a client that died mid-message and a `SIGKILL`ed client: the engine stayed up and the audio level never moved (568 consecutive blocks at 0.5000). A new client reads back the complete live graph. |
| Second client | A second connected client is told about the first one's changes. A separate control daemon and the interface can therefore both stay in sync. |
| Layout and custom data | `ingen:canvasX/Y` and arbitrary project properties (`urn:pedalpi:ns#displayName` on a block, `#assignedControl` on a port) are accepted, returned to a new client **before any save**, written into the saved bundle, and restored on load. |
| Save format (P5.2) | `patch:Copy` to `file:///…/name.ingen` writes a directory with `main.ttl`, `manifest.ttl` and `plugins.ttl`, and replies with status 0. |
| Load | Starting a new engine with that bundle restores blocks, cables, values, positions, custom properties and the JACK ports; audio passes at the expected level. |
| Parallel processing | The engine has a `-p/--threads` option. Not exercised here. |

## Engine defects found, and what was done

1. **Clients that leave are never forgotten.** `SocketServer::on_hangup()` was meant to unregister a departed client, but the socket reader it owns is a plain `SocketReader` whose virtual `on_hangup()` does nothing, so it was never called. Every interface restart left one more dead client that the engine wrote to on every change: 40 changes after 10 departures produced about 3,800 failed writes. This is what Beebo's README means by Ingen not coping with a frontend restart. **Fixed** by `patches/0001-unregister-clients-on-hangup.patch`; the same test now produces zero.
2. **Replies are up to 125 ms late.** Replies leave from the main loop, which sleeps 125 ms between passes. Audio is not affected, but every acknowledgment is. **Fixed** by `patches/0002-post-process-every-5ms.patch`: the request round trip went from a median 125 ms to 5.4 ms.
3. **A save can deadlock the engine.** The socket reader holds the RDF lock while it waits for the byte after whatever it last consumed. If a client ends its messages with a NUL (as Ingen's own Python client does), the reader is left parked with the lock held, the save waits for the lock, and no further request is ever answered. **Avoided on the client side:** send exactly one newline after the final `.` and no NUL. With that framing the save answers in under 0.1 s. This is a rule for our engine client, not an engine patch.

The next three were found on 13 October 2026 while building [the project's own adapter](15-ingen-adapter.md).

4. **A client that stops reading delays every other client.** Replies and notifications leave from one loop, in many small writes. A connected client that did not read for a moment filled its socket buffer after a few hundred bytes, the loop blocked in `send()`, and a block created by another client was answered only when the idle one read again. Audio is not affected. **Avoided on the client side:** the adapter reads on its own thread and never waits for the interface.
5. **A TCP client that closes its side leaves an engine thread at full load.** On a Unix socket a departed client is a hangup, which the reader handles. Over TCP it is an end of stream that stays readable for ever; the reader looped on it at 100% of a core. **Fixed** by `patches/0003-end-reader-at-end-of-stream.patch`; the engine now sits at 0% after TCP clients leave. The Pi uses the Unix socket, so this mattered for development from Windows.
6. **Every connection leaks a descriptor.** The engine never frees the object it makes for a client: 50 clients that came and went left 50 more open descriptors (22 before, 72 after) on the Unix socket, and departed TCP clients stay listed as half-closed. **Not fixed.** It bounds how often an interface can reconnect to one engine process, at roughly a thousand times with the usual limit.

Two rules for the client also came out of that work. A property set on the graph itself is accepted but left out of every later description unless it is sent with `patch:context ingen:internalContext`; with the context it is described, saved and restored. And `patch:Copy` of a block copies its values inside the engine but announces only the copy, not its contents, so the client must ask for them.

The engine also listens on TCP port 16180 on every interface, without authentication. That is convenient under WSL and must be closed or firewalled on the Pi.

All three patches change AGPL-3.0-or-later files and stay under that license. They are small enough to offer upstream.

## Consequences for the plan

- **D13 (companion JSON):** positions and control assignments can live on the blocks and ports inside the engine. That removes the two-file coherence problem of P5.5, and it means layout survives an interface crash, which a JSON file owned by the interface cannot offer. **Decided 13 October 2026: the companion file is dropped**; see [Architecture](02-architecture.md).
- **D15 (engine choice):** keep Ingen. The restart weakness was real, is understood, and has a small fix.
- **Engine client (P4.1):** no NUL after messages; correlate replies on `patch:Response`; validate and clamp before sending; refuse cycles locally; expect a full `Get` after connecting. Implemented and run against this engine: see [the adapter record](15-ingen-adapter.md).
- **Merge policy (P3.7):** decide between automatic attenuation and an explicit mixer before merges are enabled against the real engine.
- **Still open on the Pi:** ARM build of the same recipe, real-interface latency and xruns (WSL has no real-time scheduling, and the dummy backend xruns occasionally even when idle, so no xrun figure here is meaningful), the Mutable-derived plugins and their LV2 features (P3.6), and plugin state inside saved bundles.

## Files

| Path | Purpose |
| --- | --- |
| `tools/ingen-spike/01-build.sh` | Reproducible build into `/opt/pedalpi` |
| `tools/ingen-spike/patches/` | The three engine patches |
| `tools/ingen-spike/07-adapter.sh` | Builds the project's adapter and runs it against the live engine |
| `tools/ingen-spike/ingenprobe.py` | Minimal raw-socket protocol client |
| `tools/ingen-spike/04-verify.py`, `06-details.sh` | Measured checks |
| `tools/ingen-spike/05-save-probe.sh` | Demonstrates the save deadlock with and without a trailing NUL |
| `tools/ingen-spike/03-explore.py` | Prints raw protocol transcripts |
| `tools/ingen-spike/make-patches.sh` | Regenerates the patches from an upstream checkout |
