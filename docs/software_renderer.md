# Software Renderer

WPL v0.1 uses a simple CPU software renderer owned by the Linux X11 backend.
Draw commands are recorded into fixed-capacity draw lists and executed into a
WPL-owned framebuffer before presentation with XImage/XPutImage.

## Supported Commands

- Clear
- Filled rectangle
- Rectangle outline
- Thick line
- Filled circle
- Printable ASCII bitmap text

## Pixel Policy

- Framebuffer pixels are stored as `0xAARRGGBB`.
- Stored framebuffer alpha is always opaque after rendering.
- Source alpha controls RGB blending.
- Destination alpha is ignored.
- The XImage buffer is owned separately from the WPL framebuffer.

## Text Policy

Text uses an embedded backend-private monospace bitmap font intended for debug
overlays, labels, and prototype UI text. The font data is project-owned/internal
source data in the repository; no third-party font file is imported.

Supported text scope:

- Printable ASCII bytes 32 through 126.
- Unsupported bytes render as `?`.
- Newline advances by a fixed line height.
- Tab advances by four spaces.
- Public text metrics use the same glyph advance and line-height constants as
  the renderer.
- `WPL_DRAW_TEXT_MAX_BYTES` documents the byte limit for one text draw command;
  it is not a Unicode character count.

Deferred text scope:

- Unicode decoding and shaping.
- Kerning or proportional fonts.
- Font loading.
- FreeType, fontconfig, HarfBuzz, Xft, or X11 core font rendering.
- Rich text, editable text, selection, or layout.

## X11 Visual Policy

The v0.1 X11 presenter copies internal `0xAARRGGBB` pixels directly into an
`XImage` only for common `TrueColor` visuals with RGB masks
`0x00ff0000`, `0x0000ff00`, and `0x000000ff`. Other visuals return
`WPL_RESULT_UNSUPPORTED` rather than presenting with incorrect channel order.

## Deferred Renderer Scope

- Public clipping API.
- Anti-aliasing.
- Dashed lines.
- Rounded rectangles.
- Bezier curves.
- Gradients.
- Textures.
- GPU acceleration.
