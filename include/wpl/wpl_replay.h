#ifndef WPL_REPLAY_H
#define WPL_REPLAY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WPL_REPLAY_VERSION_V1 1u
#define WPL_REPLAY_HEADER_SIZE_V1 32u
#define WPL_REPLAY_FRAME_SIZE_V1 192u
#define WPL_REPLAY_KEY_COUNT_V1 37u
#define WPL_REPLAY_MAX_FRAMES_V1 349525ull
#define WPL_REPLAY_VALID_FLAGS_V1 0u

/* Opaque replay handles. Public replay operations are deferred. */
typedef struct WplReplayRecorder WplReplayRecorder;
typedef struct WplReplayPlayer WplReplayPlayer;

#ifdef __cplusplus
}
#endif

#endif /* WPL_REPLAY_H */
