# WPL Public API Review

This document records the Phase 10 public API boundary audit for the current
post-v0.1 development branch. It is not a full API reference; public headers
remain the source of truth.

This audit is documentation-only. It does not rename, remove, or widen any
public symbol.

## Public Headers

Public headers are located under `include/wpl/`:

- `include/wpl/wpl.h`
- `include/wpl/wpl_base.h`
- `include/wpl/wpl_result.h`
- `include/wpl/wpl_window.h`
- `include/wpl/wpl_input.h`
- `include/wpl/wpl_time.h`
- `include/wpl/wpl_draw.h`
- `include/wpl/wpl_canvas.h`
- `include/wpl/wpl_file.h`
- `include/wpl/wpl_replay.h`
- `include/wpl/wpl_debug.h`
- `include/wpl/wpl_log.h`

The intended application include is:

```c
#include <wpl/wpl.h>
```

Narrow module includes are acceptable when a translation unit intentionally needs
only one subsystem.

## Public Opaque Handles

The following public handles are opaque:

- `WplWindow`
- `WplDrawList`
- `WplReplayRecorder`
- `WplReplayPlayer`

Their storage layout is private. Applications must create, destroy, and interact
with these objects through WPL functions only.

## Public Value Types

Public value types are plain C structs and scalar enums used at the platform
boundary:

- `WplVec2`
- `WplVec2i`
- `WplRect`
- `WplColor`
- `WplWindowDesc`
- `WplCursorShape`
- `WplMouseButton`
- `WplKey`
- `WplMouseState`
- `WplKeyboardState`
- `WplInputState`
- `WplTextMetrics`
- `WplDashPattern`
- `WplPanelStyle`
- `WplCanvasView`
- `WplFileData`
- `WplDebugStats`
- `WplDebugLine`
- `WplLogLevel`
- `WplLogCallback`
- public result/input/draw/replay/log enum and constant values

These types intentionally avoid backend handles, file descriptors, raw X11 event
objects, renderer internals, and raw serialized replay structs.

## Public Constants

Current public constants are part of the API surface:

- `WPL_MAX_WINDOW_DIMENSION`
- `WPL_MAX_FILE_SIZE_V0_1`
- `WPL_DRAW_TEXT_MAX_BYTES`
- `WPL_REPLAY_MAGIC`
- `WPL_REPLAY_VERSION`
- `WPL_REPLAY_VERSION_V1`
- `WPL_REPLAY_HEADER_SIZE_V1`
- `WPL_REPLAY_FRAME_SIZE_V1`
- `WPL_REPLAY_KEY_COUNT_V1`
- `WPL_REPLAY_MAX_FRAMES_V1`
- `WPL_REPLAY_VALID_FLAGS_V1`

`WPL_MAX_FILE_SIZE_V0_1` and the replay v1 constants intentionally keep their
versioned names. They should not be renamed in a cleanup-only patch because that
would create unnecessary API churn.

## Result Policy

Fallible public functions use `WplResult`.

- Required `NULL` pointer: `WPL_RESULT_INVALID_ARGUMENT`
- Invalid value or invalid state: `WPL_RESULT_INVALID_ARGUMENT`
- Allocation failure: `WPL_RESULT_OUT_OF_MEMORY`
- Draw-list or replay capacity limit: `WPL_RESULT_CAPACITY_EXCEEDED`
- Linux/X11 platform failure: `WPL_RESULT_PLATFORM_ERROR`
- File I/O failure: `WPL_RESULT_IO_ERROR`
- Malformed replay bytes: `WPL_RESULT_PARSE_ERROR`
- v0.1 limit or unimplemented feature: `WPL_RESULT_UNSUPPORTED`
- Truncated accepted input: `WPL_RESULT_TRUNCATED`
- Generic unexpected failure: `WPL_RESULT_ERROR`

## Public Function Inventory

### Window and frame lifecycle

- `wpl_create_window`
- `wpl_destroy_window`
- `wpl_window_should_close`
- `wpl_window_request_close`
- `wpl_set_cursor_shape`
- `wpl_begin_frame`
- `wpl_pump_events`
- `wpl_end_frame`
- `wpl_window_width`
- `wpl_window_height`

### Input and timing

- `wpl_get_input`
- `wpl_time_seconds`
- `wpl_window_delta_time`

### Draw command buffer and drawing helpers

- `wpl_create_draw_list`
- `wpl_destroy_draw_list`
- `wpl_draw_list_clear`
- `wpl_draw_list_count`
- `wpl_draw_list_capacity`
- `wpl_draw_clear`
- `wpl_draw_rect`
- `wpl_draw_rounded_rect`
- `wpl_draw_panel`
- `wpl_draw_rect_outline`
- `wpl_draw_line`
- `wpl_draw_polyline`
- `wpl_draw_dashed_line`
- `wpl_draw_push_clip`
- `wpl_draw_pop_clip`
- `wpl_draw_circle`
- `wpl_draw_text`
- `wpl_measure_text`
- `wpl_text_line_height`
- `wpl_text_glyph_advance_x`
- `wpl_submit_draw_list`

`wpl_draw_panel` and `wpl_draw_rounded_rect` are drawing helpers only. They do
not introduce widget identity, layout, retained UI state, scroll policy, input
routing, focus behavior, or graph/editor semantics.

