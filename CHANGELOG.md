# Changelog

## v0.1.0 - 2026-06-09

Initial validated WPL release candidate.

### Added

- Linux-native C11 platform layer foundation.
- X11 window lifecycle.
- Frame-stable input snapshots.
- Monotonic timing.
- Fixed-capacity draw command buffer.
- Software framebuffer renderer.
- Basic 2D primitives:
  - clear
  - filled rectangle
  - rectangle outline
  - line
  - filled circle
  - ASCII bitmap text
- Canvas math:
  - screen/canvas conversion
  - pan
  - zoom around cursor
  - rectangle helpers
- Debug overlay draw-command generation.
- Binary file I/O.
- Replay v1 binary format.
- Replay recorder/player.
- Public logging callback API.
- Examples:
  - `00_empty_window`
  - `01_input_snapshot`
  - `02_draw_primitives`
  - `03_canvas_pan_zoom`
  - `04_debug_overlay`
  - `05_input_replay`
- CI on Ubuntu GCC and Ubuntu Clang.
- Public header/backend leak validation scripts.
- v0.1 validation report.

### Scope

- Linux only.
- C11 only.
- X11 only.
- Software rendering only.
- MIT licensed.
- Public API does not expose X11/backend types.

### Known Limitations

- One active window.
- No high-DPI support.
- No Wayland support.
- No SDL support.
- No Windows/macOS support.
- No GPU/OpenGL/Vulkan abstraction.
- No GUI widget system.
- No layout engine.
- No node graph/editor logic.
- ASCII bitmap text only.
- No Unicode shaping.
- No clipping API.
- No anti-aliasing.
- Uncommon X11 visuals may return `WPL_RESULT_UNSUPPORTED`.
- Fallback repeat-release path is accepted as a deferred v0.1 validation risk.

### Validation

Validation evidence is recorded in:

- `docs/validation_report_v0.1.md`
