# Replay Format

WPL replay v1 is a fixed binary schema for platform/input-boundary debugging and
regression data.

## Scope

- Replay files do not serialize raw C structs.
- Integer fields are little-endian.
- Float fields are IEEE-754 binary32 stored little-endian.
- Boolean fields are serialized as `uint8_t` values `0` or `1`.
- The header is exactly 32 bytes.
- Each frame is exactly 192 bytes.
- `WPL_KEY_COUNT` must match `WPL_REPLAY_KEY_COUNT_V1`.
- `WPL_MOUSE_BUTTON_COUNT` must remain 3 for replay v1.
- Frame count is capped by `WPL_REPLAY_MAX_FRAMES_V1`.
- Replay file size remains under the 64 MiB v0.1 file-size limit.
- v1 flags must be zero.
- Reserved fields must be zero.
- Malformed values produce parse errors.

## Header v1

The replay header stores the magic value, version, header size, frame size,
frame count, flags, and reserved field at fixed byte offsets. All integer fields
are encoded little-endian.

## Frame v1

Each frame stores frame index, accumulated time in microseconds, delta time in
microseconds, mouse state, key state, modifier state, and reserved bytes. Public
C structs are decoded into and encoded from field values explicitly; `sizeof` is
not part of the file format contract.

## Recorder / Player

The replay recorder stores `WplInputState` snapshots and frame delta timing at
the platform/input boundary.  Delta time is converted to microseconds and an
accumulated replay timestamp is stored per frame.

Saving a recorder encodes the v1 header and frames explicitly, then writes the
result through WPL file I/O.  Loading a player reads through WPL file I/O,
validates the full file, decodes every frame, and only replaces previously
loaded replay state after a complete successful decode.

Playback returns input snapshots and delta time in order.  After the final frame,
`wpl_replay_player_next` returns success with `out_has_frame = false`, zeroed
input, and zero delta time.

Replay does not record editor commands, application state, simulation state, X11
event streams, or raw C structs.  Replay UI controls, debug overlay replay state,
version migration, compression, and editor-command replay remain out of scope.

## Example Usage

`examples/05_input_replay` demonstrates the public replay APIs. Record mode
captures frame-stable `WplInputState` snapshots and frame deltas into a replay
file. Playback mode loads that file through `WplReplayPlayer` and visualizes the
replayed snapshots with existing draw commands.

The example does not replay raw X11 events, editor commands, application state,
widgets, layouts, or node graph behavior.

## Release Review Note

Replay v1 remains a platform/input-boundary format. Recorder/player APIs save and
load through the explicit v1 encoder/decoder and WPL file I/O. Raw public C
struct serialization remains prohibited.
