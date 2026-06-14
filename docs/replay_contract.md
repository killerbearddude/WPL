# WPL Replay Contract

This document records the Phase 9 replay contract for WPL. It is a contract
document only. It does not change runtime behavior, public API, tests, or
application semantics.

WPL replay operates at the platform/input snapshot boundary. It records and
plays back `WplInputState` snapshots plus frame delta values. It does not record
raw backend events, X11 event streams, editor commands, UI behavior, graph state,
application data, renderer output, file contents, or timing policy beyond the
stored frame deltas.

## Current Replay Shape

The public replay API exposes opaque recorder/player handles:

```c
#define WPL_REPLAY_MAGIC "WPLR"
#define WPL_REPLAY_VERSION 1u
#define WPL_REPLAY_VERSION_V1 WPL_REPLAY_VERSION
#define WPL_REPLAY_HEADER_SIZE_V1 32u
#define WPL_REPLAY_FRAME_SIZE_V1 192u
#define WPL_REPLAY_KEY_COUNT_V1 37u
#define WPL_REPLAY_MAX_FRAMES_V1 349525ull
#define WPL_REPLAY_VALID_FLAGS_V1 0u

typedef struct WplReplayRecorder WplReplayRecorder;
typedef struct WplReplayPlayer WplReplayPlayer;

WplResult wpl_replay_recorder_create(WplReplayRecorder** out_recorder);
void wpl_replay_recorder_destroy(WplReplayRecorder* recorder);
WplResult wpl_replay_recorder_begin(WplReplayRecorder* recorder);
WplResult wpl_replay_recorder_record_frame(WplReplayRecorder* recorder,
                                           const WplInputState* input,
                                           float delta_time);
WplResult wpl_replay_recorder_save(WplReplayRecorder* recorder,
                                   const char* path);

WplResult wpl_replay_player_create(WplReplayPlayer** out_player);
void wpl_replay_player_destroy(WplReplayPlayer* player);
WplResult wpl_replay_player_load(WplReplayPlayer* player, const char* path);
WplResult wpl_replay_player_next(WplReplayPlayer* player,
                                 WplInputState* out_input,
                                 float* out_delta_time,
                                 bool* out_has_frame);
```

Replay handles are heap-owned WPL objects. The public header exposes no replay
binary structs, file descriptors, X11 types, backend event objects, renderer
objects, or application/editor state.

## Recorder Contract

A recorder is created with `wpl_replay_recorder_create` and released with
`wpl_replay_recorder_destroy`.

Recorder behavior:

- `wpl_replay_recorder_create` requires a non-null output pointer,
- successful creation stores a non-null recorder handle,
- destroy accepts null,
- `wpl_replay_recorder_begin` resets the recorder to an empty recording session,
- frames cannot be recorded before `begin`,
- `record_frame` requires a non-null recorder and input snapshot,
- `delta_time` must be finite and non-negative,
- recorded input floats must be finite,
- invalid frame input does not append a frame,
- frame indices start at zero after `begin`,
- accumulated frame time is stored in microseconds,
- deltas that cannot be represented by the v1 frame field are rejected,
- frame count is capped by `WPL_REPLAY_MAX_FRAMES_V1`.

The recorder copies the caller-provided `WplInputState` into replay-owned frame
storage. Callers retain ownership of the original input snapshot.

## Save Contract

`wpl_replay_recorder_save` writes the active recording session to a replay v1
file.

Save behavior:

- recorder, path, and non-empty path are required,
- save before `begin` is invalid,
- zero-frame recordings are valid and contain only the v1 header,
- saved files use the public v1 header/frame sizes,
- saved files use little-endian integer and float field encoding,
- save serializes only replay frame data and input snapshots,
- save uses the file I/O atomic whole-file write helper,
- save does not change application state, input state, timing state, or renderer state.

Replay save does not create directories, choose project paths, manage file naming,
compress data, encrypt data, or implement application persistence policy.

## Player Contract

A player is created with `wpl_replay_player_create` and released with
`wpl_replay_player_destroy`.

Player behavior:

