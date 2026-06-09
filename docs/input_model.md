# WPL Input Model

WPL exposes input as frame-stable `WplInputState` snapshots. Applications should
consume the snapshot returned by `wpl_get_input` after `wpl_begin_frame` and
`wpl_pump_events`.

## Snapshot Rules

- `button_down` and `key_down` describe current held state.
- `button_pressed` and `key_pressed` are transient one-frame transitions.
- `button_released` and `key_released` are transient one-frame transitions.
- Mouse position is reported in framebuffer pixel coordinates.
- Mouse delta and wheel delta are reset at the start of each frame.

## Focus Loss

When focus is lost, the backend clears held key and mouse-button state to avoid
stuck inputs.

## Auto-Repeat Validation

The Linux X11 backend attempts to enable XKB detectable auto-repeat. This should
avoid synthetic release/press pairs while a key is held.

Manual release validation is tracked in `docs/validation.md`:

- Hold `A` in `examples/01_input_snapshot`.
- Verify one press transition while held.
- Verify no release transition until physical release.

The fallback repeat-release suppression path requires an environment where XKB
detectable auto-repeat is unavailable or disabled. This remains a documented
validation gap unless explicitly validated or deferred for v0.1.
