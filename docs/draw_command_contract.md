# WPL Draw Command Buffer Contract

This document records the Phase 4 draw command buffer contract for WPL. It is a
contract document only. It does not change runtime behavior or add new draw APIs.

WPL draw lists are fixed-capacity command buffers for software-rendered 2D
primitives. They describe rendering intent only. They do not encode widgets,
layout, editor commands, node graph semantics, scene graphs, retained UI state, or
application behavior.

## Current Public API Shape

The public draw API centers on an opaque `WplDrawList`:

```c
WplResult wpl_create_draw_list(size_t max_commands, WplDrawList** out_list);
void wpl_destroy_draw_list(WplDrawList* list);
WplResult wpl_draw_list_clear(WplDrawList* list);
size_t wpl_draw_list_count(const WplDrawList* list);
size_t wpl_draw_list_capacity(const WplDrawList* list);
```

Command-producing functions append to a caller-owned list:

```c
WplResult wpl_draw_clear(WplDrawList* list, WplColor color);
WplResult wpl_draw_rect(WplDrawList* list, WplRect rect, WplColor color);
WplResult wpl_draw_rounded_rect(WplDrawList* list, WplRect rect, float radius, WplColor color);
WplResult wpl_draw_panel(WplDrawList* list, WplRect rect, WplPanelStyle style);
WplResult wpl_draw_rect_outline(WplDrawList* list, WplRect rect, WplColor color, float thickness);
WplResult wpl_draw_line(WplDrawList* list, WplVec2 a, WplVec2 b, WplColor color, float thickness);
WplResult wpl_draw_polyline(WplDrawList* list, const WplVec2* points, size_t point_count, WplColor color, float thickness);
WplResult wpl_draw_dashed_line(WplDrawList* list, WplVec2 a, WplVec2 b, WplColor color, float thickness, WplDashPattern pattern);
WplResult wpl_draw_push_clip(WplDrawList* list, WplRect rect);
WplResult wpl_draw_pop_clip(WplDrawList* list);
WplResult wpl_draw_circle(WplDrawList* list, WplVec2 center, float radius, WplColor color);
WplResult wpl_draw_text(WplDrawList* list, WplVec2 position, const char* text, WplColor color);
```

Draw submission is separate from draw-list construction:

```c
WplResult wpl_submit_draw_list(WplWindow* window, const WplDrawList* list);
```

The application owns the draw list before and after submission. WPL submission may
read the list, but it does not transfer ownership of the list to the backend.

## Ownership and Lifetime

`WplDrawList` is caller-owned after successful creation.

Creation rules:

- `out_list == NULL` returns `WPL_RESULT_INVALID_ARGUMENT`.
- `max_commands == 0` returns `WPL_RESULT_INVALID_ARGUMENT`.
- creation failure leaves the output pointer as `NULL` when an output pointer is
  provided.
- successful creation starts with count `0` and capacity `max_commands`.

Destruction rules:

- `wpl_destroy_draw_list(NULL)` is valid and does nothing.
- after destruction, the caller must not use the pointer again.

Query rules:

- `wpl_draw_list_count(NULL)` returns `0`.
- `wpl_draw_list_capacity(NULL)` returns `0`.

Clear rules:

- `wpl_draw_list_clear(NULL)` returns `WPL_RESULT_INVALID_ARGUMENT`.
- clearing a valid list resets command count to `0`.
- clearing preserves capacity.
- clearing resets clip stack depth to `0`.

## Fixed-Capacity Contract

Draw lists are fixed-capacity. Appending a command never reallocates the command
buffer.

Simple append functions either append one command or return an error. Capacity
failure must not increment the public command count.

The following helpers may expand to multiple commands and must remain
all-or-nothing with respect to public command count:

- `wpl_draw_panel`,
- `wpl_draw_polyline`,
- `wpl_draw_dashed_line`.

If one of those helpers cannot append all required commands, it must preserve the
list count observed before the helper call.

## Command Ordering

Commands execute in append order. WPL does not sort, batch, merge, retain,
reorder, deduplicate, or interpret draw commands as application objects.

A command list is a sequential description of a software-rendered frame. It is not
a scene graph, retained-mode UI tree, layout tree, entity list, or node graph.

