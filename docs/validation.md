# WPL Validation

This document defines the validation evidence required before treating WPL changes
as ready to merge.  Recorded v0.1 release evidence lives in
`docs/validation_report_v0.1.md`.

## Automated Validation

Run the default local validation path:

```sh
./scripts/build.sh
./scripts/test.sh
ctest --test-dir build --output-on-failure
./scripts/check_public_headers.sh
./scripts/check_no_backend_leaks.sh
./scripts/check_no_frame_pacing_api.sh
```

Focused draw command validation:

```sh
ctest --test-dir build --output-on-failure -R 'wpl_test_draw_(list|list_edges|submit_lifetime)$'
```

Focused renderer validation:

```sh
ctest --test-dir build --output-on-failure -R 'wpl_test_renderer_(pixels|targets)$'
```

CI requirements:

- Ubuntu GCC job must pass.
- Ubuntu Clang job must pass.
- Ubuntu sanitizer job must pass.
- Ubuntu Xvfb smoke job must pass when the patch targets a branch that enables it.
- CI must run on pull requests targeting `main`.
- CI must run on pull requests targeting `development/linux-only-upgrade-plan`.
- CI must run on pushes to `main` and `development/linux-only-upgrade-plan`.
- CI builds graphical examples.
- Headless CI may run non-interactive X11 smoke tests under Xvfb.
- Interactive graphical examples still require manual validation on a real X11 or
  XWayland desktop session.

Optional local Clang validation when Clang is installed:

```sh
cmake -S . -B build-clang -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug
cmake --build build-clang --parallel
ctest --test-dir build-clang --output-on-failure
./scripts/check_public_headers.sh build-clang
./scripts/check_no_frame_pacing_api.sh
```

## Sanitizer Validation

Phase 0 adds an explicit sanitizer validation path for core and backend targets.
It enables AddressSanitizer and UndefinedBehaviorSanitizer for GCC or Clang.

Run:

```sh
./scripts/build_sanitize.sh
```

Equivalent CMake invocation:

```sh
cmake -S . -B build-sanitize \
  -DCMAKE_BUILD_TYPE=Debug \
  -DWPL_ENABLE_SANITIZERS=ON
cmake --build build-sanitize --parallel
ctest --test-dir build-sanitize --output-on-failure
```

Sanitizer failures must be treated as release-blocking unless the PR documents a
specific tool limitation and a follow-up plan.

## Xvfb Smoke Validation

Phase 0 adds a non-interactive X11 smoke path for CI and local headless Linux
environments.  It does not replace manual graphical validation.

Run:

```sh
./scripts/xvfb_smoke.sh
```

The script builds WPL in `build-xvfb` and runs the backend window API smoke test
under `xvfb-run`.  This validates create/pump/render/destroy coverage in a
headless X11 server without adding new platform scope.

## Lifecycle, Input, Draw, Renderer, Threading, and Timing Contracts

Lifecycle, input, text-input boundary, draw command, software-renderer,
threading, and timing assumptions are part of validation.  See:

- `docs/lifecycle_threading.md`
- `docs/input_snapshot_contract.md`
- `docs/text_input_boundary.md`
- `docs/draw_command_contract.md`
- `docs/software_renderer_contract.md`
- `docs/timing_frame_contract.md`

Relevant checks for reviewers:

- public headers still expose only portable C types and opaque handles,
- backend-native handles remain private,
- frame lifecycle order remains documented,
- transient input reset stays at `wpl_begin_frame`,
- input state remains per-window rather than hidden global state,
- text input is not inferred from key transitions or widened into editor policy,
- draw lists remain fixed-capacity and caller-owned,
- draw command storage remains private to WPL implementation files,
- draw helpers do not add widget, layout, scene graph, or editor semantics,
- multi-command draw helpers preserve all-or-nothing append behavior,
- draw submission preserves caller-owned list lifetime,
- draw submission does not expose backend renderer objects,
- renderer resources remain backend-owned and private,
- renderer changes do not introduce GPU abstraction or retained render state,
- renderer clipping, framebuffer, and presentation behavior remain documented,
- renderer pixel and target-boundary behavior stay covered by focused validation,
- frame delta stays at the `wpl_begin_frame` boundary,
- event pumping accumulates into the current frame snapshot,
- timing uses monotonic clocks,
- replay stays at the platform/input snapshot boundary,
- public headers do not expose frame-pacing API surface unless explicitly added,
- WPL APIs remain single-threaded unless explicitly documented otherwise,
- patches do not add hidden background work or unsynchronized shared state.

