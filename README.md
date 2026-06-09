# Whacky Platform Layer

Whacky Platform Layer (WPL) is a small Linux-native C11 platform layer for
canvas-heavy custom UI foundations and future node graph tooling.

Whacky Platform Layer is released under the MIT License.

## Locked v0.1 scope

- Linux only.
- C11 only.
- X11/Xlib backend only.
- Software rendering only.
- Clean C-compatible public API.
- Public headers expose no X11 or backend-specific types.
- Input is exposed as frame-stable snapshots.
- Draw lists use fixed-capacity command buffers.

## Non-goals

WPL is not a game engine, GUI toolkit, widget library, layout engine, node graph
editor, SDL wrapper, GPU abstraction, ECS, scene graph, or cross-platform backend
system.

## Current patch state

The repository includes the initial source foundation, Linux/X11 window
lifecycle, event pump, and frame-stable input snapshots. Software rendering,
file I/O, debug overlay, canvas behavior, and replay implementation are
intentionally deferred to later focused patches.

## Examples

- `examples/00_empty_window` opens and closes a basic X11 window.
- `examples/01_input_snapshot` prints keyboard, mouse, wheel, and modifier
  transitions from `WplInputState`.

## Build

```sh
./scripts/build.sh
```

## Test

```sh
./scripts/test.sh
```
