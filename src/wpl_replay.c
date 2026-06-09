/* wpl_replay.c - Replay v1 fixed binary format helpers. */

#include "wpl_replay_internal.h"

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "wpl/wpl_base.h"

_Static_assert(WPL_KEY_COUNT == WPL_REPLAY_KEY_COUNT_V1,
               "Replay v1 key count must match WPL v0.1 key enum");
_Static_assert(WPL_MOUSE_BUTTON_COUNT == 3,
               "Replay v1 mouse button count must remain 3");

#define WPL_REPLAY_HEADER_OFFSET_MAGIC 0u
#define WPL_REPLAY_HEADER_OFFSET_VERSION 4u
#define WPL_REPLAY_HEADER_OFFSET_HEADER_SIZE 8u
#define WPL_REPLAY_HEADER_OFFSET_FRAME_SIZE 12u
#define WPL_REPLAY_HEADER_OFFSET_FRAME_COUNT 16u
#define WPL_REPLAY_HEADER_OFFSET_FLAGS 24u
#define WPL_REPLAY_HEADER_OFFSET_RESERVED 28u

#define WPL_REPLAY_FRAME_OFFSET_FRAME_INDEX 0u
#define WPL_REPLAY_FRAME_OFFSET_TIME_US 8u
#define WPL_REPLAY_FRAME_OFFSET_DELTA_US 16u
#define WPL_REPLAY_FRAME_OFFSET_MOUSE_X 20u
#define WPL_REPLAY_FRAME_OFFSET_MOUSE_Y 24u
#define WPL_REPLAY_FRAME_OFFSET_MOUSE_DX 28u
#define WPL_REPLAY_FRAME_OFFSET_MOUSE_DY 32u
#define WPL_REPLAY_FRAME_OFFSET_WHEEL 36u
#define WPL_REPLAY_FRAME_OFFSET_MOUSE_DOWN 40u
#define WPL_REPLAY_FRAME_OFFSET_MOUSE_PRESSED 43u
#define WPL_REPLAY_FRAME_OFFSET_MOUSE_RELEASED 46u
#define WPL_REPLAY_FRAME_OFFSET_KEY_DOWN 49u
#define WPL_REPLAY_FRAME_OFFSET_KEY_PRESSED 86u
#define WPL_REPLAY_FRAME_OFFSET_KEY_RELEASED 123u
#define WPL_REPLAY_FRAME_OFFSET_SHIFT 160u
#define WPL_REPLAY_FRAME_OFFSET_CTRL 161u
#define WPL_REPLAY_FRAME_OFFSET_ALT 162u
#define WPL_REPLAY_FRAME_OFFSET_RESERVED 163u
#define WPL_REPLAY_FRAME_RESERVED_SIZE 29u

static void
wpl_replay_write_u32_le(uint8_t* dst, uint32_t value)
{
  dst[0] = (uint8_t)(value & 0xffu);
  dst[1] = (uint8_t)((value >> 8) & 0xffu);
  dst[2] = (uint8_t)((value >> 16) & 0xffu);
  dst[3] = (uint8_t)((value >> 24) & 0xffu);
}

static void
wpl_replay_write_u64_le(uint8_t* dst, uint64_t value)
{
  dst[0] = (uint8_t)(value & 0xffu);
  dst[1] = (uint8_t)((value >> 8) & 0xffu);
  dst[2] = (uint8_t)((value >> 16) & 0xffu);
  dst[3] = (uint8_t)((value >> 24) & 0xffu);
  dst[4] = (uint8_t)((value >> 32) & 0xffu);
  dst[5] = (uint8_t)((value >> 40) & 0xffu);
  dst[6] = (uint8_t)((value >> 48) & 0xffu);
  dst[7] = (uint8_t)((value >> 56) & 0xffu);
}

static uint32_t
wpl_replay_read_u32_le(const uint8_t* src)
{
  return ((uint32_t)src[0])
         | ((uint32_t)src[1] << 8)
         | ((uint32_t)src[2] << 16)
         | ((uint32_t)src[3] << 24);
}

