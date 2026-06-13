# WPL Timing and Frame Contract

This document records the Phase 3 timing and frame-boundary contract for WPL. It
is a contract document only. It does not change runtime behavior or add new timing
APIs.

WPL timing exists to support predictable platform-layer frame loops, input
snapshots, software rendering, debug overlays, and replay diagnostics. It must
remain small, monotonic, backend-clean, and application-policy neutral.

## Current Public API

The current public timing API is intentionally narrow:

```c
double wpl_time_seconds(void);
float wpl_window_delta_time(const WplWindow* window);
```

`wpl_time_seconds` returns monotonic process time in seconds. On supported Linux
hosts it is backed by `clock_gettime(CLOCK_MONOTONIC, ...)`.

`wpl_window_delta_time` returns the most recently computed frame delta for the
window. It returns `0.0f` for `NULL`.

## Monotonic Clock Rules

WPL timing must use a monotonic clock for frame deltas and elapsed-time
measurements. It must not use wall-clock time for frame delta calculation.

The monotonic clock contract is:

- values are seconds as `double`,
- values should not move backward during normal execution,
- values are process-local observations rather than serialized replay data,
- unexpected clock failure returns `0.0`,
- public headers expose no POSIX or backend-native clock handles.

Applications must not treat `wpl_time_seconds` as calendar time. It is for elapsed
time, frame diagnostics, and local pacing decisions.

## Frame Delta Rules

`wpl_begin_frame(window)` is the frame-delta boundary for a `WplWindow`.

Current behavior:

- `wpl_create_window` initializes `window->last_time` from `wpl_time_seconds`.
- `window->delta_time` starts at `0.0f` after creation.
- Each successful `wpl_begin_frame` samples `wpl_time_seconds`.
- If the sampled value is greater than or equal to the previous value, WPL stores
  the elapsed seconds as `window->delta_time`.
- If the sampled value is earlier than the previous value, WPL stores `0.0f`.
- `window->last_time` is updated to the sampled value.
- `wpl_window_delta_time(window)` returns the last stored value.

Delta time is per-window backend-owned state. It must not be stored in hidden
global frame state.

## Frame Lifecycle Relationship

The normal frame order remains:

```c
wpl_begin_frame(window);
wpl_pump_events(window);
input = wpl_get_input(window);
/* application update */
/* build draw list */
wpl_submit_draw_list(window, draw);
wpl_end_frame(window);
```

`wpl_begin_frame` currently does two platform-layer jobs:

- reset transient input fields,
- compute the latest frame delta.

`wpl_pump_events` does not compute frame delta.

`wpl_get_input` does not compute frame delta.

`wpl_submit_draw_list` does not compute frame delta.

`wpl_end_frame` presents the current software framebuffer. It does not reset input
and does not currently perform frame pacing.

## Frame Pacing Boundary

WPL currently does not expose:

- a sleep function,
- a yield function,
- a target-FPS API,
- a vsync API,
- a frame limiter,
- an event wait API,
- scheduler priority controls.

Frame pacing is currently application-owned. Applications may use
`wpl_time_seconds` and their own Linux/POSIX sleep policy if they want to pace the
main loop.

Phase 3c adds `scripts/check_no_frame_pacing_api.sh` as a lightweight public-header
boundary check. The script intentionally scans only `include/wpl` for pacing-like
public API names. It does not ban private backend helpers, examples, tests, or
application-side pacing experiments.

Future frame pacing support must remain narrow. It should not add a game loop,
scheduler, job system, GUI loop, or renderer policy. If added, it must document
oversleep behavior, interruption behavior, clock source, whether it blocks the
calling thread, and how it interacts with event pumping.

## Replay Boundary

Replay records platform/input snapshots, not live monotonic clock samples.

Replay determinism must not depend on the wall clock or on the current value of
`wpl_time_seconds`. If replay later records frame timing, it should record an
explicit frame delta or timestamp field in the replay format rather than deriving
replay timing from live clock queries during playback.

## Debug Overlay Boundary

Debug overlays may display frame time, FPS, and timing diagnostics. Those values
are observations of the current run. They are not stable serialized state and must
not define application update policy.

## Backend Evolution Rules

Future backend or multi-window work must preserve these timing rules unless a
patch explicitly updates this contract:

- frame delta is per-window state,
- frame delta is computed from a monotonic clock,
- frame delta is updated at `wpl_begin_frame`,
- invalid or regressing clock samples produce a safe nonnegative delta,
- public headers expose no backend-native timing handles,
- WPL does not add frame pacing policy without an explicit API and tests,
- replay stays deterministic and does not consume live clock samples as recorded
  input.

## Validation Requirements

Timing changes should add tests or documented validation for:

- `wpl_time_seconds` monotonic non-regression under normal execution,
- `wpl_window_delta_time(NULL) == 0.0f`,
- initial window delta behavior,
- begin-frame delta update behavior,
- sanitizer-clean timing code,
- backend-leak checks for public headers,
- frame-pacing boundary checks for public headers.

Run the frame-pacing boundary check with:

```sh
./scripts/check_no_frame_pacing_api.sh
```

The check must pass unless a patch explicitly introduces a public frame-pacing API
and updates this contract at the same time.
