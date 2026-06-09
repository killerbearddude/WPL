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

## Deferred

Live recorder/player state, replay file save/load orchestration, replay UI,
version migration, compression, and editor-command replay are deferred to later
focused patches.
