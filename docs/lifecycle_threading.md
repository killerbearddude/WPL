# WPL Lifecycle and Threading Contract

This document records the Phase 0 lifecycle and threading contract for post-v0.1
WPL development.  It is intentionally conservative: WPL remains a small
Linux-native C11 platform layer, and later backend/multi-window work must preserve
these public assumptions unless a patch explicitly changes the contract.

## Public API Lifecycle

WPL uses explicit ownership.  Every successful create function has a matching
destroy function, and destroy functions tolerate `NULL` unless a specific API
says otherwise.

The normal window frame order is:

```c
wpl_begin_frame(window);
wpl_pump_events(window);
input = wpl_get_input(window);
/* application update */
/* build draw list */
wpl_submit_draw_list(window, draw);
wpl_end_frame(window);
```

`wpl_begin_frame` is the frame boundary for transient input and frame delta time.
It resets per-frame input transitions, computes the latest delta time, and
prepares the backend-owned frame state for the next event pump.

`wpl_pump_events` consumes pending backend events into the current frame snapshot.
Calling it more than once during a frame is valid, but events accumulate into the
same frame snapshot.  It does not reset transient input fields.

`wpl_get_input` returns a value snapshot and does not mutate window state.
Applications must treat the returned data as frame-local observation, not as an
owned backend event stream.

`wpl_submit_draw_list` consumes the command list into WPL's current software
framebuffer.  The application retains ownership of the draw list.

`wpl_end_frame` presents the framebuffer.  It does not reset input.  A minimized
or zero-sized presentation target must remain a valid lifecycle state and should
skip presentation rather than turning the window invalid.

## Object Ownership

- `WplWindow` is opaque and backend-owned after creation.
- `WplDrawList` is caller-owned and backend-independent.
- `WplReplayRecorder` and `WplReplayPlayer` are caller-owned replay objects.
- `WplFileData` returned by file reads is caller-owned until
  `wpl_free_file_data`.
- Public headers must not expose X11, XKB, XImage, Visual, GC, Atom, or other
  backend-specific types.

Partial construction failures must clean up backend resources before returning.
When a creation API fails, the output pointer must be set to `NULL`.

## Threading Contract

Unless a function explicitly documents otherwise, WPL APIs are single-threaded.
Callers must invoke WPL functions from one owner thread or provide external
synchronization around all WPL access.

The current backend assumes single-threaded access to:

- window lifecycle and event pumping,
- input snapshots,
- framebuffer and presentation state,
- draw-list mutation,
- replay recorder/player mutation,
- global logging callback state.

This contract is deliberate for WPL's current scope.  It keeps the platform layer
small, inspectable, deterministic, and easy to validate under sanitizers.

## Allowed Cross-Thread Behavior

WPL does not currently provide cross-thread window operations, asynchronous event
delivery, or background renderer submission.

Applications may use other threads for their own work only if they do not touch
WPL-owned objects concurrently with the owner thread.  Data handed to WPL must be
fully synchronized by the application before WPL reads it.

## Backend Evolution Rules

Future Linux backend abstraction and multi-window work must preserve these rules:

- public handles remain opaque,
- backend-native handles stay private,
- event routing produces frame-stable snapshots,
- replay stays at the platform/input boundary,
- timing uses monotonic clocks,
- backend resource cleanup remains deterministic under partial failure,
- validation must cover lifecycle and threading assumptions before behavior is
  considered stable.

If later work introduces an explicit thread-safe API, that patch must document the
exact synchronization boundary and add targeted tests or diagnostics for it.
