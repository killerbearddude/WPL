# Linux/X11 Backend

WPL's Linux/X11 backend owns all Xlib interaction behind the public opaque
`WplWindow` handle.

## Backend Boundary

Public headers must not expose X11 types, cursor handles, atoms, visuals,
events, display pointers, or windows. Backend-specific state lives in
`backends/linux_x11/` implementation files and private headers.

## Cursor Shapes

The X11 backend maps public `WplCursorShape` values to X cursor font cursors.
Cursor resources are owned by the backend-private `WplWindow` and freed during
window destruction.

Current mapping:

- `WPL_CURSOR_ARROW` -> `XC_left_ptr`
- `WPL_CURSOR_HAND` -> `XC_hand2`
- `WPL_CURSOR_CROSSHAIR` -> `XC_crosshair`
- `WPL_CURSOR_MOVE` -> `XC_fleur`
- `WPL_CURSOR_NOT_ALLOWED` -> `XC_X_cursor`

The mapping is intentionally private. Public headers expose only the portable
WPL cursor enum and never expose X11 `Cursor` values or cursor font constants.

## Deferred Cursor Scope

- Cursor hiding.
- Custom cursor images.
- Animated cursors.
- Wayland cursor support.
- SDL cursor support.
