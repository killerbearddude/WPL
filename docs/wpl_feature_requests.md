# WPL Feature Request Register: WNG-Driven Requests

## Purpose

This register records post-v0.1 WPL enhancement requests raised by WNG needs.
It is a planning document only. It does not activate implementation work by
itself, and it does not transfer graph-editor responsibilities into WPL.

The goal is to keep WPL's platform-layer roadmap explicit, small, and driven by
concrete downstream pressure while preserving the WPL/WNG boundary.

## Boundary Rule

WPL owns low-level platform/rendering/text/file/input/replay infrastructure.

WNG owns graph data, nodes, ports, links, graph mutation, graph validation,
graph serialization DTOs, graph-specific editor state, graph hit testing,
selection, interaction rules, and graph rendering policy.

A WPL feature request becomes active only when a WNG milestone needs it and the
documented WNG fallback is becoming harmful.

## Current WPL v0.1 Compatibility Baseline

WPL v0.1.0 provides:

- Linux-native C11 platform layer services.
- X11/Xlib window lifecycle and event pump.
- Frame-stable input snapshots.
- Monotonic timing.
- Fixed-capacity draw command buffers.
- Software rendering with basic 2D primitives.
- ASCII bitmap text rendering.
- Canvas math for screen/canvas conversion, pan, and zoom.
- Append-only debug overlay draw-command generation.
- Binary whole-file I/O.
- Replay v1 recording/playback for input snapshots.
- Public logging callback API.

WPL v0.1.0 deliberately does not provide graph data structures, node/port/link
logic, widgets, layout, editor command systems, or graph serialization.

## Request Priority Definitions

- **P0**: Blocks an immediate WNG milestone or forces unsafe/duplicated WNG
  infrastructure.
- **P1**: Strongly improves WNG implementation quality or removes likely WNG
  duplication, but has an acceptable short-term fallback.
- **P2**: Useful for polish or longer-term maintainability. Not required for the
  first WNG integrations.
- **P3**: Future investigation or convenience; not part of near-term WPL scope.

Blocker status values:

- **Blocking**: WNG should not proceed without the WPL support.
- **Near-term**: WNG can proceed temporarily, but the fallback should not remain
  permanent.
- **Deferred**: Acceptable to postpone until a later WNG milestone.
- **Non-blocking**: Helpful, but not required for planned WNG work.

## Request Summary

| ID | Request | Priority | Blocker status |
|---|---|---:|---|
| WPL-FR-WNG-001 | Public ASCII text measurement | P0 | Implemented after v0.1 |
| WPL-FR-WNG-002 | Public text command byte limit | P0 | Implemented after v0.1 |
| WPL-FR-WNG-003 | Polyline or cubic Bezier primitive | P1 | Partially implemented after v0.1 |
| WPL-FR-WNG-004 | Dashed line primitive | P1 | Implemented after v0.1 |
| WPL-FR-WNG-005 | Cursor shape API | P1 | Deferred |
| WPL-FR-WNG-006 | Clip rectangle stack | P1 | Near-term |
| WPL-FR-WNG-007 | Clipboard text API | P2 | Deferred |
| WPL-FR-WNG-008 | User-supplied debug overlay lines | P1 | Deferred |
| WPL-FR-WNG-009 | Panel and rounded rectangle primitives | P1 | Deferred |
| WPL-FR-WNG-010 | Expanded key and modifier support | P2 | Deferred |
| WPL-FR-WNG-011 | Atomic file write helper | P0 | Near-term |
| WPL-FR-WNG-012 | Native file dialog API | P3 | Non-blocking |
| WPL-FR-WNG-013 | High-DPI policy | P2 | Deferred |
| WPL-FR-WNG-014 | Anti-aliased primitive rendering | P2 | Deferred |
| WPL-FR-WNG-015 | Extended text support | P3 | Non-blocking |

## Feature Requests

### WPL-FR-WNG-001 Public ASCII text measurement

- **Implementation status:** Implemented after v0.1 in Patch 022.
- **Priority:** P0
- **Blocker status:** Near-term
- **Need:** WNG needs deterministic sizing for labels, port names, diagnostics,
  and lightweight canvas annotations before issuing draw commands.
- **Requested API direction:** Public fixed-font ASCII measurement helpers, such
  as text width, line height, glyph advance, and multi-line bounds using the same
  bitmap font assumptions as WPL text rendering.
- **Required behavior:** Measurement must match the existing ASCII bitmap text
  renderer, be backend-independent, avoid dynamic allocation, and document byte
  handling for tabs, newlines, carriage returns, and unsupported bytes.
- **WNG fallback:** Duplicate WPL's text metrics in WNG using hard-coded glyph
  dimensions, with drift risk if WPL text rendering changes.

### WPL-FR-WNG-002 Public text command byte limit

- **Implementation status:** Implemented after v0.1 in Patch 022.
- **Priority:** P0
- **Blocker status:** Near-term
- **Need:** WNG needs to clamp or split debug/label text before appending draw
  commands without depending on private implementation limits.