### Canvas math

- `wpl_screen_to_canvas`
- `wpl_canvas_to_screen`
- `wpl_canvas_pan_by`
- `wpl_canvas_zoom_around`
- `wpl_rect_contains_point`
- `wpl_rect_intersects`
- `wpl_rect_intersection`

Canvas APIs remain backend-independent math helpers. They do not own UI behavior,
selection state, graph hit testing, widget layout, or editor command policy.

### File I/O

- `wpl_read_entire_file`
- `wpl_write_entire_file`
- `wpl_write_entire_file_atomic`
- `wpl_free_file_data`

File I/O remains whole-file platform infrastructure. It does not define project
formats, graph serialization, asset pipelines, or application persistence policy.

### Replay

- `wpl_replay_recorder_create`
- `wpl_replay_recorder_destroy`
- `wpl_replay_recorder_begin`
- `wpl_replay_recorder_record_frame`
- `wpl_replay_recorder_save`
- `wpl_replay_player_create`
- `wpl_replay_player_destroy`
- `wpl_replay_player_load`
- `wpl_replay_player_next`

Replay remains at the platform/input snapshot boundary. It does not expose raw
X11 events, replay binary structs, editor commands, graph state, application
state, event synthesis, or timeline controls.

### Debug overlay and logging

- `wpl_debug_draw_overlay`
- `wpl_debug_draw_overlay_ex`
- `wpl_set_log_callback`
- `wpl_log`

The debug overlay appends draw commands to a caller-owned draw list. It owns no
persistent UI state. The logging API stores a process-global callback pointer;
that global callback is intentional and is not a platform backend handle.

## Ownership Rules

- `WplWindow` is owned by the caller after `wpl_create_window` succeeds and is
  released with `wpl_destroy_window`.
- `WplDrawList` is owned by the caller after `wpl_create_draw_list` succeeds and
  is released with `wpl_destroy_draw_list`.
- `WplFileData.data` is owned by the caller after successful read and is released
  with `wpl_free_file_data`.
- `WplReplayRecorder` and `WplReplayPlayer` are owned by the caller after create
  functions succeed and are released with their destroy functions.
- Draw helper inputs, debug overlay custom lines, replay input snapshots, and log
  messages remain caller-owned unless a function explicitly copies data into WPL
  storage.
- The debug overlay appends commands to a caller-owned draw list; it owns no
  persistent state.
- The logging API stores a process-global callback pointer supplied by the
  application; it performs no allocation and owns no message storage.

## Null Behavior

Expected safe-null behavior:

- Destroy functions tolerate `NULL`.
- `wpl_free_file_data(NULL)` is safe.
- Scalar accessors return safe zero values for `NULL` where applicable.
- `wpl_window_should_close(NULL)` returns true.
- `wpl_get_input(NULL)` returns a zeroed input snapshot.
- Logging accepts a null callback to disable logging.
- `wpl_log` accepts a null message and delivers it as an empty string.

Required output pointers for fallible APIs remain required and return
`WPL_RESULT_INVALID_ARGUMENT` when `NULL`.

## Backend Leakage Audit

Public headers must not include X11 headers or expose X11/private backend types.
Forbidden public types include:

- `Display`
- raw X11 `Window`
- `XEvent`
- `XImage`
- `GC`
- `Atom`
- `Visual`
- `Colormap`
- `KeySym`
- `KeyCode`
- `Xkb`

`WplWindow` is an allowed WPL opaque handle and is not the X11 `Window` type.

Backend leakage is checked with:

```sh
./scripts/check_no_backend_leaks.sh
./scripts/check_public_headers.sh
```

## Phase 10 API Audit Result

No hard API conformance blocker is identified by this Phase 10a audit. The
current public API remains small, C-compatible, Linux-native, backend-isolated,
and scoped to platform services.

The following current API surface areas are intentionally accepted as platform
layer infrastructure, not higher-level UI/editor behavior:

- cursor shape selection,
- rounded rectangle drawing,
- panel draw helper,
- dashed line helper,
- polyline helper,
- clip rectangle stack,
- text measurement helpers,
- atomic whole-file write,
- replay recorder/player,
- debug overlay custom lines.

## Deferred API Areas

The following areas are intentionally not part of the current public API:

- allocator hooks,
- public framebuffer readback,
- transform stack API,
- font selection or font loading,
- high-DPI policy,
- multi-window management,
- Wayland or cross-platform backend selection,
- public frame pacing, sleep, yield, target-FPS, or event-wait APIs,
- replay timeline controls,
- raw event recording or event synthesis,
- UI widget/layout APIs,
- node graph/editor APIs,
- graph serialization or project file formats,
- asset pipeline APIs.

## Cleanup Guidance

API cleanup patches must stay small and explicit. They should prefer documented
compatibility over churn.

Allowed cleanup work:

- update stale documentation,
- add missing focused tests,
- clarify comments without widening scope,
- tighten validation around existing behavior,
- remove private/backend leakage if discovered.

Avoid cleanup work that:

- renames public symbols without a concrete blocker,
- removes public constants used by existing callers,
- adds GUI/widget/layout semantics,
- adds graph/editor policy,
- exposes backend handles,
- broadens replay beyond input snapshots,
- adds new platform support.
