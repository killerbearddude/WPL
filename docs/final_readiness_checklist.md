# WPL Final Readiness Checklist

This checklist records the current final-readiness review gate for the Linux-native
C11/X11 platform-layer scope after Phases 0 through 10a.

Historical v0.1 evidence remains in `docs/validation_report_v0.1.md`.

## Scope Lock

- [x] Linux only.
- [x] C11 only.
- [x] X11 backend only.
- [x] Software rendering only.
- [x] MIT licensed.
- [x] No SDL, Wayland, Windows, or macOS backend.
- [x] No GPU abstraction.
- [x] No widgets or layout engine.
- [x] No node graph logic.
- [x] No application/editor behavior.
- [x] No asset pipeline.

## API and Backend Boundary

- [x] Public headers live under `include/wpl/`.
- [x] Public API is C-compatible.
- [x] Public headers do not expose X11 types.
- [x] Public headers do not expose renderer internals.
- [x] Public headers do not expose replay binary structs.
- [x] API audit identifies no hard API conformance blocker.
- [x] Frame-pacing API remains out of the public surface.

## Validation Gate

Run before treating a branch as ready:

```sh
./scripts/build.sh
./scripts/test.sh
ctest --test-dir build --output-on-failure
./scripts/check_public_headers.sh build
./scripts/check_no_backend_leaks.sh
./scripts/check_no_frame_pacing_api.sh
```

Focused subsystem validation is documented in `docs/validation.md`.

## Expected Test Surface

The current development branch registers 23 CTest targets when a C++ compiler is available.

## Known Limitations

- Linux/X11 only.
- One active window.
- Software rendering only.
- No high-DPI policy.
- No anti-aliasing.
- ASCII bitmap text only.
- No Unicode text shaping.
- No allocator hooks.
- No public framebuffer readback API.
- No transform stack API.
- No font selection or font loading API.
- No public sleep/yield/target-FPS/event-wait APIs.
- Replay v1 only.
- Replay does not expose raw events, event synthesis, timeline controls, or application/editor state capture.
- Graphical examples require X11/XWayland.

## Final Gate

- [ ] Confirm all intended feature and documentation PRs are merged.
- [ ] Confirm no local uncommitted changes.
- [ ] Run the local validation gate.
- [ ] Run sanitizer validation or document any tool limitation.
- [ ] Run Xvfb smoke validation or document any environment limitation.
- [ ] Confirm CI passes.
- [ ] Manually smoke-test graphical examples on Linux X11/XWayland.
- [ ] Review `README.md` for current quickstart accuracy.
- [ ] Review `docs/api_review.md` for unresolved API blockers.
- [ ] Review this checklist for unresolved blockers.