- **Requested API direction:** Public constant for maximum bytes accepted by a
  single text draw command, for example `WPL_DRAW_TEXT_MAX_BYTES`.
- **Required behavior:** The public limit must match draw-list storage behavior,
  be documented as byte-oriented rather than Unicode character-oriented, and be
  test-covered.
- **WNG fallback:** Conservatively truncate to a smaller private WNG limit, which
  wastes available text capacity and may still drift from WPL.

### WPL-FR-WNG-003 Polyline or cubic Bezier primitive

- **Implementation status:** Partially implemented after v0.1 in Patch 024 as
  `wpl_draw_polyline`. Cubic Bezier support remains deferred.
- **Implementation note:** The initial implementation expands a polyline into
  `point_count - 1` existing line commands. This preserves the fixed-capacity
  draw-list model and avoids variable-sized draw command payloads.
- **Priority:** P1
- **Blocker status:** Near-term
- **Need:** WNG graph links need stable line/curve rendering without turning WNG
  into a low-level rasterization layer.
- **Requested API direction:** Add either a polyline primitive or a cubic Bezier
  primitive that remains purely geometric and application-agnostic.
- **Required behavior:** The primitive must use existing draw-list ownership
  rules, clip safely to framebuffer bounds, avoid graph-specific semantics, and
  allow WNG to choose graph rendering policy.
- **WNG fallback:** Approximate links with multiple WPL line commands generated
  in WNG.

### WPL-FR-WNG-004 Dashed line primitive

- **Implementation status:** Implemented after v0.1 in Patch 025 as
  `wpl_draw_dashed_line`.
- **Implementation note:** The initial implementation expands each visible dash
  into an existing line command. This preserves the fixed-capacity draw-list
  model and avoids introducing a renderer-specific dashed-line command.
- **Priority:** P1
- **Blocker status:** Near-term
- **Need:** WNG needs visual distinctions for pending links, selection previews,
  invalid connections, and diagnostics.
- **Requested API direction:** Add a dashed line draw command or a dash option on
  line-like primitives.
- **Required behavior:** Dashing must be geometry-level only. It must not encode
  graph concepts such as selected, invalid, hovered, or preview.
- **WNG fallback:** Emit many short line segments from WNG, increasing command
  count and duplicating dash generation logic.

### WPL-FR-WNG-005 Cursor shape API

- **Priority:** P1
- **Blocker status:** Deferred
- **Need:** WNG needs standard cursor feedback for panning, link dragging,
  resizing affordances, and text-like interactions.
- **Requested API direction:** Public cursor-shape API with a small enum of
  platform-neutral shapes.
- **Required behavior:** WPL maps public cursor shapes to X11 cursors privately.
  Public headers must not expose X11 cursor handles or atoms.
- **WNG fallback:** Use the default cursor for all modes.

### WPL-FR-WNG-006 Clip rectangle stack

- **Priority:** P1
- **Blocker status:** Near-term
- **Need:** WNG needs to constrain canvas panels, inspectors, minimap regions,
  and debug overlays without implementing framebuffer clipping itself.
- **Requested API direction:** Add draw-list clip push/pop commands or an
  equivalent fixed-capacity clip stack.
- **Required behavior:** Clip behavior must remain rectangular, deterministic,
  bounded, and independent of widgets or layout.
- **WNG fallback:** Manually cull some geometry in WNG and accept overdraw for
  primitives that are difficult to clip.

### WPL-FR-WNG-007 Clipboard text API

- **Priority:** P2
- **Blocker status:** Deferred
- **Need:** WNG eventually needs copy/paste for node names, graph snippets, and
  text fields owned by WNG.
- **Requested API direction:** Minimal UTF-8 text clipboard read/write API using
  private X11 clipboard handling.
- **Required behavior:** WPL only transports text. WNG owns serialization,
  graph object interpretation, command handling, and paste policy.
- **WNG fallback:** No clipboard support or application-specific temporary file
  interchange.

### WPL-FR-WNG-008 User-supplied debug overlay lines

- **Priority:** P1
- **Blocker status:** Deferred
- **Need:** WNG needs to display application diagnostics near WPL's existing
  overlay without implementing a separate overlay path.
- **Requested API direction:** Extend debug overlay input to accept bounded
  caller-provided text lines.
- **Required behavior:** WPL appends text draw commands only. It must not parse,
  categorize, or own WNG diagnostics.
- **WNG fallback:** Emit separate fixed-position text commands from WNG.

### WPL-FR-WNG-009 Panel and rounded rectangle primitives

- **Priority:** P1
- **Blocker status:** Deferred
- **Need:** WNG needs simple visual containers and node bodies without creating a
  widget system in WPL.
- **Requested API direction:** Add panel-style rectangle helpers and/or rounded
  rectangle primitive support.
- **Required behavior:** These must remain visual primitives only. WPL must not
  own layout, hit testing, node identity, selection, or editor behavior.