## Validation Rules

Draw APIs validate public arguments before appending commands.

General validation:

- `list == NULL` is invalid for append functions.
- colors must contain finite floating-point values.
- out-of-range but finite color components are accepted by the draw-list layer.
- positions, endpoints, rectangle coordinates, radii, and thickness values must be
  finite where applicable.

Rectangle validation:

- rectangle coordinates and dimensions must be finite.
- width and height must be nonnegative.
- derived right/bottom edges must remain finite.

Line validation:

- endpoints must be finite.
- thickness must be finite and nonnegative.
- derived bounds around the line must remain finite.

Circle validation:

- center must be finite.
- radius must be finite and nonnegative.
- derived bounds around the circle must remain finite.

Dashed-line validation:

- dash length and gap length must be finite and positive.
- zero-length dashed lines append zero commands and return success after argument
  validation.

Polyline validation:

- point array must not be `NULL`.
- point count must be at least `2`.
- every point and segment must satisfy finite line geometry requirements.

Text validation:

- text pointer must not be `NULL`.
- text position must be finite.
- accepted text length is limited by `WPL_DRAW_TEXT_MAX_BYTES` non-null bytes.
- the byte limit is not a Unicode character limit.
- over-limit text returns `WPL_RESULT_TRUNCATED` without appending a text command.

## Clip Stack Contract

Clip commands are part of the draw stream.

`wpl_draw_push_clip` validates the rectangle, appends a push command, then
increments draw-list clip depth.

`wpl_draw_pop_clip` requires current clip depth greater than zero. If depth is
zero, it returns `WPL_RESULT_INVALID_ARGUMENT` and appends nothing. On success, it
appends a pop command and decrements clip depth.

`wpl_draw_list_clear` resets clip depth to zero. Applications should not rely on a
clip region surviving across list clear boundaries.

## Text Metrics Boundary

Text measurement helpers are draw-layer support functions for ASCII bitmap text:

```c
WplResult wpl_measure_text(const char* text, WplTextMetrics* out_metrics);
float wpl_text_line_height(void);
float wpl_text_glyph_advance_x(void);
```

They do not add Unicode shaping, font fallback, bidirectional layout, text input,
selection, cursor movement, widgets, or editor behavior.

## Private Command Storage

Concrete command storage is private to WPL implementation files. Public headers
must not expose `WplDrawCommand`, command enum values, renderer internals, backend
handles, X11 types, or software framebuffer storage.

This allows the renderer and backend to consume command lists without making
command storage part of the public ABI.

## Backend and Renderer Boundary

Draw commands are backend-independent rendering intent. The software renderer and
X11 backend may interpret them privately for presentation, but public APIs must
remain backend-clean.

Submitting a draw list does not give application code access to backend-native
renderer objects, XImage state, X11 handles, visual data, or framebuffer memory.

## Non-goals

The draw command buffer deliberately does not provide:

- GUI widgets,
- layout,
- retained-mode UI,
- immediate-mode GUI policy,
- node graph logic,
- graph hit testing,
- scene graphs,
- ECS,
- GPU abstraction,
- shader/material systems,
- z-index or layer policy,
- animation systems,
- text editing behavior,
- asset loading or sprite atlases.

## Validation Requirements

Draw command changes should add tests or documented validation for:

- creation and destruction edge cases,
- count and capacity behavior,
- capacity failure preserving count,
- multi-command helper all-or-nothing behavior,
- finite geometry validation,
- text byte limit behavior,
- clip push/pop depth behavior,
- draw submission and caller-owned lifetime behavior,
- public-header backend cleanliness,
- sanitizer-clean draw code.

Current focused draw validation targets:

```sh
ctest --test-dir build --output-on-failure -R 'wpl_test_draw_(list|list_edges|submit_lifetime)$'
```

The focused targets cover the draw-list contract, edge-case ordering, and draw
submission/lifetime boundary. They do not replace full CTest, sanitizer
validation, backend-leak checks, or manual graphical smoke validation.

Phase 4a documented this contract. Phase 4b added focused draw-list edge tests.
Phase 4c added draw submission/lifetime boundary tests. Phase 4d syncs this
validation documentation and closes the Phase 4 draw command buffer hardening
pass.
