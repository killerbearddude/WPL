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

This repository is currently bootstrapped as a source foundation only. The X11
window lifecycle, event pump, input translation, software renderer, file I/O,
debug overlay, and replay implementation are intentionally deferred to later
patches.

## Build

```sh
./scripts/build.sh
```

## Test

```sh
./scripts/test.sh
```
