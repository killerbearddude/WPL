# WPL Input Snapshot Contract

This document records the Phase 2 input contract for WPL. It describes the public behavior that backend implementations, replay code, tests, and examples must preserve.

WPL exposes input as frame-stable snapshots. Applications observe input through `wpl_get_input(window)`. They do not consume raw backend event streams.

## Scope

The current public snapshot contains:

- keyboard key down state,
- keyboard key pressed transitions,
- keyboard key released transitions,
- modifier state for Shift, Control, and Alt,
- mouse position,
- mouse movement delta,
- mouse button down state,
- mouse button pressed transitions,
- mouse button released transitions,
- signed mouse wheel delta.

The current public snapshot does not contain text input, UTF-8 composition, clipboard state, tablet input, touch input, raw relative mouse input, or high-DPI scale information.

## Frame Lifecycle

The normal input frame order is:

```c
wpl_begin_frame(window);
wpl_pump_events(window);
input = wpl_get_input(window);
```

`wpl_begin_frame` is the input frame boundary. It resets transient input fields for the next frame and leaves held state intact.

`wpl_pump_events` consumes all pending backend events for the window and accumulates them into the current frame snapshot. Calling it more than once in a frame is valid. It must not reset transient fields.

`wpl_get_input` returns the current snapshot by value. It does not mutate window state. Passing `NULL` returns a zeroed snapshot.

`wpl_end_frame` does not reset input.

## Persistent State

Persistent state survives `wpl_begin_frame` until a later backend event changes it or the window lifecycle forces it clear.

Persistent fields are:

- `keyboard.key_down[]`,
- `keyboard.shift_down`,
- `keyboard.ctrl_down`,
- `keyboard.alt_down`,
- `mouse.button_down[]`,
- `mouse.position`.

Focus loss clears key and mouse-button down state. It also clears modifier state. This prevents stuck keys or buttons when the backend stops delivering matching release events to the window.

Mouse position is the latest known window-local position. It remains valid until updated by motion, button, enter, or leave handling. When the pointer leaves and later re-enters, the first re-entry position initializes the cursor position without producing a movement delta.

## Transient State

Transient state describes what happened during the current frame. It is reset by `wpl_begin_frame` and then accumulated by `wpl_pump_events`.

Transient fields are:

- `keyboard.key_pressed[]`,
- `keyboard.key_released[]`,
- `mouse.button_pressed[]`,
- `mouse.button_released[]`,
- `mouse.delta`,
- `mouse.wheel_delta`.

Pressed transitions are set only when the corresponding down state changes from false to true during the frame. Repeated press events for an already-down key or button keep the down state true but do not create additional pressed transitions.

Released transitions are set only when the corresponding down state changes from true to false during the frame. Release events for an already-up key or button keep the down state false but do not create additional released transitions.

If a press and release for the same key or button both occur in one frame, both transition flags may be true at the end of the frame. The final down state must reflect the last transition processed by the backend.

## Keyboard Semantics

`WPL_KEY_UNKNOWN` is not tracked as a real key. Unknown backend key symbols are ignored by the key transition arrays.

Alphabetic keys map to `WPL_KEY_A` through `WPL_KEY_Z` independent of case. Special keys are limited to the public `WplKey` enum.

Modifier state is separate from key transition arrays. Shift, Control, and Alt state are updated from backend modifier masks and from direct modifier key press or release events when necessary. The public API does not currently expose left versus right modifier identity.

Backends should suppress synthetic key-release/key-press pairs generated only by keyboard auto-repeat. Holding a key should keep `key_down[key]` true without emitting repeated released transitions. Repeated pressed transitions are not created while the key is already down.

## Mouse Semantics

Mouse position is window-local in pixels with the origin at the top-left of the window content area.

Mouse movement delta accumulates motion events during the frame. It is reset to zero by `wpl_begin_frame`. Button press and release events update the current mouse position but do not add movement delta by themselves.

Mouse wheel input is exposed as a signed frame delta:

- positive values mean wheel-up,
- negative values mean wheel-down.

The current X11 backend reports vertical wheel notches as `+1.0f` and `-1.0f`. Horizontal wheel events are ignored by the current public API.

## Snapshot Ownership

`WplInputState` is returned by value. The caller owns its copy. Modifying that copy does not change WPL state.

The authoritative mutable input state belongs to the backend-owned `WplWindow`. Input state is per-window. Implementations must not use hidden global input state for keys, mouse buttons, cursor position, wheel state, or modifiers.

## Replay Boundary

Replay records and plays back `WplInputState` snapshots at the platform/input boundary. Replay does not serialize raw X11 events, backend-native keycodes, window-system messages, editor commands, widgets, or application state.

A replayed input frame should be treated as if `wpl_get_input` returned that snapshot for the frame. Replay code must preserve the same persistent/transient field meaning as live input.

## Backend Evolution Rules

Future backend, multi-window, or text-input work must preserve these rules unless a patch explicitly updates this contract:

- input is frame-stable and snapshot-based,
- transient fields reset at `wpl_begin_frame`,
- event pumping accumulates into the current frame,
- held state persists across frames until changed or cleared by lifecycle,
- input state is per-window, not global,
- replay stays at the `WplInputState` boundary,
- public headers remain backend-clean,
- text input must be added explicitly rather than overloaded onto key state.
