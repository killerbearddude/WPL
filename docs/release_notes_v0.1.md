# WPL v0.1 Release Notes

## Summary

WPL v0.1 is the first validated release of the Whacky Platform Layer: a small MIT-licensed Linux-native C11 platform layer for canvas-heavy custom UI foundations and future node graph tools.

It provides platform/window lifecycle, frame-stable input snapshots, timing, fixed-capacity draw command buffers, software rendering, canvas math, debug overlay generation, file I/O, replay recording/playback, a public logging callback, and examples.

## Validated Commit

`7b0ad584a2e3a13fa8b7b621239b56161bb6518c`

## Validation Status

Status: **PASS**

Validation evidence:

- `docs/validation_report_v0.1.md`

## Supported Scope

- Linux
- C11
- X11/Xlib backend
- Software framebuffer rendering
- GCC
- Clang

## Non-Goals

This release does not include:

- GUI widgets
- Layout engine
- Node graph logic
- Editor behavior
- SDL
- Wayland
- Windows/macOS
- GPU abstraction
- OpenGL/Vulkan/EGL/GLX abstraction
- ECS
- Scene graph
- Asset pipeline

## Included Public Modules

- `wpl_base.h`
- `wpl_result.h`
- `wpl_log.h`
- `wpl_window.h`
- `wpl_input.h`
- `wpl_time.h`
- `wpl_draw.h`
- `wpl_canvas.h`
- `wpl_file.h`
- `wpl_replay.h`
- `wpl_debug.h`

## Examples

- `examples/00_empty_window`
- `examples/01_input_snapshot`
- `examples/02_draw_primitives`
- `examples/03_canvas_pan_zoom`
- `examples/04_debug_overlay`
- `examples/05_input_replay`

## Known Limitations

- One active window.
- X11/XWayland only.
- No native Wayland backend.
- No high-DPI model.
- ASCII bitmap text only.
- No clipping API.
- No anti-aliasing.
- No GPU path.
- Uncommon X11 visuals may be unsupported.
- Fallback repeat-release path remains a deferred validation risk.

## Release Decision

Approved for v0.1 tagging based on the zero-trust PASS gate and recorded validation evidence.
