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
