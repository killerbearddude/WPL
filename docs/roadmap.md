# Roadmap

## Completed for v0.1 Candidate

- Repository skeleton, MIT license, CMake build, and scripts.
- X11 window lifecycle and event pump.
- Frame-stable input snapshots.
- Monotonic timing and frame delta tracking.
- Fixed-capacity draw command buffer.
- Software renderer foundation.
- Basic primitives: rectangles, outlines, lines, and circles.
- ASCII bitmap text rendering.
- Canvas math for pan/zoom and coordinate conversion.
- Canvas pan/zoom example.
- Append-only debug overlay.
- Public logging callback API.
- Binary whole-file I/O.
- Replay v1 binary format foundation.
- Replay recorder/player.
- Input replay example.
- Public header smoke tests and backend-leak checks.
- Ubuntu GitHub Actions CI.
- v0.1 release-readiness and API review documentation.
- Patch 017 spec-conformance rework: logging API, canvas example, replay
  header validation, X11 visual-mask validation, renderer cast safety, and
  input transition test expansion.

## Historical v0.1 Release Gate

- Confirm CI remains green on `main`.
- Confirm README quickstart works on a clean Ubuntu machine.
- Manually smoke-test graphical examples on Linux X11/XWayland.
- Review and close any issues found by final release checklist.
- Manually validate XKB detectable auto-repeat and fallback repeat suppression
  across representative X11 configurations.

These items were resolved or explicitly accepted before the `v0.1.0` tag. They
remain here as release history, not as active blockers for current `main`.

Patch 017 resolved the hard spec-conformance blockers found after the initial
release-readiness review.

## Completed After v0.1

- Public ASCII text measurement.
- Public text command byte limit.
- Atomic whole-file write helper.
- Polyline draw helper.
- Dashed line draw helper.
- Clip rectangle stack.
- Cursor shape API.
- User-supplied debug overlay lines.
- Panel and rounded rectangle primitives.
- Software renderer contract documentation and renderer pixel/target tests.
- Canvas math contract documentation and rectangle/transform edge tests.
- Debug overlay contract documentation and edge tests.
- File I/O contract documentation and edge tests.
- Replay contract documentation and edge tests.
- Phase 10 public API audit.
- Final readiness checklist for current branch validation.

## Current Validation and Readiness References

- `docs/validation.md` defines required validation commands and reviewer gates.
- `docs/api_review.md` records the current public API audit result.
- `docs/final_readiness_checklist.md` records the current branch readiness gate.
- Historical `v0.1.0` validation evidence remains in
  `docs/validation_report_v0.1.md`.

## Deferred Until After v0.1

- High-DPI design.
- Multiple windows.
- Wayland investigation.
- Anti-aliased rasterization.
- Richer text support.
- Allocator hooks.
- Release packaging.
- Backend-independent software renderer split, if the X11 renderer grows too
  backend-coupled.
- Targeted fallback repeat-release validation in an environment that can force
  non-XKB-detectable repeat behavior.

## Post-v0.1 WNG-Driven Roadmap

WNG feature requests are tracked in:

- `docs/wpl_feature_requests.md`

A WPL feature request becomes active only when a WNG milestone needs it and the
documented WNG fallback is becoming harmful.

### Implemented After v0.1

- WPL-FR-WNG-001 — Public ASCII text measurement
- WPL-FR-WNG-002 — Public text command byte limit
- WPL-FR-WNG-011 — Atomic file write helper
- WPL-FR-WNG-003 — Polyline draw helper
- WPL-FR-WNG-004 — Dashed line draw helper
- WPL-FR-WNG-006 — Clip rectangle stack
- WPL-FR-WNG-005 — Cursor shape API
- WPL-FR-WNG-008 — User-supplied debug overlay lines
- WPL-FR-WNG-009 — Panel and rounded rectangle primitives

### Active Candidates

No additional WNG-driven request is promoted automatically after Patch 029.
Future work should come from concrete WNG integration pressure.

### Deferred / Future

- WPL-FR-WNG-010 — Expanded key and modifier support
- WPL-FR-WNG-007 — Clipboard text API
- WPL-FR-WNG-012 — Native file dialog API
- WPL-FR-WNG-013 — High-DPI policy
- WPL-FR-WNG-014 — Anti-aliased primitive rendering
- WPL-FR-WNG-015 — Extended text support

## Explicit Non-Goals

- GUI widgets.
- Layout engine.
- Node graph logic.
- GPU abstraction.
- SDL backend.
- Cross-platform backend architecture.
- Application-specific editor behavior.
- ECS or scene graph.
- Asset pipeline.

Cubic Bezier link primitive remains deferred until WNG proves straight/polyline
links are insufficient.
