# Linux Backend Interface

This document records the Phase 1 private backend interface direction for WPL.
The interface is internal implementation infrastructure only; it is not public
API and is not installed for applications.

## Objective

Introduce a narrow backend boundary so the current X11 implementation can be
wrapped before later Linux backend work.  The first slice is behavior-preserving:
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
`backends/linux_x11/wpl_linux_x11_backend.c` registers the existing X11 entry
points in that table.

This keeps the first refactor small and gives later patches a concrete seam for
moving dispatch, renderer presentation, and multi-window routing without changing
the public API.
