# Whacky Platform Layer

Whacky Platform Layer (WPL) is a small MIT-licensed, Linux-native C11 platform
layer for canvas-heavy custom UI foundations and future node graph tools.

WPL owns platform interaction and low-level service infrastructure. Higher-level
application code owns UI behavior, editor behavior, and node graph semantics.

## Current status

WPL is a v0.1 release candidate with zero-trust validation status: **PASS**.

Validation evidence is recorded in:

- `docs/validation_report_v0.1.md`

Release notes are prepared in:

- `docs/release_notes_v0.1.md`

The repository currently includes:

- X11 window lifecycle and event pump
- Frame-stable input snapshots
- Monotonic timing and frame delta tracking
- Fixed-capacity draw command buffer
- Software renderer
- Basic 2D primitives
- ASCII bitmap text
- Canvas math
- Canvas pan/zoom example
- Debug overlay
- Public logging callback
- File I/O
- Replay v1 binary format
- Replay recorder/player
- Input replay example
- Ubuntu GCC/Clang CI workflow

See `docs/release_checklist.md`, `docs/validation_report_v0.1.md`, `docs/release_notes_v0.1.md`, `docs/release_process.md`, `docs/validation.md`, and `docs/api_review.md` for the v0.1 readiness review, validation evidence, and release preparation notes.

## Scope

- Linux only.
- C11 public API.
- X11/Xlib backend.
- Software rendering.
- Public headers expose portable C types and opaque handles only.
- Backend details stay private to implementation files.
- Input is exposed as frame-stable snapshots.
- Replay operates at the platform/input boundary.

## Non-goals

WPL deliberately does not provide:

- SDL support.
- Wayland support.
- Windows or macOS support.
- GPU abstraction.
- OpenGL, Vulkan, EGL, GLX, Metal, or DirectX abstraction.
- GUI widgets.
- Layout system.
- Node graph logic.
- Scene graph.
- ECS.
- Asset pipeline.
- Application-specific editor logic.

## Requirements

- Linux.
- CMake 3.16 or newer.
- C11 compiler such as GCC or Clang.
- X11 development headers and libraries.

A C++ compiler is optional and is used only for public-header include smoke
coverage when available.

On Ubuntu-like systems:

```sh
sudo apt-get install build-essential cmake libx11-dev
```

## Build

```sh
./scripts/build.sh
```

## Test

```sh
./scripts/test.sh
```

Focused checks:

```sh
ctest --test-dir build --output-on-failure
./scripts/check_public_headers.sh
./scripts/check_no_backend_leaks.sh
```

## Continuous Integration

Pull requests and pushes to `main` are validated on Ubuntu with CMake, GCC, Clang, the X11 development headers, public-header smoke tests, backend-leak checks, and CTest.

CI builds the library, tests, and examples. It does not run graphical examples because they require a display server. Manual graphical validation is documented in `docs/validation.md`.

## Examples

After building, examples are available under `build/examples/`.

- `00_empty_window/wpl_empty_window` opens and closes a basic X11 window.
- `01_input_snapshot/wpl_input_snapshot` prints keyboard, mouse, wheel, and
  modifier transitions from `WplInputState`.
- `02_draw_primitives/wpl_draw_primitives` renders primitives and ASCII bitmap
  text through the software renderer.
- `03_canvas_pan_zoom/wpl_canvas_pan_zoom` demonstrates canvas coordinate
  conversion, mouse-drag panning, and cursor-anchored wheel zoom.
- `04_debug_overlay/wpl_debug_overlay` shows the append-only debug overlay using
  draw commands.
- `05_input_replay/wpl_input_replay` records and plays back `WplInputState`
  snapshots through the replay API.

Record an input replay:

```sh
./build/examples/05_input_replay/wpl_input_replay record /tmp/wpl_input.replay
```

Play it back:

```sh
./build/examples/05_input_replay/wpl_input_replay playback /tmp/wpl_input.replay
```

The input replay example records input snapshots only. It does not replay raw
X11 events, editor commands, application state, widgets, or node graph behavior.

More example notes are in `docs/examples.md`. Manual X11/XWayland smoke validation is documented in `docs/validation.md`.

## Public API include style

Applications should include the umbrella header unless they intentionally need a
narrow module header:

```c
#include <wpl/wpl.h>
```

Public headers are C-compatible and backend-clean. They do not expose X11 types,
file descriptors, XImage ownership, renderer internals, or replay binary structs.

## Backend isolation rule

Only `backends/linux_x11/` may include X11 headers or use X11 types. Core source
files, public headers, tests, and examples must consume backend-independent WPL
APIs.

## Known limitations

- Linux/X11 only.
- One active window.
- Software rendering only.
- No high-DPI support.
- No clipping API.
- No anti-aliasing.
- ASCII bitmap text only.
- No Unicode text shaping.
- The `v0.1.0` tag has no atomic file-write protocol; post-v0.1 `main` now
  includes a low-level atomic whole-file write helper.
- Replay v1 only.
- X11 renderer supports common TrueColor visuals with RGB masks
  `0x00ff0000`, `0x0000ff00`, and `0x000000ff`.
- Graphical examples require X11/XWayland.
- CI builds graphical examples but does not run them.

## Post-v0.1 Roadmap

WPL v0.1.0 is released and validated.

Post-v0.1 WPL work is now guided by WNG-driven feature requests recorded in:

- `docs/wpl_feature_requests.md`

These requests are platform-layer enhancements only. WPL will not absorb WNG
graph data, graph mutation, graph validation, graph serialization DTOs, graph
hit testing, selection rules, editor command logic, widgets, or layout systems.

Post-v0.1 `main` additionally includes public ASCII text measurement and a
public text command byte limit for WNG planning. These are not part of the
`v0.1.0` tag until a later release is cut.

Post-v0.1 `main` also includes atomic whole-file write support for safer host
persistence workflows. This remains low-level file I/O infrastructure and does
not add graph serialization or file format policy.

Post-v0.1 `main` additionally includes a public polyline draw helper for WNG
graph-link rendering experiments. The helper expands to existing line commands
and is not part of the `v0.1.0` tag.

Post-v0.1 `main` also includes dashed-line drawing through a helper that
expands to existing line commands. This remains a low-level drawing primitive
and does not add graph/editor semantics.

Post-v0.1 `main` additionally includes draw-list clip rectangle push/pop
support for constrained rendering. This is renderer behavior only and does not
add widget, panel, scroll, minimap, graph, or editor policy.

Post-v0.1 `main` additionally includes a small cursor shape API for platform
cursor feedback. X11 cursor resources remain backend-private and are not part of
the `v0.1.0` tag.

Post-v0.1 `main` additionally includes custom debug overlay lines through
`wpl_debug_draw_overlay_ex`. WPL appends caller-provided diagnostics as draw
commands and does not interpret graph/editor state.

## License

WPL is released under the MIT License. See `LICENSE`.
