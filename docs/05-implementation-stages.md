# Implementation roadmap

**Updated:** 13 October 2026.  
**Current state:** Windows LVGL desktop editor accepted with a simulated engine; Phase 2 is complete on the PC. Phase 1 retains Linux platform and debugger setup work. Phase 0 hardware work is deferred while the Pi is packed.

The detailed task list, component boundaries, dependencies, deliverables and exit checks are in the [technical implementation plan](10-technical-implementation-plan.md). That plan is the authoritative phase sequence and supersedes this document's earlier Stage 0–9 outline.

## Development order

Start the shared LVGL application and 800 × 480 simulator on the Windows PC. Prove the minimal JACK/Ingen audio path on the Pi early, while the desktop editor is still small. Integrate the two before expanding the effect library. The simulator uses the same interface/domain code with a simulated engine; it does not establish Pi audio performance or touch usability.

| Phase | Work | Evidence required to advance |
| --- | --- | --- |
| 0 | Identify and preserve the current Pi system | Hardware/OS profile, device/service inventory, recoverable development setup and baseline measurements |
| 1 | Shared application and Windows LVGL simulator | Reproducible 800 × 480 application, domain/engine contracts and simulated backend; no Pi dependency |
| 2 | Desktop patch editor | Add/move/connect/edit/remove workflows, valid graph rules and visible pending/error states |
| 3 | JACK/Ingen audio foundation on the Pi | Working simple effect, verified protocol/host/routing semantics and client reconnection |
| 4 | Real engine and native Pi interface | Touch editing matches actual engine state; UI-only restart preserves audio |
| 5 | Patch, plugin-state and asset persistence | Coherent saved graph/state bundle plus companion JSON; restore and incomplete-save recovery |
| 6 | Clouds and representative patch | Tested granular controls and saved Clouds-plus-conventional-effect patch with resource evidence |
| 7 | Physical controls | One knob and one stomp assignment validated and restored; first complete musical milestone |
| 8 | NAM/cabinet evaluation | Pinned runtime comparison, file/state/calibration support and measured tested patch budgets |
| 9 | Catalogue and complete interface | Each selected entry classified; advertised supported entries pass compatibility and usability checks |
| 10 | Dedicated startup and recovery | Offline unattended boot, readiness, startup patch, graceful shutdown and measured boot results |
| 11 | Validation and reproducible release | Requirements matrix, complete measurements, build/install/recovery package and known limitations |

## Dependency rules

- Phases 1–2 can start without the Pi; Phase 3 requires the hardware/OS/audio facts from Phase 0.
- Phase 3 may overlap desktop UI work. Settle the engine protocol and required host features before building the full UI integration.
- Phase 4 needs the desktop editor and working engine. Phase 5 establishes persistence before the complete musical milestone.
- Phase 6 validates Clouds. Phase 7 supplies the knob/switch behaviour required by the first working milestone.
- NAM evaluation can start after Phases 4–5 and does not block the first Clouds milestone. Catalogue support remains conditional on individual results.
- Startup configuration follows a working representative patch; boot optimization follows measurements.

## Work boundaries

The new interface, build/configuration work and dependency records belong outside unchanged upstream snapshots. Casette's downloaded reference does not complete a build or select our operating system, devices, GPIO assignments or model runtime.

Ingen can serialize an audio graph as a `.ingen` directory bundle with Turtle and plugin state. The agreed companion JSON remains alongside the graph/bundle. Do not assume a flat graph file alone captures every plugin asset or that independent saves of two files are a coherent patch transaction.

Each phase leaves a reviewable artifact and measured or inspected evidence. Hardware acceptance occurs on the actual Pi, display, audio interface and controllers. A failed host assumption blocks the dependent task, not independent simulator work. Changes in source versions, hardware or platform require repeating affected checks, not automatically restarting the entire project.

Implementation started on the independent Windows UI work on 13 October 2026. See [the UI development record](11-windows-ui-foundation.md), [desktop review checklist](12-desktop-editor-review.md) and [domain/engine contracts](13-domain-and-engine-contracts.md) for evidence and current limits; hardware-dependent phases remain planned.