static uint64_t
wpl_replay_read_u64_le(const uint8_t* src)
{
  return ((uint64_t)src[0])
         | ((uint64_t)src[1] << 8)
         | ((uint64_t)src[2] << 16)
         | ((uint64_t)src[3] << 24)
         | ((uint64_t)src[4] << 32)
         | ((uint64_t)src[5] << 40)
         | ((uint64_t)src[6] << 48)
         | ((uint64_t)src[7] << 56);
}

static void
wpl_replay_write_f32_le(uint8_t* dst, float value)
{
  uint32_t bits = 0u;

  memcpy(&bits, &value, sizeof(bits));
  wpl_replay_write_u32_le(dst, bits);
}

static float
wpl_replay_read_f32_le(const uint8_t* src)
{
  uint32_t bits = wpl_replay_read_u32_le(src);
  float value = 0.0f;

  memcpy(&value, &bits, sizeof(value));
  return value;
}

static uint8_t
wpl_replay_bool_to_u8(bool value)
{
  return value ? 1u : 0u;
}

static WplResult
wpl_replay_u8_to_bool(uint8_t value, bool* out_bool)
{
  if (out_bool == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (value == 0u) {
    *out_bool = false;
    return WPL_RESULT_OK;
  }

  if (value == 1u) {
    *out_bool = true;
    return WPL_RESULT_OK;
  }

  return WPL_RESULT_PARSE_ERROR;
}

static bool
wpl_replay_f32_is_finite(float value)
{
  return isfinite(value) != 0;
}

static bool
wpl_replay_input_floats_are_finite(const WplInputState* input)
{
  return wpl_replay_f32_is_finite(input->mouse.position.x)
         && wpl_replay_f32_is_finite(input->mouse.position.y)
         && wpl_replay_f32_is_finite(input->mouse.delta.x)
         && wpl_replay_f32_is_finite(input->mouse.delta.y)
         && wpl_replay_f32_is_finite(input->mouse.wheel_delta);
}

WplResult
wpl_replay_encode_header_v1(uint64_t frame_count,
                            uint8_t out_header[WPL_REPLAY_HEADER_SIZE_V1])
{
  if (out_header == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (frame_count > WPL_REPLAY_MAX_FRAMES_V1)
    return WPL_RESULT_UNSUPPORTED;

  memset(out_header, 0, WPL_REPLAY_HEADER_SIZE_V1);
  memcpy(&out_header[WPL_REPLAY_HEADER_OFFSET_MAGIC], WPL_REPLAY_MAGIC, 4u);
  wpl_replay_write_u32_le(&out_header[WPL_REPLAY_HEADER_OFFSET_VERSION],
                          WPL_REPLAY_VERSION);
  wpl_replay_write_u32_le(&out_header[WPL_REPLAY_HEADER_OFFSET_HEADER_SIZE],
                          WPL_REPLAY_HEADER_SIZE_V1);
  wpl_replay_write_u32_le(&out_header[WPL_REPLAY_HEADER_OFFSET_FRAME_SIZE],
                          WPL_REPLAY_FRAME_SIZE_V1);
  wpl_replay_write_u64_le(&out_header[WPL_REPLAY_HEADER_OFFSET_FRAME_COUNT],
                          frame_count);
  wpl_replay_write_u32_le(&out_header[WPL_REPLAY_HEADER_OFFSET_FLAGS],
                          WPL_REPLAY_VALID_FLAGS_V1);
  wpl_replay_write_u32_le(&out_header[WPL_REPLAY_HEADER_OFFSET_RESERVED], 0u);

  return WPL_RESULT_OK;
}

WplResult
wpl_replay_decode_header_v1(const uint8_t* data,
                            size_t size,
                            WplReplayHeaderV1* out_header)
{
  uint32_t version = 0u;
  uint32_t header_size = 0u;
  uint32_t frame_size = 0u;
  uint64_t frame_count = 0u;
  uint32_t flags = 0u;
  uint32_t reserved = 0u;

  if (data == NULL || out_header == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  out_header->frame_count = 0u;

  if (size < WPL_REPLAY_HEADER_SIZE_V1)
    return WPL_RESULT_PARSE_ERROR;

  if (memcmp(&data[WPL_REPLAY_HEADER_OFFSET_MAGIC], WPL_REPLAY_MAGIC, 4u) != 0)
    return WPL_RESULT_PARSE_ERROR;

  version = wpl_replay_read_u32_le(&data[WPL_REPLAY_HEADER_OFFSET_VERSION]);
  if (version != WPL_REPLAY_VERSION)
    return WPL_RESULT_UNSUPPORTED;

  header_size = wpl_replay_read_u32_le(
    &data[WPL_REPLAY_HEADER_OFFSET_HEADER_SIZE]);
  if (header_size < WPL_REPLAY_HEADER_SIZE_V1)
    return WPL_RESULT_PARSE_ERROR;

  frame_size = wpl_replay_read_u32_le(
    &data[WPL_REPLAY_HEADER_OFFSET_FRAME_SIZE]);
  if (frame_size != WPL_REPLAY_FRAME_SIZE_V1)
    return WPL_RESULT_PARSE_ERROR;

  frame_count = wpl_replay_read_u64_le(
    &data[WPL_REPLAY_HEADER_OFFSET_FRAME_COUNT]);
  if (frame_count > WPL_REPLAY_MAX_FRAMES_V1)
    return WPL_RESULT_PARSE_ERROR;

  flags = wpl_replay_read_u32_le(&data[WPL_REPLAY_HEADER_OFFSET_FLAGS]);
  if (flags != WPL_REPLAY_VALID_FLAGS_V1)
    return WPL_RESULT_PARSE_ERROR;

  reserved = wpl_replay_read_u32_le(
    &data[WPL_REPLAY_HEADER_OFFSET_RESERVED]);
  if (reserved != 0u)
    return WPL_RESULT_PARSE_ERROR;

  out_header->frame_count = frame_count;
  return WPL_RESULT_OK;
}

WplResult
wpl_replay_encode_frame_v1(const WplReplayFrameV1* frame,
                           uint8_t out_frame[WPL_REPLAY_FRAME_SIZE_V1])
{
  size_t i = 0u;

  if (frame == NULL || out_frame == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (!wpl_replay_input_floats_are_finite(&frame->input))
    return WPL_RESULT_INVALID_ARGUMENT;

  memset(out_frame, 0, WPL_REPLAY_FRAME_SIZE_V1);

  wpl_replay_write_u64_le(&out_frame[WPL_REPLAY_FRAME_OFFSET_FRAME_INDEX],
                          frame->frame_index);
  wpl_replay_write_u64_le(&out_frame[WPL_REPLAY_FRAME_OFFSET_TIME_US],
                          frame->time_microseconds);
  wpl_replay_write_u32_le(&out_frame[WPL_REPLAY_FRAME_OFFSET_DELTA_US],
                          frame->delta_microseconds);
  wpl_replay_write_f32_le(&out_frame[WPL_REPLAY_FRAME_OFFSET_MOUSE_X],
                          frame->input.mouse.position.x);
  wpl_replay_write_f32_le(&out_frame[WPL_REPLAY_FRAME_OFFSET_MOUSE_Y],
                          frame->input.mouse.position.y);
  wpl_replay_write_f32_le(&out_frame[WPL_REPLAY_FRAME_OFFSET_MOUSE_DX],
                          frame->input.mouse.delta.x);
  wpl_replay_write_f32_le(&out_frame[WPL_REPLAY_FRAME_OFFSET_MOUSE_DY],
                          frame->input.mouse.delta.y);
  wpl_replay_write_f32_le(&out_frame[WPL_REPLAY_FRAME_OFFSET_WHEEL],
                          frame->input.mouse.wheel_delta);

  for (i = 0u; i < WPL_MOUSE_BUTTON_COUNT; i++) {
    out_frame[WPL_REPLAY_FRAME_OFFSET_MOUSE_DOWN + i] =
      wpl_replay_bool_to_u8(frame->input.mouse.button_down[i]);
    out_frame[WPL_REPLAY_FRAME_OFFSET_MOUSE_PRESSED + i] =
      wpl_replay_bool_to_u8(frame->input.mouse.button_pressed[i]);
    out_frame[WPL_REPLAY_FRAME_OFFSET_MOUSE_RELEASED + i] =
      wpl_replay_bool_to_u8(frame->input.mouse.button_released[i]);
  }

  for (i = 0u; i < WPL_KEY_COUNT; i++) {
    out_frame[WPL_REPLAY_FRAME_OFFSET_KEY_DOWN + i] =
      wpl_replay_bool_to_u8(frame->input.keyboard.key_down[i]);
    out_frame[WPL_REPLAY_FRAME_OFFSET_KEY_PRESSED + i] =
      wpl_replay_bool_to_u8(frame->input.keyboard.key_pressed[i]);
    out_frame[WPL_REPLAY_FRAME_OFFSET_KEY_RELEASED + i] =
      wpl_replay_bool_to_u8(frame->input.keyboard.key_released[i]);
  }

  out_frame[WPL_REPLAY_FRAME_OFFSET_SHIFT] =
    wpl_replay_bool_to_u8(frame->input.keyboard.shift_down);
  out_frame[WPL_REPLAY_FRAME_OFFSET_CTRL] =
    wpl_replay_bool_to_u8(frame->input.keyboard.ctrl_down);
  out_frame[WPL_REPLAY_FRAME_OFFSET_ALT] =
    wpl_replay_bool_to_u8(frame->input.keyboard.alt_down);

  return WPL_RESULT_OK;
}

WplResult
wpl_replay_decode_frame_v1(const uint8_t* data,
                           size_t size,
                           WplReplayFrameV1* out_frame)
{
  WplReplayFrameV1 frame;
  WplResult result = WPL_RESULT_OK;
  size_t i = 0u;

  if (data == NULL || out_frame == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  memset(out_frame, 0, sizeof(*out_frame));

  if (size < WPL_REPLAY_FRAME_SIZE_V1)
    return WPL_RESULT_PARSE_ERROR;

  memset(&frame, 0, sizeof(frame));

  frame.frame_index = wpl_replay_read_u64_le(
    &data[WPL_REPLAY_FRAME_OFFSET_FRAME_INDEX]);
  frame.time_microseconds = wpl_replay_read_u64_le(
    &data[WPL_REPLAY_FRAME_OFFSET_TIME_US]);
  frame.delta_microseconds = wpl_replay_read_u32_le(
    &data[WPL_REPLAY_FRAME_OFFSET_DELTA_US]);
  frame.input.mouse.position.x = wpl_replay_read_f32_le(
    &data[WPL_REPLAY_FRAME_OFFSET_MOUSE_X]);
  frame.input.mouse.position.y = wpl_replay_read_f32_le(
    &data[WPL_REPLAY_FRAME_OFFSET_MOUSE_Y]);
  frame.input.mouse.delta.x = wpl_replay_read_f32_le(
    &data[WPL_REPLAY_FRAME_OFFSET_MOUSE_DX]);
  frame.input.mouse.delta.y = wpl_replay_read_f32_le(
    &data[WPL_REPLAY_FRAME_OFFSET_MOUSE_DY]);
  frame.input.mouse.wheel_delta = wpl_replay_read_f32_le(
    &data[WPL_REPLAY_FRAME_OFFSET_WHEEL]);

  if (!wpl_replay_input_floats_are_finite(&frame.input))
    return WPL_RESULT_PARSE_ERROR;

  for (i = 0u; i < WPL_MOUSE_BUTTON_COUNT; i++) {
    result = wpl_replay_u8_to_bool(
      data[WPL_REPLAY_FRAME_OFFSET_MOUSE_DOWN + i],
      &frame.input.mouse.button_down[i]);
    if (result != WPL_RESULT_OK)
      return result;

    result = wpl_replay_u8_to_bool(
      data[WPL_REPLAY_FRAME_OFFSET_MOUSE_PRESSED + i],
      &frame.input.mouse.button_pressed[i]);
    if (result != WPL_RESULT_OK)
      return result;

    result = wpl_replay_u8_to_bool(
      data[WPL_REPLAY_FRAME_OFFSET_MOUSE_RELEASED + i],
      &frame.input.mouse.button_released[i]);
    if (result != WPL_RESULT_OK)
      return result;
  }

  for (i = 0u; i < WPL_KEY_COUNT; i++) {
    result = wpl_replay_u8_to_bool(data[WPL_REPLAY_FRAME_OFFSET_KEY_DOWN + i],
                                   &frame.input.keyboard.key_down[i]);
    if (result != WPL_RESULT_OK)
      return result;

    result = wpl_replay_u8_to_bool(
      data[WPL_REPLAY_FRAME_OFFSET_KEY_PRESSED + i],
      &frame.input.keyboard.key_pressed[i]);
    if (result != WPL_RESULT_OK)
      return result;

    result = wpl_replay_u8_to_bool(
      data[WPL_REPLAY_FRAME_OFFSET_KEY_RELEASED + i],
      &frame.input.keyboard.key_released[i]);
    if (result != WPL_RESULT_OK)
      return result;
  }

  result = wpl_replay_u8_to_bool(data[WPL_REPLAY_FRAME_OFFSET_SHIFT],
                                 &frame.input.keyboard.shift_down);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_replay_u8_to_bool(data[WPL_REPLAY_FRAME_OFFSET_CTRL],
                                 &frame.input.keyboard.ctrl_down);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_replay_u8_to_bool(data[WPL_REPLAY_FRAME_OFFSET_ALT],
                                 &frame.input.keyboard.alt_down);
  if (result != WPL_RESULT_OK)
    return result;

  for (i = 0u; i < WPL_REPLAY_FRAME_RESERVED_SIZE; i++) {
    if (data[WPL_REPLAY_FRAME_OFFSET_RESERVED + i] != 0u)
      return WPL_RESULT_PARSE_ERROR;
  }

  *out_frame = frame;
  return WPL_RESULT_OK;
}

WplResult
wpl_replay_validate_file_size_v1(uint64_t frame_count, size_t actual_size)
{
  uint64_t expected_size = WPL_REPLAY_HEADER_SIZE_V1;

  if (frame_count > WPL_REPLAY_MAX_FRAMES_V1)
    return WPL_RESULT_PARSE_ERROR;

  expected_size += frame_count * (uint64_t)WPL_REPLAY_FRAME_SIZE_V1;

  if (actual_size > (size_t)WPL_MAX_FILE_SIZE_V0_1)
    return WPL_RESULT_UNSUPPORTED;

  if ((uint64_t)actual_size < expected_size)
    return WPL_RESULT_PARSE_ERROR;

  return WPL_RESULT_OK;
}

WplResult
wpl_replay_recorder_create(WplReplayRecorder** out_recorder)
{
  if (out_recorder != NULL)
    *out_recorder = NULL;

  return WPL_RESULT_UNSUPPORTED;
}

void
wpl_replay_recorder_destroy(WplReplayRecorder* recorder)
{
  (void)recorder;
}

WplResult
wpl_replay_recorder_begin(WplReplayRecorder* recorder)
{
  (void)recorder;
  return WPL_RESULT_UNSUPPORTED;
}

WplResult
wpl_replay_recorder_record_frame(WplReplayRecorder* recorder,
                                 const WplInputState* input,
                                 float delta_time)
{
  (void)recorder;
  (void)input;
  (void)delta_time;
  return WPL_RESULT_UNSUPPORTED;
}

WplResult
wpl_replay_recorder_save(WplReplayRecorder* recorder, const char* path)
{
  (void)recorder;
  (void)path;
  return WPL_RESULT_UNSUPPORTED;
}

WplResult
wpl_replay_player_create(WplReplayPlayer** out_player)
{
  if (out_player != NULL)
    *out_player = NULL;

  return WPL_RESULT_UNSUPPORTED;
}

void
wpl_replay_player_destroy(WplReplayPlayer* player)
{
  (void)player;
}

WplResult
wpl_replay_player_load(WplReplayPlayer* player, const char* path)
{
  (void)player;
  (void)path;
  return WPL_RESULT_UNSUPPORTED;
}

WplResult
wpl_replay_player_next(WplReplayPlayer* player,
                       WplInputState* out_input,
                       float* out_delta_time,
                       bool* out_has_frame)
{
  (void)player;
  if (out_input != NULL)
    *out_input = (WplInputState){0};
  if (out_delta_time != NULL)
    *out_delta_time = 0.0f;
  if (out_has_frame != NULL)
    *out_has_frame = false;

  return WPL_RESULT_UNSUPPORTED;
}
