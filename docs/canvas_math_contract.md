# WPL Canvas Math Contract

This document records the Phase 6 canvas math and pan/zoom contract for WPL. It
is a contract document only. It does not change runtime behavior, public API, or
application semantics.

WPL's canvas math helpers provide small backend-independent coordinate and
rectangle operations for canvas-heavy custom UI foundations. They are not a GUI
widget system, layout engine, scene graph, node graph model, selection system,
interaction framework, camera system, or editor command layer.

## Current Canvas Shape

The public canvas API is pure value-based C API surface:

```c
WplResult wpl_screen_to_canvas(const WplCanvasView* view,
                               WplVec2 screen,
                               WplVec2* out_canvas);

WplResult wpl_canvas_to_screen(const WplCanvasView* view,
                               WplVec2 canvas,
                               WplVec2* out_screen);

WplResult wpl_canvas_pan_by(WplCanvasView* view, WplVec2 screen_delta);

WplResult wpl_canvas_zoom_around(WplCanvasView* view,
                                 WplVec2 cursor_screen,
                                 float zoom_factor);

bool wpl_rect_contains_point(WplRect rect, WplVec2 point);
bool wpl_rect_intersects(WplRect a, WplRect b);
WplRect wpl_rect_intersection(WplRect a, WplRect b);
```

The module does not own windows, input state, draw lists, renderer state, X11
handles, application objects, or editor selection state.

## Coordinate Contract

`WplCanvasView` stores:

- `pan`: a screen-space pixel offset,
- `zoom`: pixels per canvas unit,
- `min_zoom`: positive lower zoom bound,
- `max_zoom`: positive upper zoom bound.

Current coordinate conversion behavior:

- screen-to-canvas uses `(screen - pan) / zoom`,
- canvas-to-screen uses `canvas * zoom + pan`,
- valid conversions are inverse operations within floating-point precision,
- all input vectors and view fields must be finite,
- `zoom`, `min_zoom`, and `max_zoom` must be positive,
- `min_zoom` must be less than or equal to `max_zoom`,
- invalid arguments return `WPL_RESULT_INVALID_ARGUMENT`.

The API uses single-precision floats. Callers must not require exact round trips
for extremely large values or values that overflow finite float arithmetic.

## Pan Contract

`wpl_canvas_pan_by` applies a screen-space pixel delta directly to `view->pan`.
It does not convert pan deltas through canvas units.

Current behavior:

- valid finite deltas update `pan`,
- `zoom`, `min_zoom`, and `max_zoom` are preserved,
- invalid views or non-finite deltas return `WPL_RESULT_INVALID_ARGUMENT`,
- overflowed pan results are rejected rather than stored,
- failure preserves the original view.

## Zoom Contract

`wpl_canvas_zoom_around` zooms around a screen-space cursor position while
preserving the canvas point under that cursor.

Current behavior:

- `zoom_factor` must be finite and positive,
- the requested zoom is clamped to `[min_zoom, max_zoom]`,
- the post-zoom pan is adjusted so the cursor-anchored canvas point is preserved,
- if the computed view is invalid or non-finite, the function returns
  `WPL_RESULT_INVALID_ARGUMENT`,
- failure preserves the original view.

The function is a coordinate transform helper only. It does not interpret mouse
wheels, gestures, focus state, editor modes, snapping, minimaps, camera bounds, or
node graph navigation policy.

## Rectangle Contract

`WplRect` uses `{x, y, w, h}` with `x`/`y` as the top-left/minimum corner and
`w`/`h` as size.

Current behavior:

- negative-width or negative-height rectangles are empty,
- non-finite rectangles are invalid and treated as empty by boolean helpers,
- zero-area rectangles do not contain points and do not intersect,
- point containment includes left/top edges and excludes right/bottom edges,
- intersections require positive-area overlap,
- touching edges do not count as intersection,
- invalid or non-overlapping intersections return `{0, 0, 0, 0}`.

Rectangle helpers are low-level math utilities. They do not imply hit-test
priority, z-order, selection rules, drag handles, scroll regions, panels, widgets,
or node graph semantics.

## Backend and Ownership Boundary

Canvas math is backend-independent and lives in core code. It must not include X11
headers, use backend-private handles, allocate renderer resources, mutate input
snapshots, or own draw command buffers.

The application decides how canvas coordinates map to UI behavior. WPL only
provides deterministic coordinate and rectangle operations.

## Non-goals

The canvas math module deliberately does not provide:

- GUI widgets,
- layout,
- retained UI trees,
- scene graphs,
- node graph storage,
- node/link hit-test policy,
- selection state,
- drag/drop behavior,
- snapping policy,
- camera constraints,
- minimap behavior,
- scrollbars,
- animation,
- high-DPI policy,
- input gesture recognition,
- renderer resources,
- backend handles.

## Validation Requirements

Canvas changes should add tests or documented validation for:

- coordinate conversion round trips,
- pan updates in screen-space pixels,
- cursor-anchored zoom invariants,
- min/max zoom clamping,
- failure preserving the original view,
- invalid and non-finite input rejection,
- point containment boundary rules,
- rectangle intersection and empty-rectangle rules,
- overflowed rectangle edge rejection,
- backend-leak checks for public headers.

Current focused canvas validation target:

```sh
ctest --test-dir build --output-on-failure -R 'wpl_test_canvas$'
```

The focused canvas target covers coordinate round trips, screen-space pan,
cursor-anchored zoom, zoom clamping, failure preservation, invalid/non-finite
inputs, point containment boundary rules, rectangle intersection rules,
zero-area and negative-size rectangles, non-finite rectangles, overflowed
rectangle edges, and transform edge behavior.

Focused canvas validation does not replace full CTest, sanitizer validation,
backend-leak checks, frame-pacing boundary checks, or Xvfb smoke validation.

Phase 6a documented this contract. Phase 6b added rectangle edge coverage. Phase
6c added pan/zoom transform edge coverage. Phase 6d syncs this validation
documentation and closes the Phase 6 canvas math hardening pass.
