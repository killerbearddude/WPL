#ifndef WPL_REPLAY_INTERNAL_H
#define WPL_REPLAY_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

#include "wpl/wpl_input.h"
#include "wpl/wpl_replay.h"
#include "wpl/wpl_result.h"

typedef struct WplReplayHeaderV1 {
  uint64_t frame_count;
} WplReplayHeaderV1;

typedef struct WplReplayFrameV1 {
  uint64_t frame_index;
  uint64_t time_microseconds;
  uint32_t delta_microseconds;
  WplInputState input;
} WplReplayFrameV1;

WplResult wpl_replay_encode_header_v1(
  uint64_t frame_count,
  uint8_t out_header[WPL_REPLAY_HEADER_SIZE_V1]);

WplResult wpl_replay_decode_header_v1(const uint8_t* data,
                                      size_t size,
                                      WplReplayHeaderV1* out_header);

WplResult wpl_replay_encode_frame_v1(
  const WplReplayFrameV1* frame,
  uint8_t out_frame[WPL_REPLAY_FRAME_SIZE_V1]);

WplResult wpl_replay_decode_frame_v1(const uint8_t* data,
                                     size_t size,
                                     WplReplayFrameV1* out_frame);

WplResult wpl_replay_validate_file_size_v1(uint64_t frame_count,
                                           size_t actual_size);

#endif /* WPL_REPLAY_INTERNAL_H */
