# Domain and engine contracts

**Implemented:** 13 October 2026. These contracts are used by the Windows editor and its deterministic simulated engine. The UI depends on `Engine`, rather than the concrete `SimulatedEngine`. They establish the adapter boundary; they do not implement Ingen's protocol.

## Identity and metadata

`InstanceId` identifies a module instance; plugin URI identifies its definition; the display name is independent of both. IDs are nonzero, unique in a graph, and not reused by ordinary add/duplicate operations. Duplicate copies parameters and software bypass into a new instance, leaving cables and control assignments with the original. Only the input/output endpoint definitions may be fixed; they cannot be deleted, moved, bypassed or duplicated.

Ports have logical symbols, input/output direction, explicit `PortType`, channel count and optional per-channel engine symbols. Each module can retain a port snapshot. This preserves a missing module's saved graph topology even when its definition cannot be resolved. Only mono/stereo audio is routed in this version; Control and Midi are represented in metadata but refused as cable endpoints.

Parameters are tagged Continuous, Integer, Toggle or Enumeration. The canonical numeric payload is a `double`; discrete tags require exact integral values, bounded to integers representable exactly by that payload. Toggles have range 0–1; enumeration values are zero-based indices into their choices. Ranges, values and logarithmic minima must be finite and valid. Symbols are unique per module. Domain validation runs before an externally loaded graph replaces the active graph; rejected commands retain the previous state.

Catalogue availability is explicit: Available, Untested or Unavailable, with a reason for disabled entries. All available entries in this build are labelled demo entries. A missing definition remains a movable/removable placeholder with retained settings. Parameter/bypass/duplicate/new-cable commands for unavailable instances fail; their existing logical cables are retained but have no active routing plan.

## Logical cables and routing plans

`Connection` is one logical cable between module IDs and logical port symbols. `RoutingPlan` lists its `PhysicalConnection` records, or reports an error. The LVGL cable view retains this plan separately from its line coordinates. Moving/panning never alters routing.

| Logical connection | Physical plan |
| --- | --- |
| Mono → mono | One output to one input |
| Mono → stereo | Same mono output to both input channels |
| Stereo → stereo | Left → left; right → right |
| Stereo → mono | Left and right to the same mono input |

Explicit channel symbols take precedence. Demo ports without them use the mono logical symbol or stereo `_L`/`_R` suffixes. A real metadata adapter must supply actual plugin/engine symbols; these demo defaults are not verified Ingen port identities. Stereo → mono and parallel merge plans express multiple links without gain compensation. Actual host mixing and levels remain a Phase 3 measurement, and this simulator executes no audio connections.

Graph validation enforces endpoint identity, unique module/parameter/port symbols, finite typed values, assignment ranges, existing cable endpoints, direction/type/channel compatibility, no duplicate logical cables and no feedback cycles. Missing modules' retained ports allow an incomplete graph to be restored for repair. New connections to them remain disabled.

Fan-out and fan-in are supported by the same connection rules as a series chain; no special parallel cable type is required. `parallel_demo_graph()` supplies five stereo logical cables (ten expanded channel connections), with Drive feeding Clouds and Delay and both feeding OUT. Removing one cable leaves the other branch intact. This fixture demonstrates graph routing, not a verified audio summing policy.

## Lifecycle and commands

`Engine::connect()` and `disconnect()` operate the connection lifecycle. The simulator normally starts Ready; reconnect passes through Connecting for 100 ms. Commands submitted outside Ready receive a correlated failure and are never queued for later application. Disconnect/error cancels queued edits and emits correlated failures, then a state/snapshot event. Reconnect keeps the last accepted graph and never applies cancelled edits.

`submit(Command)` returns a request ID. `advance(milliseconds)` drains events; each command result has its ID, accepted/failure state, message, current engine state and authoritative graph snapshot. State changes and simulated external graph replacements use request ID zero. The application tracks pending IDs, commits snapshots rather than optimistic mutations, and defers widget rebuilds until input gestures end.

Commands cover add, remove, move, duplicate, parameter, software bypass, connect/disconnect ports, load an external graph, create, rename, save and restore a session snapshot. New patch creates fixed IN/OUT with a direct cable and a dirty marker. The UI guards replacement of unsaved edits and waits for pending work. Create/rename names reject blanks/control characters and are bounded; newly typed names are limited to 48 characters and the domain accepts at most 96 UTF-8 bytes. Rename preserves an existing backend name longer than 48 characters until the user edits it, rather than silently truncating it.

`SaveGraph` specifies Session or Persistent scope and whether plugin state is requested. Session save runs through the same delayed/rejectable engine command path, retains one in-memory graph snapshot, and clears the active graph's dirty marker only after success. `LoadSession` restores that snapshot, which survives new-patch creation and reconnect during the process. Engine events return the saved graph for UI preview. Closing the process clears it. A rejected save changes neither the saved snapshot nor dirty state.

`EngineCapabilities` reports simulation, session save, persistent save, plugin-state capture and audio processing. The simulator supports the first two; persistent save and plugin-state commands return explicit unsupported-capability failures. No files or plugin state are written. Optional simulation controls default to unsupported in the abstract interface; the simulator implements error/rejection/missing-module fixtures for development.

`Engine::telemetry()` supplies optional global tempo (BPM) and CPU percentage independently of patch snapshots or module selection; its default is unknown for both. The simulator provides fixed display fixtures: 120 BPM and 12% CPU while Ready. CPU becomes unknown when disconnected, connecting or in error; the tempo fixture is retained across patch changes. This does not implement a musical clock, tempo propagation, MIDI sync or processor measurement. The footer displays unknown/non-finite/out-of-range CPU readings and non-positive tempo as `--`; load at 85% or above uses a warning color. Real adapters must supply fresh readings with a documented CPU measurement scope and source.

Real adapters must map these contracts to the verified protocol, provide their actual catalogue/port metadata, marshal I/O off the LVGL thread, and bound event queues. Timeouts, reconnect synchronization against a running host and coherent disk/plugin-state transactions belong to Phases 4–5.
