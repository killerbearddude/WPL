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

Deferred text scope:

- Unicode decoding and shaping.
- Kerning or proportional fonts.
- Font loading.
- FreeType, fontconfig, HarfBuzz, Xft, or X11 core font rendering.
- Rich text, editable text, selection, or layout.

## Deferred Renderer Scope

- Public clipping API.
- Anti-aliasing.
- Dashed lines.
- Rounded rectangles.
- Bezier curves.
- Gradients.
- Textures.
- GPU acceleration.
