# WPL Public API Review

This document records the v0.1 public API boundary review. It is not a full API
reference; public headers remain the source of truth.

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
- `WplRect`
- `WplColor`
- `WplInputState`
- `WplWindowDesc`
- `WplCanvasView`
- `WplFileData`
- `WplDebugStats`
- public result/input/draw/replay enum and constant values

These types intentionally avoid backend handles, file descriptors, and raw
serialized replay structs.

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

## Ownership Rules

- `WplWindow` is owned by the caller after `wpl_create_window` succeeds and is
  released with `wpl_destroy_window`.
- `WplDrawList` is owned by the caller after `wpl_create_draw_list` succeeds and
  is released with `wpl_destroy_draw_list`.
- `WplFileData.data` is owned by the caller after successful read and is released
  with `wpl_free_file_data`.
- `WplReplayRecorder` and `WplReplayPlayer` are owned by the caller after create
  functions succeed and are released with their destroy functions.
- The debug overlay appends commands to a caller-owned draw list; it owns no
  persistent state.

## Null Behavior

Expected safe-null behavior:

- Destroy functions tolerate `NULL`.
- `wpl_free_file_data(NULL)` is safe.
- Scalar accessors return safe zero values for `NULL` where applicable.
- `wpl_window_should_close(NULL)` returns true.
- `wpl_get_input(NULL)` returns a zeroed input snapshot.

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

`WplWindow` is an allowed WPL opaque handle and is not the X11 `Window` type.

Backend leakage is checked with:

```sh
./scripts/check_no_backend_leaks.sh
./scripts/check_public_headers.sh
```

## Deferred API Areas

The following areas are intentionally not part of the v0.1 public API:

- allocator hooks
- public framebuffer readback
- clipping stack API
- transform stack API
- font selection or font loading
- high-DPI policy
- multi-window management
- Wayland or cross-platform backend selection
- replay timeline controls
- UI widget/layout APIs
- node graph/editor APIs

## v0.1 API Blockers

No known API blockers were identified in this review. Remaining release work is
validation-oriented: confirm CI, clean Ubuntu quickstart, graphical example smoke
tests, and final tag checklist completion.
