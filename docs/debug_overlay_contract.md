# WPL Debug Overlay Contract

This document records the Phase 7 debug overlay contract for WPL. It is a
contract document only. It does not change runtime behavior, public API, tests,
or application semantics.

WPL's debug overlay helpers append diagnostic draw commands into a caller-owned
`WplDrawList`. The overlay is platform-layer diagnostics infrastructure. It is
not a GUI widget system, layout engine, retained overlay tree, inspector panel,
profiler UI, editor command surface, or node graph debugger.

## Current Debug Overlay Shape

The public debug overlay API is value-based C API surface:

```c
typedef struct WplDebugStats {
  float fps;
  float frame_ms;
  int window_width;
  int window_height;
  size_t draw_command_count;
  const char* backend_name;
} WplDebugStats;

typedef struct WplDebugLine {
  const char* label;
  const char* value;
} WplDebugLine;

WplResult wpl_debug_draw_overlay(WplDrawList* list,
                                 const WplDebugStats* stats,
                                 const WplInputState* input);

WplResult wpl_debug_draw_overlay_ex(WplDrawList* list,
                                    const WplDebugStats* stats,
                                    const WplInputState* input,
                                    const WplDebugLine* lines,
                                    size_t line_count);
```

The module reads caller-provided stats and input snapshots. It does not own,
mutate, sample, or poll platform state. It does not allocate renderer resources or
own a window.

## Draw Command Contract

The debug overlay is append-only with respect to caller draw lists:

- successful calls append draw commands to the supplied list,
- existing commands before the overlay call remain in place,
- the base overlay currently requires ten draw commands,
- `wpl_debug_draw_overlay_ex` requires one additional draw command per custom
  line,
- insufficient capacity returns `WPL_RESULT_CAPACITY_EXCEEDED`,
- failed calls must not leave partial overlay commands in the list.

The overlay describes diagnostics using existing draw commands. It does not add a
new renderer path, retained render state, GPU abstraction, text system, or backend
presentation API.

## Input and Stats Contract

`WplDebugStats` is caller-provided diagnostic data. The overlay formats it into
text commands. `backend_name == NULL` is accepted and displayed as `unknown`.

`WplInputState` is read as an already-stable input snapshot. The overlay does not
poll events, reset transient input, infer text input, interpret editor commands,
or widen input semantics.

## Custom Line Contract

`wpl_debug_draw_overlay_ex` may append caller-provided diagnostic lines:

- `lines == NULL` is valid only when `line_count == 0`,
- each custom line requires non-null `label` and `value`,
- each custom line is formatted as `label: value`,
- custom line formatting uses a fixed internal buffer,
- overly long formatted custom lines return `WPL_RESULT_TRUNCATED`,
- very large `line_count` values return `WPL_RESULT_UNSUPPORTED`,
- failed validation must not mutate the draw-list count.

Custom lines are diagnostics only. WPL does not interpret labels, values, graph
state, editor modes, profiling categories, or application data schemas.

## Backend and Ownership Boundary

The debug overlay lives in core code and must remain backend-independent. It must
not include X11 headers, expose backend-native handles, own X11 resources, or call
backend presentation paths.

The caller owns:

- the draw list,
- the input snapshot,
- the debug stats values,
- custom line storage,
- custom line string lifetimes.

WPL consumes those values during the call only and emits draw commands.

## Failure and Mutation Rules

Debug overlay failures must be simple and deterministic:

- null required pointers return `WPL_RESULT_INVALID_ARGUMENT`,
- capacity exhaustion returns `WPL_RESULT_CAPACITY_EXCEEDED`,
- unsupported line counts return `WPL_RESULT_UNSUPPORTED`,
- formatting truncation returns `WPL_RESULT_TRUNCATED`,
- no failed call should commit a partial overlay append.

Validation may happen before all other validation when capacity is already
insufficient. Callers must not depend on validation order except for the returned
`WplResult` and the no-partial-mutation rule.

## Non-goals

The debug overlay module deliberately does not provide:

- GUI widgets,
- layout,
- panels,
- retained UI state,
- scrollable views,
- graph inspectors,
- editor command controls,
- live profiling storage,
- logging sinks,
- input event capture,
- replay recording,
- renderer resources,
- backend handles,
- GPU acceleration,
- font loading,
- Unicode shaping.

## Validation Requirements

Debug overlay changes should add tests or documented validation for:

- invalid argument rejection,
- null backend-name fallback,
- base overlay command count,
- custom line command count,
- existing draw command preservation,
- capacity failure without mutation,
- large line-count rejection,
- invalid custom line rejection,
- truncation failure without mutation,
- backend-leak checks for public headers.

Current focused debug overlay validation target:

```sh
ctest --test-dir build --output-on-failure -R 'wpl_test_debug_overlay$'
```

Focused debug overlay validation does not replace full CTest, sanitizer
validation, backend-leak checks, frame-pacing boundary checks, focused draw,
renderer, canvas validation, or Xvfb smoke validation.

Phase 7a adds this contract document and cross-references. Additional Phase 7
patches can add edge tests or implementation hardening where concrete coverage
needs are identified.
