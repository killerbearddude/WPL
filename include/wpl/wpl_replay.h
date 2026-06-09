#ifndef WPL_REPLAY_H
#define WPL_REPLAY_H

#include <stdbool.h>
#include <stdint.h>

#include "wpl_input.h"
#include "wpl_result.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WPL_REPLAY_MAGIC "WPLR"
#define WPL_REPLAY_VERSION 1u
#define WPL_REPLAY_VERSION_V1 WPL_REPLAY_VERSION
#define WPL_REPLAY_HEADER_SIZE_V1 32u
#define WPL_REPLAY_FRAME_SIZE_V1 192u
#define WPL_REPLAY_KEY_COUNT_V1 37u
#define WPL_REPLAY_MAX_FRAMES_V1 349525ull
#define WPL_REPLAY_VALID_FLAGS_V1 0u

/* Opaque replay handles for input-snapshot recording and playback. */
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

#ifdef __cplusplus
}
#endif

#endif /* WPL_REPLAY_H */
