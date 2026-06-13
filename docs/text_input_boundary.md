# WPL Text Input Boundary

This document records the Phase 2d text-input boundary for WPL. It is a contract
and scope document only. It does not introduce a text input API or change current
runtime behavior.

WPL currently exposes keyboard state, mouse state, modifiers, and wheel input as
frame-stable `WplInputState` snapshots. It does not expose committed text,
UTF-8 input, IME composition, clipboard contents, text editing commands, cursor
movement policy, selection policy, or text layout.

## Objective

Keep keyboard input and text input clearly separated before any future text API is
added.

Keyboard state answers questions such as:

- is this physical/logical key currently down?
- did this key transition this frame?
- are Shift, Control, or Alt down?

Text input answers a different question:

- what user-intended text was committed this frame?

Those are not equivalent. Future text input must not be inferred by overloading
`keyboard.key_pressed[]` or by requiring application code to map key enums to
characters.

## Current Public Behavior

Current WPL public input contains:

- key down state,
- key pressed and released transitions,
- modifier state,
- mouse position and movement delta,
- mouse button state and transitions,
- vertical wheel delta.

Current WPL public input does not contain:

- committed text,
- UTF-8 byte buffers,
- IME preedit/composition text,
- dead-key composition state,
- clipboard contents,
- text cursor or selection state,
- key repeat text events,
- application/editor commands such as copy, paste, undo, redo, delete word, or
  move cursor by word.

## Non-goals

Phase 2d does not add:

- a text input API,
- Unicode normalization,
- text shaping,
- font fallback,
- bidi processing,
- clipboard support,
- IME candidate UI,
- text editing widgets,
- command routing,
- editor behavior,
- node graph text labels,
- layout policy.

## Future API Rules

If WPL later adds text input, it should be a narrow platform-layer service with
these rules:

- Text input must be explicit in the public API.
- Text input must remain per-window.
- Text input must be frame-stable and snapshot-compatible.
- Text input must not expose X11/XIM/XKB types in public headers.
- Text input must not add widget, editor, or layout behavior.
- Text input must distinguish committed text from key state.
- Text input must document encoding, buffer ownership, truncation behavior, and
  reset timing.
- Text input must preserve replay determinism at the platform/input boundary.

A future minimal shape could be a bounded per-frame committed UTF-8 buffer owned
by WPL and copied out through `wpl_get_input` or a separate query. That is only a
direction, not an accepted API design.

## Encoding Boundary

Any future committed text should use UTF-8 at the public API boundary. Public API
users should not need to consume platform-native keysyms, keycodes, compose state,
or XIM-specific handles.

The API must specify:

- whether text is null-terminated or length-delimited,
- maximum bytes per frame,
- what happens when input exceeds the buffer,
- whether invalid UTF-8 can appear,
- whether line breaks and control characters are included,
- whether key repeat can generate repeated text.

Until that API exists, WPL makes no public committed-text guarantees.

## Composition Boundary

IME preedit/composition is not committed text. WPL should not expose partial
composition state accidentally through key transitions.

If composition is supported later, that patch must explicitly define whether WPL
exposes:

- committed text only,
- committed text plus preedit text,
- preedit cursor/range data,
- composition start/update/end diagnostics.

The default preferred scope is committed text only unless a concrete application
need requires more.

## Clipboard Boundary

Clipboard is separate from text input. Paste behavior is an application/editor
policy decision unless WPL later adds a low-level clipboard service.

A future clipboard service must be documented independently and must not turn WPL
into a text editor, widget toolkit, or command system.

## Replay Boundary

Replay must remain at the platform/input boundary. If text input is added later,
replay should record committed text snapshots rather than raw X11 events, XIM
messages, editor commands, or widget state.

Replay files must continue to avoid backend-native event streams.

## Validation Requirements

A future text-input patch must add tests or documented manual validation for:

- per-frame reset behavior,
- per-window ownership,
- UTF-8 length and truncation behavior,
- replay round-trip behavior if text is recorded,
- backend-leak checks for public headers,
- at least one X11/XWayland manual text-entry smoke path if live text input is
  implemented.

Phase 2d adds only this boundary document. No runtime validation beyond normal
build, test, sanitizer, public-header, backend-leak, and Xvfb smoke checks is
required.
