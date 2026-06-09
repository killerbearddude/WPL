# WPL v0.1 Release Checklist

This checklist is for deciding whether the current WPL tree is ready for a
future `v0.1` tag. It is intentionally limited to the Linux-native C11/X11
platform-layer scope.

## Scope Lock

- [x] Linux only.
- [x] C11 only.
- [x] X11 backend only.
- [x] Software rendering only.
- [x] MIT license.
- [x] No SDL backend.
- [x] No Wayland backend.
- [x] No Windows or macOS backend.
- [x] No GPU abstraction.
- [x] No widgets.
- [x] No layout engine.
- [x] No node graph logic.
- [x] No application/editor behavior.

## Public API

- [x] Public headers live under `include/wpl/`.
- [x] Public API is C-compatible.
- [x] Umbrella include is `#include <wpl/wpl.h>`.
- [x] Public ownership rules are explicit for draw lists, replay handles, file
      data, and windows.
- [x] Public opaque handles are used for backend or stateful objects.
- [x] Public headers do not expose X11 types.
- [x] `WplInputState` exposes frame-stable input snapshots.
- [x] Draw lists are fixed-capacity and append rendering intent only.
- [x] Replay does not serialize raw C structs.

## Backend Isolation

- [x] X11 includes are isolated to `backends/linux_x11/`.
- [x] Core source files do not depend on X11.
- [x] Tests for core modules link against `wpl_core` where practical.
- [x] Examples consume public WPL APIs rather than backend-private symbols.
- [x] `scripts/check_no_backend_leaks.sh` validates the public/backend boundary.

## Build and CI

- [x] CMake configures the project.
- [x] `./scripts/build.sh` builds the library, tests, and examples.
- [x] GitHub Actions validates pull requests and pushes to `main` on Ubuntu.
- [x] CI installs only the required Linux/X11 build dependencies.
- [x] CI builds graphical examples but does not run them.
- [x] CI runs CTest.
- [x] CI runs public-header and backend-leak checks.

## Tests

Expected CTest registrations:

- [x] `wpl_test_input_state`
- [x] `wpl_test_draw_list`
- [x] `wpl_test_canvas`
- [x] `wpl_test_debug_overlay`
- [x] `wpl_test_file_io`
- [x] `wpl_test_replay_format`
- [x] `wpl_test_replay`
- [x] `wpl_test_public_headers`
- [x] `wpl_test_public_headers_cpp` when a C++ compiler is available

## Examples

Expected graphical examples:

- [x] `examples/00_empty_window`
- [x] `examples/01_input_snapshot`
- [x] `examples/02_draw_primitives`
- [x] `examples/04_debug_overlay`
- [x] `examples/05_input_replay`

Graphical examples require a Linux X11/XWayland desktop for manual runtime
validation. CI builds them but does not run them.

## Documentation

- [x] README describes project scope, non-goals, build/test flow, examples, CI,
      implemented modules, backend isolation, and license.
- [x] API review document exists.
- [x] Examples document exists.
- [x] Roadmap/status document exists.
- [x] Replay format document explains fixed binary encoding and no raw struct
      serialization.
- [x] File I/O document explains whole-file binary I/O and v0.1 limits.

## Known Limitations

- Linux/X11 only.
- One active window.
- Software rendering only.
- No high-DPI support.
- No clipping API.
- No anti-aliasing.
- ASCII bitmap text only.
- No Unicode text shaping.
- No atomic file-write protocol.
- Replay v1 only.
- Graphical examples require X11/XWayland.
- CI builds graphical examples but does not run them.

## v0.1 Blockers

- [ ] Confirm all CI checks pass on `main`.
- [ ] Confirm README quickstart works on a clean Ubuntu machine.
- [ ] Confirm manual graphical examples run on a Linux X11/XWayland desktop.
- [ ] Confirm public headers have no backend leakage.
- [ ] Confirm replay round-trip tests pass.

No known code blockers were identified in this review.

## Tagging Checklist

Before tagging `v0.1`:

- [ ] Start from clean `main`.
- [ ] Run `./scripts/build.sh`.
- [ ] Run `./scripts/test.sh`.
- [ ] Run `ctest --test-dir build --output-on-failure`.
- [ ] Run `./scripts/check_public_headers.sh`.
- [ ] Run `./scripts/check_no_backend_leaks.sh`.
- [ ] Confirm CI is passing on `main`.
- [ ] Manually smoke-test graphical examples on Linux X11/XWayland.
- [ ] Review `README.md` for current quickstart accuracy.
- [ ] Review this checklist for unresolved blockers.
