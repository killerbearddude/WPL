# WPL Linux-Only Upgrade Implementation Plan

This document resumes the original WPL Linux-Only Upgrade Plan after the Phase 0
through Phase 10 hardening and readiness pass.

The previous pass completed repository hardening, validation coverage, subsystem
contracts, replay/file/debug/canvas/renderer edge coverage, public API audit, and
final readiness documentation. It did not fully implement every feature goal from
the original Linux-only upgrade plan.

This document defines the remaining implementation work required to complete the
original plan while preserving WPL's platform-layer boundaries.

## Objective

Complete the original Linux-only upgrade intent:

1. Add a private Linux display-backend abstraction.
2. Preserve and refactor the current X11 backend behind that abstraction.
3. Support multiple native windows.
4. Improve the software-renderer presentation path.
5. Add DPI/content-scale, text-input, clipboard, and richer input primitives.
6. Add a native Wayland backend.
7. Expand validation across the new backend and input/rendering surfaces.

## Scope Rules

The implementation remains Linux-native and C11-only.

In scope:

- Private backend abstraction.
- X11 backend refactor.
- X11 multi-window support.
- Wayland backend support.
- Software rendering only.
- X11 XImage fallback presentation.
- Optional X11 XShm presentation.
- Dirty rectangle tracking and validation.
- Content-scale and framebuffer-size APIs.
- UTF-8 text-input snapshots.
- UTF-8 clipboard get/set API.
- Richer mouse/window input state.
- Xvfb, sanitizer, and Wayland-headless validation paths.

Out of scope:

- GUI widgets.
- Layout engine.
- Node graph logic.
- Application/editor behavior.
- GPU abstraction.
- SDL backend.
- Windows or macOS support.
- ECS, scene graph, or asset pipeline.
- Graph serialization or project-file policy.

## Current Baseline

The current branch is a hardened X11-only platform layer.

Completed baseline work includes:

- Lifecycle/threading documentation.
- Input snapshot contract and tests.
- Timing/frame contract and checks.
- Draw command contract and edge tests.
- Software renderer contract and pixel/target tests.
- Canvas math contract and edge tests.
- Debug overlay contract and edge tests.
- File I/O contract and edge tests.
- Replay contract and edge tests.
- Public API audit.
- Final readiness checklist.
- README documentation index and roadmap sync.

Known remaining gaps from the original upgrade intent:

- The public build still selects only the X11 backend.
- The X11 implementation still owns backend behavior directly.
- Multiple windows are not the active public behavior target.
- Wayland backend is not implemented.
- Dirty rectangle presentation is not implemented.
- XShm presentation is not implemented.
- DPI/content scale is not exposed.
- Text input is not separated from key state as UTF-8 text snapshots.
- Clipboard API is not implemented.
- Richer mouse state remains limited.

## Implementation Phases

### Phase 11: Original-plan scope reset

Document the remaining original-plan implementation work and prevent the prior
hardening pass from being mistaken for full feature completion.

Deliverables:

- This implementation plan.
- README or roadmap link updates in a follow-up patch if needed.

Acceptance:

- Documentation-only.
- No public API, source, tests, examples, or build behavior changes.

### Phase 12: Private backend abstraction

Introduce the internal backend interface and dispatcher while preserving current
X11 behavior.

Deliverables:

- Private backend vtable and backend-owned window hooks.
- Public API dispatcher in backend-independent source.
- X11 backend wired through the dispatcher.
- X11 remains the only enabled backend.

Acceptance:

- Existing public API remains source-compatible.
- Existing X11 examples and tests pass.
- Public headers expose no backend types.
- Backend-leak checks pass.

### Phase 13: X11 backend ownership cleanup

Move display ownership, atoms, and backend-global state into an explicit X11
backend object.

Deliverables:

- X11 backend object.
- Shared Display ownership inside the backend.
- Window registry scaffolding.
- No user-visible multi-window behavior yet unless fully tested.

Acceptance:

- One-window behavior remains compatible.
- Creation and destruction cleanup paths are deterministic.
- Sanitizer validation remains clean.

### Phase 14: X11 multi-window support

Remove the one-active-window restriction and route events by native X11 window.

Deliverables:

- Multiple `WplWindow` instances per X11 backend.
- X11 Window-to-WplWindow registry.
- Routed event dispatch.
- Per-window input, size, close, cursor, timing, framebuffer, and renderer state.
- Multi-window Xvfb smoke tests.

Acceptance:

- Two X11 windows can be created, pumped, rendered, resized, and destroyed.
- Close/input/resize events affect only the owning window.
- Destroying one window does not invalidate another.
- Public headers remain backend-clean.

### Phase 15: Renderer validation and dirty rectangles

Add backend-independent renderer validation and dirty region tracking before any
presentation optimization.

Deliverables:

- Draw-list validation pass.
- Dirty rectangle accumulator.
- Clip-aware dirty bounds.
- Dirty overflow collapse-to-full-frame behavior.
- Focused renderer validation tests.

Acceptance:

- Existing renderer pixel behavior remains stable.
- Invalid draw data is rejected or ignored according to documented policy.
- Dirty tracking is deterministic and test-covered.
- Full-frame fallback remains available.

### Phase 16: X11 XShm presentation path

Add optional MIT-SHM presentation for X11 with a safe XImage fallback.

Deliverables:

