# WPL Software Renderer Contract

This document records the Phase 5 software renderer contract for WPL. It is a
contract document only. It does not change runtime behavior, public API, or draw
semantics.

WPL's renderer is a small CPU software renderer used to consume WPL draw command
lists and present pixels through the Linux/X11 backend. It is not a GPU
abstraction, scene graph, widget renderer, layout engine, retained UI system, or
material/shader framework.

## Current Renderer Shape

The public API does not expose a renderer object. Applications build a
`WplDrawList` and submit it to a window:

```c
WplResult wpl_submit_draw_list(WplWindow* window, const WplDrawList* list);
WplResult wpl_end_frame(WplWindow* window);
```

The concrete software renderer state is backend-private and currently lives behind
`WplWindow` in the Linux/X11 backend implementation. Public headers must not expose
framebuffer pointers, XImage ownership, X11 handles, renderer command storage, or
backend-native visual state.

## Ownership and Lifetime

Renderer resources are owned by the platform/backend window implementation.
Application code owns draw lists before and after submission. Draw submission reads
commands from the list; it does not take ownership of the list and does not make
the list immutable.

The backend owns:

- the CPU framebuffer allocation,
- XImage presentation storage,
- active renderer clip state,
- renderer clip stack storage,
- conversion from draw commands to pixels,
- presentation to the X11 window.

Renderer resources are destroyed with the window/backend resources. Application
code must not retain or access renderer-private memory.

## Render Target Contract

Render targets are sized from the current window dimensions.

Current behavior:

- zero or negative window dimensions are treated as a no-op render/present target,
- framebuffer and XImage resources are resized when window dimensions change,
- existing renderer resources may be destroyed and replaced during resize,
- render target allocation failures return an error result,
- renderer memory remains backend-private.

The software renderer currently targets 32-bit packed pixels internally. Public API
consumers must not depend on that representation.

## Color Contract

The draw-list layer accepts finite color values, including finite values outside
`[0, 1]`.

The renderer converts color channels to 8-bit output channels when rasterizing.
Current behavior clamps channels at rasterization time:

- values less than or equal to `0.0f` map to `0`,
- values greater than or equal to `1.0f` map to `255`,
- values between `0.0f` and `1.0f` map by rounding to the nearest 8-bit value.

The renderer treats output pixels as opaque after rasterization. Source alpha is
used for simple source-over blending against the destination pixel. Alpha values at
or below zero leave the destination unchanged; alpha values at or above one replace
the destination color.

This is a simple software-rendering rule, not a color-management system. WPL does
not currently define linear/sRGB conversion, premultiplied-alpha policy, ICC
profiles, HDR, wide-gamut output, or gamma-correct blending.

## Geometry and Rasterization Contract

Draw-list construction validates finite geometry before commands reach the
renderer. The renderer still defensively clamps raster bounds to the active clip
and framebuffer dimensions.

Current renderer behavior is intentionally simple:

- rectangles fill integer pixel bounds derived from float rectangles,
- rounded rectangles are filled with simple CPU tests against corner radii,
- rectangle outlines are rendered as filled border regions,
- lines are rendered by testing pixel centers against segment distance,
- circles are rendered by testing pixel centers against radius distance,
- text is rendered with the built-in ASCII bitmap font,
- primitives are not anti-aliased.

WPL does not currently define subpixel coverage, anti-aliasing, path filling,
joins, caps, gradients, patterns, image sampling, sprite atlases, or vector text
rendering.

## Clip Contract

Clip commands are interpreted by the renderer as a stack of rectangular clips.

Current behavior:

- each draw submission resets renderer clip state before processing commands,
- push-clip intersects the requested clip rectangle with the current active clip,
- pop-clip restores the previous clip or full-window clip,
- empty clips are valid and suppress raster output while active,
- renderer clip storage is backend-private and can grow to match submitted command
  requirements.

The draw-list layer owns command-level clip-depth validation. The renderer owns
active raster clipping during submission.

## Text Rendering Boundary

The renderer draws text with the built-in ASCII bitmap font used by WPL examples
and debug overlays.

Text rendering does not provide:

- Unicode shaping,
- bidirectional layout,
- font fallback,
- kerning,
- selection,
- cursor movement,
- IME composition,
- clipboard integration,
- rich text,
- editor behavior.

Text input and text editing remain outside the software-renderer boundary.

## Presentation Boundary

`wpl_submit_draw_list` converts draw commands into the backend framebuffer.
`wpl_end_frame` presents the current framebuffer through the backend presenter.

Current Linux/X11 presentation uses XImage/XPutImage privately. Public headers must
not expose XImage pointers, Visual data, Display handles, Window handles, GC
handles, or X11 error-handling state.

X11 protocol errors remain part of the backend boundary. WPL does not currently
install a custom global X11 error handler for presentation errors.

## Non-goals

The software renderer deliberately does not provide:

- GPU abstraction,
- OpenGL/Vulkan/Metal/DirectX integration,
- shader or material systems,
- retained render objects,
- scene graphs,
- z-index or layer policy,
- batching API,
- texture/image API,
- font loading,
- rich text layout,
- widgets,
- layout,
- node graph semantics,
- editor command behavior,
- animation systems,
- color-management policy.

## Validation Requirements

Renderer changes should add tests or documented validation for:

- null argument handling at submit/present boundaries,
- zero-sized window no-op behavior,
- caller-owned draw-list lifetime after submit,
- render target resize behavior,
- framebuffer clear and primitive raster behavior,
- clip reset and clip-stack behavior,
- finite color clamping/blending behavior,
- text rendering bounds behavior,
- sanitizer-clean renderer code,
- backend-leak checks for public headers,
- Xvfb smoke coverage for the X11 presentation path.

Current focused renderer validation targets:

```sh
ctest --test-dir build --output-on-failure -R 'wpl_test_renderer_(pixels|targets)$'
```

The focused targets cover framebuffer/color/clip pixel behavior and render-target
resize/presentation boundaries. They do not replace full CTest, sanitizer
validation, backend-leak checks, or Xvfb smoke validation.

Phase 5a documented this contract. Phase 5b added renderer framebuffer/color/clip
pixel tests. Phase 5c added renderer target resize and presentation-boundary
tests. Phase 5d syncs this validation documentation and closes the Phase 5
software-renderer hardening pass.
