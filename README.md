# Whacky Platform Layer

Whacky Platform Layer (WPL) is a small MIT-licensed, Linux-native C11 platform
layer for canvas-heavy custom UI foundations and future node graph tools.

WPL owns platform interaction and low-level service infrastructure. Higher-level
application code owns UI behavior, editor behavior, and node graph semantics.

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

WPL is not a game engine, GUI toolkit, widget library, layout engine, node graph
editor, SDL wrapper, Wayland abstraction, Windows/macOS layer, GPU abstraction,
ECS, scene graph, asset pipeline, or application-specific editor framework.

## Build requirements

- Linux
- CMake 3.16 or newer
- C11 compiler such as GCC or Clang
- X11 development headers and libraries

A C++ compiler is optional and is used only for public-header include smoke
coverage when available.

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

## Examples

After building, examples are available under `build/examples/`:

- `00_empty_window/wpl_empty_window` opens and closes a basic X11 window.
- `01_input_snapshot/wpl_input_snapshot` prints keyboard, mouse, wheel, and
  modifier transitions from `WplInputState`.
- `02_draw_primitives/wpl_draw_primitives` renders primitives and ASCII bitmap
  text through the software renderer.
- `04_debug_overlay/wpl_debug_overlay` shows the append-only debug overlay using
  draw commands.

## Public API include style

Applications should include the umbrella header unless they intentionally need a
narrow module header:

```c
#include <wpl/wpl.h>
```

Public headers are C-compatible and backend-clean. They do not expose X11 types,
file descriptors, XImage ownership, renderer internals, or replay binary structs.

## Implemented modules

- X11 window lifecycle and event pump
- Frame-stable input snapshots
- Monotonic timing and frame delta tracking
- Fixed-capacity draw command buffer
- Software renderer with clear, rectangles, outlines, lines, circles, and ASCII
  bitmap text
- Canvas math for pan/zoom and coordinate conversion
- Append-only debug overlay
- Binary whole-file I/O
- Replay v1 binary format
- Replay recorder/player for input snapshots

## Backend isolation rule

Only `backends/linux_x11/` may include X11 headers or use X11 types. Core source
files, public headers, tests, and examples must consume backend-independent WPL
APIs.

## License

WPL is released under the MIT License. See `LICENSE`.
