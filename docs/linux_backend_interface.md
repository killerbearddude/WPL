# Linux Backend Interface

This document records the Phase 1 private backend interface direction for WPL.
The interface is internal implementation infrastructure only; it is not public
API and is not installed for applications.

## Objective

Introduce a narrow backend boundary so the current X11 implementation can be
wrapped before later Linux backend work.  The first slices are behavior-preserving:
X11 remains the only backend, public headers stay unchanged, and no runtime
backend selection is exposed.

## Scope

The private backend table covers the current platform-facing services:

- window creation and destruction,
- close state and close requests,
- cursor shape updates,
- frame begin/event pump/end lifecycle,
- window dimensions and frame delta access,
- draw-list submission.

## Non-goals

This phase does not add:

- Wayland support,
- runtime backend selection,
- multi-window routing,
- public backend handles,
- public native window access,
- GPU abstraction,
- GUI widgets or layout policy.

## Design Rules

- Public headers must continue to expose only portable C types and opaque handles.
- X11/XKB/XImage/Visual/GC/Atom types remain backend-private.
- Backend-neutral private headers must not include X11 or Wayland headers.
- Public behavior must remain unchanged while the backend boundary is introduced.
- Later backend work should route public entry points through the internal table
  before adding a second backend.

## Current State

`src/wpl_backend_internal.h` defines the private `WplBackendVTable`.
`backends/linux_x11/wpl_linux_x11_backend.c` owns the current default backend
selection and the public dispatch wrappers for window lifecycle, cursor,
frame-boundary, window dimension, frame-delta, and draw-list submission APIs.

`backends/linux_x11/wpl_linux_x11_window.c` still contains the X11 window
implementation, but its public symbol names are compiled as `wpl_linux_x11_*`
backend-private symbols before they are registered in the backend table.

`backends/linux_x11/wpl_linux_x11_renderer.c` still contains the X11 software
renderer implementation, but `wpl_submit_draw_list` is compiled as
`wpl_linux_x11_submit_draw_list` before it is registered in the backend table.

Presentation remains backend-private through `wpl_linux_x11_present_frame`, which
is consumed by the X11 frame-end implementation.  No runtime backend selection or
second backend exists yet.
