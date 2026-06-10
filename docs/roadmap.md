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

## v0.1 Blockers

- Confirm CI remains green on `main`.
- Confirm README quickstart works on a clean Ubuntu machine.
- Manually smoke-test graphical examples on Linux X11/XWayland.
- Review and close any issues found by final release checklist.
- Manually validate XKB detectable auto-repeat and fallback repeat suppression
  across representative X11 configurations.

Patch 017 resolved the hard spec-conformance blockers found after the initial
release-readiness review. Remaining work is validation-oriented unless new
issues are discovered.

## Deferred Until After v0.1

- High-DPI design.
- Multiple windows.
- Wayland investigation.
- Anti-aliased rasterization.
- Public clipping API.
- Richer text support.
- Allocator hooks.
- Sanitizer CI.
- Release packaging.
- Backend-independent software renderer split, if the X11 renderer grows too
  backend-coupled.

## Post-v0.1 WNG-Driven Roadmap

WNG feature requests are tracked in:

- `docs/wpl_feature_requests.md`

A WPL feature request becomes active only when a WNG milestone needs it and the
documented WNG fallback is becoming harmful.

### Implemented After v0.1

- WPL-FR-WNG-001 — Public ASCII text measurement
- WPL-FR-WNG-002 — Public text command byte limit

### v0.1.1 Candidate: Immediate WNG Support

- WPL-FR-WNG-011 — Atomic file write helper

### v0.1.2 Candidate: Graph Rendering Primitives

- WPL-FR-WNG-003 — Polyline or cubic Bezier primitive
- WPL-FR-WNG-004 — Dashed line primitive
- WPL-FR-WNG-006 — Clip rectangle stack

### v0.1.3 Candidate: Editor Usability Support

- WPL-FR-WNG-005 — Cursor shape API
- WPL-FR-WNG-008 — User-supplied debug overlay lines
- WPL-FR-WNG-009 — Panel and rounded rectangle primitives

### Deferred / Future

- WPL-FR-WNG-007 — Clipboard text API
- WPL-FR-WNG-010 — Expanded key and modifier support
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