- `wpl_replay_player_create` requires a non-null output pointer,
- destroy accepts null,
- `wpl_replay_player_load` requires a non-null player and non-empty path,
- load reads a whole replay file through file I/O,
- load rejects invalid magic, unsupported version, wrong header/frame sizes,
  invalid flags, nonzero reserved header data, invalid frame data, invalid frame
  ordering, invalid accumulated times, short files, and trailing bytes,
- failed load does not replace a previously loaded valid replay,
- failed load does not rewind the existing playback cursor,
- successful load resets playback cursor to the first frame,
- zero-frame replays are valid and immediately report no frame.

`wpl_replay_player_next` requires non-null output pointers and a loaded player.
It resets output values before validation returns. When a frame is available it
copies the recorded `WplInputState`, reports the stored delta time in seconds,
sets `out_has_frame` to true, and advances the cursor. At end of replay it
returns `WPL_RESULT_OK`, clears outputs, and sets `out_has_frame` to false.

Playback does not inject OS events, mutate platform input state, move the cursor,
press keys, submit draw commands, or drive an application loop. Higher-level code
chooses how to consume the returned snapshots.

## Binary Format Contract

Replay v1 is a fixed-size binary format:

- magic: `WPLR`,
- version: `1`,
- header size: `32` bytes,
- frame size: `192` bytes,
- key count: `37`,
- valid flags: `0`,
- max frames: `349525`, constrained by the current file-size cap.

The v1 frame contains:

- frame index,
- accumulated time in microseconds,
- frame delta in microseconds,
- mouse position,
- mouse delta,
- mouse wheel delta,
- mouse button down/pressed/released states,
- keyboard down/pressed/released states,
- modifier state,
- reserved bytes that must be zero.

Boolean fields are encoded as `0` or `1`. Other boolean byte values are parse
errors. Floating-point fields must be finite. Reserved bytes must be zero.

Internal binary structs live in private implementation headers. Public headers
expose only constants, opaque handles, C-compatible function declarations, and
portable input/result types.

## Boundary and Non-goals

Replay remains a platform/input debugging and regression tool. It deliberately
does not provide:

- raw X11 event recording,
- event synthesis,
- application state capture,
- graph serialization,
- editor command history,
- undo/redo,
- scene snapshots,
- renderer capture,
- frame pacing,
- deterministic scheduler,
- wall-clock recording,
- asset serialization,
- project file format policy,
- compression,
- encryption,
- cross-version migration,
- backend handles.

## Validation Requirements

Replay changes should add tests or documented validation for:

- recorder create/destroy behavior,
- recorder begin/reset behavior,
- record-before-begin rejection,
- invalid argument handling,
- invalid `player_next` output reset behavior,
- negative/nonfinite delta rejection,
- zero-delta round trip behavior,
- nonfinite input rejection without state mutation,
- zero-frame save/load behavior,
- one-frame save/load behavior,
- multi-frame round trip behavior,
- frame index sequencing,
- accumulated time sequencing,
- malformed header rejection,
- unsupported version rejection,
- malformed boolean rejection,
- nonzero reserved byte rejection,
- nonfinite replay float rejection,
- truncated file rejection,
- trailing byte rejection,
- failed load preserving previous valid replay,
- failed load preserving playback cursor position,
- replay format size validation,
- public-header/backend leak checks.

Current focused replay validation target:

```sh
ctest --test-dir build --output-on-failure -R 'wpl_test_replay(_format|_edges)?$'
```

The focused replay targets cover v1 header/frame encoding, malformed input
rejection, recorder/player validation, invalid next-output reset, zero-frame
save/load, zero-delta round trips, multi-frame round trips, trailing-byte
rejection, failed-load preservation, and cursor-preservation behavior.

Focused replay validation does not replace full CTest, sanitizer validation,
backend-leak checks, frame-pacing boundary checks, focused draw, renderer, canvas,
debug overlay, file I/O validation, or Xvfb smoke validation.

Phase 9a documented this contract. Phase 9b added replay edge coverage. Phase 9c
syncs this validation documentation and closes the Phase 9 replay hardening pass.