- CMake detection for XShm/Xext.
- Runtime XShm detection.
- X11 error-trap wrapper for optional XShm setup.
- Shared-memory image allocation and cleanup.
- Fallback to XImage when unsupported.

Acceptance:

- XImage path remains available.
- XShm path passes Xvfb/local smoke where supported.
- Failed XShm setup does not fail window creation if XImage works.
- No leaked shared-memory segments under failure.

### Phase 17: DPI and coordinate scale

Expose content scale and distinguish logical window size from framebuffer size.

Deliverables:

- Public content-scale query API.
- Public framebuffer-size query API.
- Logical-to-framebuffer and framebuffer-to-logical helpers.
- Documentation for coordinate ownership.
- Canvas scale interop tests.

Acceptance:

- Existing examples behave unchanged at scale 1.0.
- Scale APIs return deterministic fallback values on X11.
- Canvas math remains backend-independent.

### Phase 18: Text-input snapshots

Add bounded UTF-8 text input as frame-stable input snapshot data, separate from
physical key state.

Deliverables:

- `WplTextInputState` in `WplInputState`.
- Per-frame UTF-8 byte buffer.
- Truncation flag.
- X11 text lookup path.
- Tests for reset, append, truncation, and key/text separation.

Acceptance:

- Text input is snapshot-based.
- Key state remains independent.
- No text editing, selection, widget, or IME UI policy is added.

### Phase 19: Clipboard API

Add backend-independent UTF-8 clipboard get/set support.

Deliverables:

- Public clipboard header.
- Explicit owned clipboard text result type.
- X11 CLIPBOARD/UTF8_STRING implementation.
- Tests for empty text, UTF-8 bytes, invalid arguments, ownership reset, and
  same-process smoke.

Acceptance:

- API ownership is explicit.
- Clipboard support does not add text editor behavior.
- Unsupported backend behavior is deterministic.

### Phase 20: Richer mouse/window input state

Extend input snapshots with backend-supported pointer/window state.

Deliverables:

- Inside-window state.
- Horizontal wheel path where supported.
- Cursor visibility/inside state documentation.
- X11 enter/leave tests.

Acceptance:

- Existing input behavior remains compatible.
- Added fields are frame-stable.
- Backend limitations are documented.

### Phase 21: Wayland backend bootstrap

Add the optional Wayland backend and keep X11 as the default backend until the
Wayland path is mature.

Deliverables:

- Wayland build option.
- xdg-shell protocol support.
- Wayland backend skeleton.
- Window create/configure/destroy.
- wl_shm software presentation.
- Pointer input.
- Keyboard input through xkbcommon.
- Content scale/output handling.
- Basic close handling.

Acceptance:

- X11 remains the default and continues passing validation.
- Wayland backend builds when dependencies are enabled.
- Wayland smoke can create, render, pump, and destroy a window.
- Public headers expose no Wayland types.

### Phase 22: Wayland clipboard and text parity

Bring Wayland text/clipboard support to the same platform-layer contract level as
X11 where practical.

Deliverables:

- Wayland data-device clipboard path.
- Wayland keyboard-to-text path or documented limitation.
- Headless/manual Wayland validation notes.

Acceptance:

- Clipboard API behavior is deterministic on Wayland.
- Any unsupported text-input feature is documented precisely.

### Phase 23: Expanded validation and CI

Expand validation for the new feature surface.

Deliverables:

- Multi-window Xvfb smoke.
- XShm fallback smoke.
- Optional Wayland headless smoke script.
- Sanitizer validation across new tests.
- Renderer/text/clipboard malformed-input tests.
- Backend-leak checks updated for Wayland-private types.

Acceptance:

- Full default test suite passes.
- Public header C/C++ smoke passes.
- X11 backend leak check passes.
- Wayland backend leak check passes when enabled.
- Sanitizer path is clean.

### Phase 24: Final original-plan API audit and readiness closeout

Close the original Linux-only upgrade plan after feature implementation.

Deliverables:

- Updated `docs/api_review.md`.
- Updated `docs/final_readiness_checklist.md`.
- Updated README and roadmap.
- Final original-plan completion report.

Acceptance:

- All original upgrade goals are either implemented or explicitly deferred with
  rationale.
- No hard public API blocker remains.
- Final readiness validation is recorded.

## Definition of Done

The original WPL Linux-Only Upgrade Plan is complete when:

- X11 is behind a private backend interface.
- Wayland exists as an optional Linux backend.
- Multiple X11 windows are supported and tested.
- Software rendering has validation and dirty-region support.
- X11 XShm is available where supported with XImage fallback.
- DPI/content-scale APIs are exposed and documented.
- Text input is separate from physical key state.
- Clipboard UTF-8 API exists.
- Richer pointer/window input state is exposed where supported.
- Public headers remain backend-clean.
- X11 and Wayland backend details remain private.
- Validation covers public headers, backend leaks, frame-pacing boundary,
  sanitizer runs, Xvfb smoke, and Wayland smoke.
- The project still contains no widgets, layout engine, node graph logic,
  application/editor behavior, GPU abstraction, or cross-platform backend layer.

## Immediate Next Work

After this document lands, proceed with backend abstraction in small patches:

1. Add the private backend interface skeleton.
2. Add the public API dispatcher.
3. Wrap the current X11 implementation with no behavior change.
4. Add backend-interface tests.
5. Only then begin X11 multi-window work.