## Manual Graphical Validation

Graphical runtime validation requires a Linux X11 or XWayland desktop session.
Run:

```sh
./build/examples/00_empty_window/wpl_empty_window
./build/examples/01_input_snapshot/wpl_input_snapshot
./build/examples/02_draw_primitives/wpl_draw_primitives
./build/examples/03_canvas_pan_zoom/wpl_canvas_pan_zoom
./build/examples/04_debug_overlay/wpl_debug_overlay
./build/examples/05_input_replay/wpl_input_replay record /tmp/wpl_input.replay
./build/examples/05_input_replay/wpl_input_replay playback /tmp/wpl_input.replay
```

The helper script prints these commands:

```sh
./scripts/manual_graphical_smoke.sh
```

To run them sequentially:

```sh
./scripts/manual_graphical_smoke.sh --run
```

Required checks:

### 00_empty_window

- Window opens.
- Close button exits cleanly.
- Resize does not crash.

### 01_input_snapshot

- Key press/release prints correctly.
- Mouse press/release prints correctly.
- Wheel events print.
- Escape exits cleanly.

### 02_draw_primitives

- Clear background is visible.
- Rectangles render.
- Lines render.
- Circles render.
- Text renders.
- Resize/minimize is safe.

### 03_canvas_pan_zoom

- Grid/markers are visible.
- Mouse drag pans.
- Mouse wheel zooms around cursor.
- Screen/canvas coordinates update.
- Escape exits cleanly.

### 04_debug_overlay

- Overlay is visible.
- FPS/frame/window/mouse/draw stats are visible.
- Mouse state updates.
- Resize is safe.

### 05_input_replay

- Record mode creates a replay file.
- Playback mode loads the replay file.
- Playback visualizes recorded input.
- Replay reaches end cleanly.

## X11/XWayland Environment

Manual validation should record the environment used:

- X11 desktop or XWayland session.
- Distribution and version.
- Window manager or desktop environment.
- Compiler used for the local build.

## XKB Auto-Repeat Validation

Run:

```sh
./build/examples/01_input_snapshot/wpl_input_snapshot
```

Manual test:

1. Press and hold `A`.
2. Keep holding long enough for OS key repeat to begin.
3. Release `A`.

Expected behavior with XKB detectable auto-repeat enabled:

- One `key_pressed` transition for `A`.
- `key_down[A]` remains true while held.
- No repeated `key_released[A]` appears while held.
- One `key_released[A]` appears when physically released.

If the example prints only transitions, the expected behavior is:

- `A` press appears once.
- No `A` release appears until physical release.

## Fallback Repeat-Release Validation

The fallback repeat-release path is used when XKB detectable auto-repeat is
unavailable or disabled.  Default CI does not cover this path.

Current validation status:

- XKB detectable auto-repeat behavior requires manual validation on a real X
  server.
- Fallback repeat-release suppression requires validation in an environment where
  XKB detectable auto-repeat is unavailable or deliberately disabled.
- This remains a tracked release-gate validation item unless explicitly accepted
  as deferred risk.

Possible future closure:

- Add an internal-only test hook to force fallback repeat handling.
- Add a controlled Xvfb/Xnest scenario if it can reliably disable detectable
  auto-repeat without increasing platform scope.

## Release Evidence Checklist

Before final PASS:

- [ ] Ubuntu GCC CI passes.
- [ ] Ubuntu Clang CI passes.
- [ ] Ubuntu sanitizer CI passes.
- [ ] Ubuntu Xvfb smoke CI passes where enabled.
- [ ] Local default compiler build and tests pass.
- [ ] Optional local Clang build and tests pass, or limitation is documented.
- [ ] Sanitizer build and tests pass, or limitation is documented.
- [ ] Xvfb smoke validation passes, or limitation is documented.
- [ ] Manual graphical smoke validation is completed on X11/XWayland.
- [ ] XKB detectable auto-repeat behavior is manually validated.
- [ ] Fallback repeat-release path is validated or accepted as deferred risk.
- [ ] Public header/backend leak checks pass.
- [ ] Frame-pacing boundary check passes.
- [ ] Focused draw command validation passes.
- [ ] Focused renderer validation passes.
- [ ] Replay round-trip tests pass.
- [ ] README quickstart is verified on a clean Ubuntu-like machine.