- **WNG fallback:** Use rectangular primitives with square corners and WNG-side
  visual conventions.

### WPL-FR-WNG-010 Expanded key and modifier support

- **Priority:** P2
- **Blocker status:** Deferred
- **Need:** WNG may need additional keys for editor shortcuts, navigation,
  search, and command palettes.
- **Requested API direction:** Expand public key enum and modifier reporting in a
  controlled, test-covered way.
- **Required behavior:** Input remains a frame-stable snapshot. WPL must not
  implement shortcut dispatch or editor commands.
- **WNG fallback:** Limit initial WNG shortcuts to the v0.1 key set.

### WPL-FR-WNG-011 Atomic file write helper

- **Priority:** P0
- **Blocker status:** Near-term
- **Implementation status:** Implemented after v0.1 in Patch 023.
- **Need:** WNG needs safer graph-save behavior than direct truncate/write for
  user-authored files.
- **Requested API direction:** Add an atomic write helper that writes to a temp
  file and renames into place on success.
- **Required behavior:** WPL owns file-system mechanics only. WNG owns graph data
  serialization, versioning, DTO schema, and save policy.
- **WNG fallback:** Implement temporary-file write/rename logic in WNG, creating
  duplicated platform behavior.

### WPL-FR-WNG-012 Native file dialog API

- **Priority:** P3
- **Blocker status:** Non-blocking
- **Need:** WNG may eventually need native open/save dialogs for graph files.
- **Requested API direction:** Investigate whether a Linux-native file dialog API
  belongs in WPL without adding toolkit dependencies.
- **Required behavior:** Any future API must avoid pulling WPL into a GUI toolkit
  role. Dialog policy and file semantics stay in WNG.
- **WNG fallback:** Use CLI paths, project-relative paths, or a WNG-owned file
  picker.

### WPL-FR-WNG-013 High-DPI policy

- **Priority:** P2
- **Blocker status:** Deferred
- **Need:** WNG canvas rendering will eventually need explicit pixel/scaling
  behavior across displays.
- **Requested API direction:** Define WPL's high-DPI policy and expose necessary
  scale data if required.
- **Required behavior:** The policy must be Linux/X11-focused, simple, and not
  coupled to layout or widget systems.
- **WNG fallback:** Treat one screen unit as one framebuffer pixel for early WNG
  milestones.

### WPL-FR-WNG-014 Anti-aliased primitive rendering

- **Priority:** P2
- **Blocker status:** Deferred
- **Need:** WNG graph links and rounded shapes would benefit visually from
  smoother primitive rendering.
- **Requested API direction:** Add anti-aliased rasterization for selected
  primitives or a renderer-quality policy.
- **Required behavior:** Anti-aliasing must not introduce GPU abstractions,
  scene graphs, retained objects, or graph semantics.
- **WNG fallback:** Accept aliased primitives for v0.1-era WNG prototypes.

### WPL-FR-WNG-015 Extended text support

- **Priority:** P3
- **Blocker status:** Non-blocking
- **Need:** WNG may eventually need richer labels, Unicode text, or external font
  support.
- **Requested API direction:** Investigate extended text support after WNG has
  proven concrete requirements.
- **Required behavior:** WPL must avoid becoming a full text layout engine.
  Unicode shaping, rich text policy, and editor text widgets are out of scope
  unless explicitly re-scoped later.
- **WNG fallback:** Restrict early WNG labels and diagnostics to ASCII.

## Explicit Non-Requests

The following are not WPL feature requests:

- Graph data model.
- Node, port, or link objects.
- Graph mutation or validation rules.
- Graph serialization DTOs.
- Graph-specific hit testing.
- Selection rules.
- Editor command dispatch.
- Undo/redo.
- Widget hierarchy.
- Layout engine.
- Docking/panels as UI objects.
- Scene graph.
- ECS.
- GPU abstraction.
- SDL, Wayland, Windows, or macOS backend work.

## WNG Implementation Guidance

WNG should consume WPL as a low-level service layer:

- Use WPL input snapshots, but own WNG interaction state and command mapping.
- Use WPL draw commands, but own graph rendering policy and draw ordering.
- Use WPL canvas math, but own graph object coordinate systems and hit testing.
- Use WPL file helpers, but own graph serialization format and DTOs.
- Use WPL replay at the input/platform boundary only, not as graph-command
  replay.

WNG should prefer local fallbacks while feature pressure is low. A request should
be promoted to active WPL work only when repeated WNG code duplication or
correctness risk makes the fallback harmful.

## Acceptance Criteria

This register is complete for the first post-v0.1 planning pass when:

- All 15 WNG-driven WPL feature requests are listed.
- Each request includes priority, blocker status, need, requested API direction,
  required behavior, and WNG fallback.
- The WPL/WNG ownership boundary is explicit.
- Explicit non-requests are preserved.
- The roadmap links to this register.
- No implementation work is included in the patch that adds this document.
