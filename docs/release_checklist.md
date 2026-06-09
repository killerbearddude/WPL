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
- [x] Public logging callback API is present.
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

Current gate status: **CONDITIONAL PASS**. Final PASS requires the validation evidence listed in `docs/validation.md`.

- [x] CMake configures the project.
- [x] `./scripts/build.sh` builds the library, tests, and examples.
- [x] GitHub Actions validates pull requests and pushes to `main` on Ubuntu.
- [x] CI installs only the required Linux/X11 build dependencies.
- [x] Ubuntu GCC CI job is defined.
- [x] Ubuntu Clang CI job is defined.
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
- [x] `wpl_test_log`

## Examples

Expected graphical examples:

- [x] `examples/00_empty_window`
- [x] `examples/01_input_snapshot`
- [x] `examples/02_draw_primitives`
- [x] `examples/03_canvas_pan_zoom`
- [x] `examples/04_debug_overlay`
- [x] `examples/05_input_replay`

Graphical examples require a Linux X11/XWayland desktop for manual runtime validation. CI builds them but does not run them. Required manual checks are listed in `docs/validation.md`.

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

## Resolved in Patch 017

- [x] Added missing public logging API.
- [x] Added required `examples/03_canvas_pan_zoom`.
- [x] Tightened replay v1 header-size validation to exact equality.
- [x] Added X11 TrueColor visual-mask validation before direct framebuffer copy.
- [x] Hardened renderer float-to-int conversion paths against undefined behavior.
- [x] Expanded input transition unit tests.
- [x] Corrected release-readiness language that overstated status.

## v0.1 Blockers

Required before final PASS:

- [ ] Ubuntu GCC CI passes on latest `main`.
- [ ] Ubuntu Clang CI passes on latest `main`.
- [ ] Manual graphical smoke test completed on X11/XWayland.
- [ ] XKB detectable auto-repeat behavior manually validated.
- [ ] Fallback repeat-release path validated or explicitly accepted as deferred risk.
- [ ] Confirm all CI checks pass on `main`.
- [ ] Confirm README quickstart works on a clean Ubuntu machine.
- [ ] Confirm manual graphical examples run on a Linux X11/XWayland desktop.
- [ ] Confirm public headers have no backend leakage.
- [ ] Confirm replay round-trip tests pass.

Known remaining validation gaps:

- XKB detectable auto-repeat behavior should be manually validated on a real X server.
- Fallback repeat-release suppression should be tested in an environment where detectable auto-repeat is unavailable or explicitly accepted as deferred risk.
- Graphical examples require manual X11/XWayland validation.

No hard code blockers are currently known after the Patch 017 rework, but the release gate remains conditional until the validation evidence above is completed.

## Tagging Checklist

Before tagging `v0.1`:

- [ ] Start from clean `main`.
- [ ] Run `./scripts/build.sh`.
- [ ] Run `./scripts/test.sh`.
- [ ] Run `ctest --test-dir build --output-on-failure`.
- [ ] Run `./scripts/check_public_headers.sh`.
- [ ] Run `./scripts/check_no_backend_leaks.sh`.
- [ ] Confirm Ubuntu GCC CI is passing on `main`.
- [ ] Confirm Ubuntu Clang CI is passing on `main`.
- [ ] Manually smoke-test graphical examples on Linux X11/XWayland.
- [ ] Review `README.md` for current quickstart accuracy.
- [ ] Review this checklist for unresolved blockers.
