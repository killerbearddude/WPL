# WPL v0.1 Validation

This document defines the validation evidence required before treating the WPL
v0.1 candidate as a final release candidate. Recorded v0.1 evidence lives in `docs/validation_report_v0.1.md`.

## Automated Validation

Run the default local validation path:

```sh
./scripts/build.sh
./scripts/test.sh
ctest --test-dir build --output-on-failure
./scripts/check_public_headers.sh
./scripts/check_no_backend_leaks.sh
```

CI requirements:

- Ubuntu GCC job must pass.
- Ubuntu Clang job must pass.
- CI must run on pull requests targeting `main`.
- CI must run on pushes to `main`.
- CI builds graphical examples but does not execute them.

Optional local Clang validation when Clang is installed:

```sh
cmake -S . -B build-clang -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug
cmake --build build-clang --parallel
ctest --test-dir build-clang --output-on-failure
./scripts/check_public_headers.sh build-clang
```

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

Graphical validation is intentionally not run in CI because CI does not provide a
stable interactive display server for these examples.

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
unavailable or disabled. Default CI does not cover this path.

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

Before final v0.1 PASS:

- [ ] Ubuntu GCC CI passes on latest `main`.
- [ ] Ubuntu Clang CI passes on latest `main`.
- [ ] Local default compiler build and tests pass.
- [ ] Optional local Clang build and tests pass, or limitation is documented.
- [ ] Manual graphical smoke validation is completed on X11/XWayland.
- [ ] XKB detectable auto-repeat behavior is manually validated.
- [ ] Fallback repeat-release path is validated or accepted as deferred risk.
- [ ] Public header/backend leak checks pass.
- [ ] Replay round-trip tests pass.
- [ ] README quickstart is verified on a clean Ubuntu-like machine.
