# WPL Examples

Examples are graphical Linux/X11 programs. They are built by the default CMake build and by CI, but CI does not run them because they require an X server. Manual validation requirements are listed in `docs/validation.md`.

## 00_empty_window

Run:

```sh
./build/examples/00_empty_window/wpl_empty_window
```

Purpose:

- Demonstrates X11 window creation, event pumping, and clean shutdown.

Deliberately does not demonstrate:

- Rendering beyond the platform lifecycle.
- Input visualization.
- Widgets, layout, or editor behavior.

## 01_input_snapshot

Run:

```sh
./build/examples/01_input_snapshot/wpl_input_snapshot
```

Purpose:

- Demonstrates frame-stable `WplInputState` snapshots.
- Prints mouse, keyboard, wheel, and modifier transitions.

Deliberately does not demonstrate:

- Raw X11 events.
- Input rebinding.
- Widget event dispatch.

## 02_draw_primitives

Run:

```sh
./build/examples/02_draw_primitives/wpl_draw_primitives
```

Purpose:

- Demonstrates software-rendered clear, filled rectangles, outlines, lines,
  circles, alpha blending, and ASCII bitmap text.

Deliberately does not demonstrate:

- GPU rendering.
- Clipping API.
- Anti-aliasing.
- Rich text, Unicode shaping, or font loading.
- Widgets or layout.

## 03_canvas_pan_zoom

Run:

```sh
./build/examples/03_canvas_pan_zoom/wpl_canvas_pan_zoom
```

Purpose:

- Demonstrates public canvas math APIs.
- Shows canvas-to-screen conversion, screen-to-canvas conversion, mouse-drag
  panning, and cursor-anchored wheel zoom.
- Draws a transformed canvas-space grid, origin marker, and simple shapes.

Deliberately does not demonstrate:

- Node graph objects or node selection.
- Editor modes, handles, widgets, or layout.
- Renderer transform stacks or retained canvas objects.

## 04_debug_overlay

Run:

```sh
./build/examples/04_debug_overlay/wpl_debug_overlay
```

Purpose:

- Demonstrates the append-only debug overlay.
- Shows backend name, timing, window size, mouse state, wheel delta, and draw
  command count using normal draw commands.

Deliberately does not demonstrate:

- Direct overlay rendering.
- Panels, docking, menus, or widgets.
- Backend querying from the overlay module.

## 05_input_replay

Record:

```sh
./build/examples/05_input_replay/wpl_input_replay record /tmp/wpl_input.replay
```

Playback:

```sh
./build/examples/05_input_replay/wpl_input_replay playback /tmp/wpl_input.replay
```

Purpose:

- Demonstrates public replay recorder/player APIs.
- Records `WplInputState` snapshots and frame deltas.
- Loads a replay file and visualizes replayed snapshots.

Deliberately does not demonstrate:

- Raw X11 event replay.
- Editor command replay.
- Simulation replay.
- Timeline UI, pause/resume controls, or scrubbing.
- Replay format changes.

## Manual Runtime Requirements

All examples require Linux with an X11 or XWayland display. CI builds the examples but does not run them. Use `scripts/manual_graphical_smoke.sh` to print or run the manual smoke-test commands.
