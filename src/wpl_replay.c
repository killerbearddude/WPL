/* wpl_replay.c - Replay v1 fixed binary format helpers. */

#include "wpl_replay_internal.h"

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "wpl/wpl_base.h"
#include "wpl/wpl_file.h"

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

#define WPL_REPLAY_INITIAL_FRAME_CAPACITY 1024u

struct WplReplayRecorder {
  WplReplayFrameV1* frames;
  uint64_t frame_count;
  uint64_t frame_capacity;
  uint64_t next_frame_index;
  uint64_t accumulated_time_microseconds;
  bool recording;
};

struct WplReplayPlayer {
  WplReplayFrameV1* frames;
  uint64_t frame_count;
  uint64_t cursor;
  bool loaded;
};

static void
wpl_replay_write_u32_le(uint8_t* dst, uint32_t value)
{
  dst[0] = (uint8_t)(value & 0xffu);
  dst[1] = (uint8_t)((value >> 8u) & 0xffu);
  dst[2] = (uint8_t)((value >> 16u) & 0xffu);
  dst[3] = (uint8_t)((value >> 24u) & 0xffu);
}

static void
wpl_replay_write_u64_le(uint8_t* dst, uint64_t value)
{
  dst[0] = (uint8_t)(value & 0xffu);
  dst[1] = (uint8_t)((value >> 8u) & 0xffu);
  dst[2] = (uint8_t)((value >> 16u) & 0xffu);
  dst[3] = (uint8_t)((value >> 24u) & 0xffu);
  dst[4] = (uint8_t)((value >> 32u) & 0xffu);
  dst[5] = (uint8_t)((value >> 40u) & 0xffu);
  dst[6] = (uint8_t)((value >> 48u) & 0xffu);
  dst[7] = (uint8_t)((value >> 56u) & 0xffu);
}

static uint32_t
wpl_replay_read_u32_le(const uint8_t* src)
{
  return ((uint32_t)src[0])
         | ((uint32_t)src[1] << 8u)
         | ((uint32_t)src[2] << 16u)
         | ((uint32_t)src[3] << 24u);
}

static uint64_t
wpl_replay_read_u64_le(const uint8_t* src)
{
  return ((uint64_t)src[0])
         | ((uint64_t)src[1] << 8u)
         | ((uint64_t)src[2] << 16u)
         | ((uint64_t)src[3] << 24u)
         | ((uint64_t)src[4] << 32u)
         | ((uint64_t)src[5] << 40u)
         | ((uint64_t)src[6] << 48u)
         | ((uint64_t)src[7] << 56u);
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
  if (input == NULL)
    return false;

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
  uint32_t version;
  uint32_t header_size;
  uint32_t frame_size;
  uint64_t frame_count;
  uint32_t flags;
  uint32_t reserved;

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
  if (header_size != WPL_REPLAY_HEADER_SIZE_V1)
    return WPL_RESULT_PARSE_ERROR;

  frame_size = wpl_replay_read_u32_le(
    &data[WPL_REPLAY_HEADER_OFFSET_FRAME_SIZE]);
  if (frame_size != WPL_REPLAY_FRAME_SIZE_V1)
    return WPL_RESULT_PARSE_ERROR;

  frame_count = wpl_replay_read_u64_le(
    &data[WPL_REPLAY_HEADER_OFFSET_FRAME_COUNT]);
  if (frame_count > WPL_REPLAY_MAX_FRAMES_V1)
    return WPL_RESULT_UNSUPPORTED;

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
  size_t i;

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
  WplResult result;
  size_t i;

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
    return WPL_RESULT_UNSUPPORTED;

  expected_size += frame_count * (uint64_t)WPL_REPLAY_FRAME_SIZE_V1;

  if (actual_size > (size_t)WPL_MAX_FILE_SIZE_V0_1)
    return WPL_RESULT_UNSUPPORTED;

  if ((uint64_t)actual_size < expected_size)
    return WPL_RESULT_PARSE_ERROR;

  return WPL_RESULT_OK;
}

static bool
wpl_replay_delta_time_is_valid(float delta_time)
{
  return isfinite(delta_time) != 0 && delta_time >= 0.0f;
}

static WplResult
wpl_replay_delta_to_microseconds(float delta_time,
                                 uint32_t* out_microseconds)
{
  double microseconds;

  if (out_microseconds == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  *out_microseconds = 0u;

  if (!wpl_replay_delta_time_is_valid(delta_time))
    return WPL_RESULT_INVALID_ARGUMENT;

  microseconds = ((double)delta_time * 1000000.0) + 0.5;
  if (microseconds > (double)UINT32_MAX)
    return WPL_RESULT_INVALID_ARGUMENT;

  *out_microseconds = (uint32_t)microseconds;
  return WPL_RESULT_OK;
}

static WplResult
wpl_replay_expected_file_size_v1(uint64_t frame_count, size_t* out_size)
{
  uint64_t total_size = WPL_REPLAY_HEADER_SIZE_V1;

  if (out_size == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  *out_size = 0u;

  if (frame_count > WPL_REPLAY_MAX_FRAMES_V1)
    return WPL_RESULT_UNSUPPORTED;

  total_size += frame_count * (uint64_t)WPL_REPLAY_FRAME_SIZE_V1;
  if (total_size > (uint64_t)WPL_MAX_FILE_SIZE_V0_1
      || total_size > (uint64_t)SIZE_MAX)
    return WPL_RESULT_UNSUPPORTED;

  *out_size = (size_t)total_size;
  return WPL_RESULT_OK;
}

static WplResult
wpl_replay_recorder_ensure_capacity(WplReplayRecorder* recorder)
{
  uint64_t new_capacity;
  WplReplayFrameV1* new_frames;

  if (recorder == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (recorder->frame_count < recorder->frame_capacity)
    return WPL_RESULT_OK;

  if (recorder->frame_count >= WPL_REPLAY_MAX_FRAMES_V1)
    return WPL_RESULT_CAPACITY_EXCEEDED;

  if (recorder->frame_capacity == 0u) {
    new_capacity = WPL_REPLAY_INITIAL_FRAME_CAPACITY;
  } else {
    if (recorder->frame_capacity > (UINT64_MAX / 2u))
      return WPL_RESULT_CAPACITY_EXCEEDED;
    new_capacity = recorder->frame_capacity * 2u;
  }

  if (new_capacity < recorder->frame_count + 1u)
    new_capacity = recorder->frame_count + 1u;

  if (new_capacity > WPL_REPLAY_MAX_FRAMES_V1)
    new_capacity = WPL_REPLAY_MAX_FRAMES_V1;

  if (new_capacity > (uint64_t)(SIZE_MAX / sizeof(WplReplayFrameV1)))
    return WPL_RESULT_OUT_OF_MEMORY;

  new_frames = (WplReplayFrameV1*)realloc(
    recorder->frames,
    (size_t)new_capacity * sizeof(WplReplayFrameV1));
  if (new_frames == NULL)
    return WPL_RESULT_OUT_OF_MEMORY;

  recorder->frames = new_frames;
  recorder->frame_capacity = new_capacity;
  return WPL_RESULT_OK;
}

WplResult
wpl_replay_recorder_create(WplReplayRecorder** out_recorder)
{
  WplReplayRecorder* recorder;

  if (out_recorder == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  *out_recorder = NULL;
  recorder = (WplReplayRecorder*)calloc(1u, sizeof(*recorder));
  if (recorder == NULL)
    return WPL_RESULT_OUT_OF_MEMORY;

  *out_recorder = recorder;
  return WPL_RESULT_OK;
}

void
wpl_replay_recorder_destroy(WplReplayRecorder* recorder)
{
  if (recorder == NULL)
    return;

  free(recorder->frames);
  free(recorder);
}

WplResult
wpl_replay_recorder_begin(WplReplayRecorder* recorder)
{
  if (recorder == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  recorder->frame_count = 0u;
  recorder->next_frame_index = 0u;
  recorder->accumulated_time_microseconds = 0u;
  recorder->recording = true;
  return WPL_RESULT_OK;
}

WplResult
wpl_replay_recorder_record_frame(WplReplayRecorder* recorder,
                                 const WplInputState* input,
                                 float delta_time)
{
  WplReplayFrameV1 frame;
  WplResult result;
  uint32_t delta_microseconds;

  if (recorder == NULL || input == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (!recorder->recording)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (!wpl_replay_input_floats_are_finite(input))
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_replay_delta_to_microseconds(delta_time, &delta_microseconds);
  if (result != WPL_RESULT_OK)
    return result;

  if (recorder->frame_count >= WPL_REPLAY_MAX_FRAMES_V1)
    return WPL_RESULT_CAPACITY_EXCEEDED;

  if (UINT64_MAX - recorder->accumulated_time_microseconds
      < (uint64_t)delta_microseconds)
    return WPL_RESULT_UNSUPPORTED;

  result = wpl_replay_recorder_ensure_capacity(recorder);
  if (result != WPL_RESULT_OK)
    return result;

  memset(&frame, 0, sizeof(frame));
  frame.frame_index = recorder->next_frame_index;
  frame.delta_microseconds = delta_microseconds;
  frame.time_microseconds = (recorder->accumulated_time_microseconds
                             + (uint64_t)delta_microseconds);
  frame.input = *input;

  recorder->frames[recorder->frame_count] = frame;
  recorder->frame_count++;
  recorder->next_frame_index++;
  recorder->accumulated_time_microseconds = frame.time_microseconds;
  return WPL_RESULT_OK;
}

WplResult
wpl_replay_recorder_save(WplReplayRecorder* recorder, const char* path)
{
  uint8_t* bytes;
  size_t total_size;
  WplResult result;
  uint64_t i;

  if (recorder == NULL || path == NULL || path[0] == '\0')
    return WPL_RESULT_INVALID_ARGUMENT;

  if (!recorder->recording)
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_replay_expected_file_size_v1(recorder->frame_count,
                                            &total_size);
  if (result != WPL_RESULT_OK)
    return result;

  bytes = (uint8_t*)malloc(total_size);
  if (bytes == NULL)
    return WPL_RESULT_OUT_OF_MEMORY;

  result = wpl_replay_encode_header_v1(recorder->frame_count, bytes);
  if (result == WPL_RESULT_OK) {
    for (i = 0u; i < recorder->frame_count; i++) {
      result = wpl_replay_encode_frame_v1(
        &recorder->frames[i],
        bytes + WPL_REPLAY_HEADER_SIZE_V1
          + ((size_t)i * WPL_REPLAY_FRAME_SIZE_V1));
      if (result != WPL_RESULT_OK)
        break;
    }
  }

  if (result == WPL_RESULT_OK)
    result = wpl_write_entire_file(path, bytes, total_size);

  free(bytes);
  return result;
}

WplResult
wpl_replay_player_create(WplReplayPlayer** out_player)
{
  WplReplayPlayer* player;

  if (out_player == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  *out_player = NULL;
  player = (WplReplayPlayer*)calloc(1u, sizeof(*player));
  if (player == NULL)
    return WPL_RESULT_OUT_OF_MEMORY;

  *out_player = player;
  return WPL_RESULT_OK;
}

void
wpl_replay_player_destroy(WplReplayPlayer* player)
{
  if (player == NULL)
    return;

  free(player->frames);
  free(player);
}

WplResult
wpl_replay_player_load(WplReplayPlayer* player, const char* path)
{
  WplFileData file = {0};
  WplReplayHeaderV1 header = {0};
  WplReplayFrameV1* new_frames = NULL;
  WplResult result;
  size_t expected_size;
  uint64_t expected_time = 0u;
  uint64_t i;

  if (player == NULL || path == NULL || path[0] == '\0')
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_read_entire_file(path, &file);
  if (result != WPL_RESULT_OK)
    return result;

  if (file.size < WPL_REPLAY_HEADER_SIZE_V1) {
    result = WPL_RESULT_PARSE_ERROR;
    goto cleanup;
  }

  result = wpl_replay_decode_header_v1((const uint8_t*)file.data,
                                       file.size,
                                       &header);
  if (result != WPL_RESULT_OK)
    goto cleanup;

  result = wpl_replay_validate_file_size_v1(header.frame_count, file.size);
  if (result != WPL_RESULT_OK)
    goto cleanup;

  result = wpl_replay_expected_file_size_v1(header.frame_count,
                                            &expected_size);
  if (result != WPL_RESULT_OK)
    goto cleanup;

  if (file.size != expected_size) {
    result = WPL_RESULT_PARSE_ERROR;
    goto cleanup;
  }

  if (header.frame_count > 0u) {
    if (header.frame_count > (uint64_t)(SIZE_MAX / sizeof(WplReplayFrameV1))) {
      result = WPL_RESULT_OUT_OF_MEMORY;
      goto cleanup;
    }

    new_frames = (WplReplayFrameV1*)malloc(
      (size_t)header.frame_count * sizeof(WplReplayFrameV1));
    if (new_frames == NULL) {
      result = WPL_RESULT_OUT_OF_MEMORY;
      goto cleanup;
    }
  }

  for (i = 0u; i < header.frame_count; i++) {
    result = wpl_replay_decode_frame_v1(
      ((const uint8_t*)file.data) + WPL_REPLAY_HEADER_SIZE_V1
        + ((size_t)i * WPL_REPLAY_FRAME_SIZE_V1),
      file.size - (WPL_REPLAY_HEADER_SIZE_V1
                   + ((size_t)i * WPL_REPLAY_FRAME_SIZE_V1)),
      &new_frames[i]);
    if (result != WPL_RESULT_OK)
      goto cleanup;

    if (new_frames[i].frame_index != i) {
      result = WPL_RESULT_PARSE_ERROR;
      goto cleanup;
    }

    if (UINT64_MAX - expected_time
        < (uint64_t)new_frames[i].delta_microseconds) {
      result = WPL_RESULT_PARSE_ERROR;
      goto cleanup;
    }

    expected_time += (uint64_t)new_frames[i].delta_microseconds;
    if (new_frames[i].time_microseconds != expected_time) {
      result = WPL_RESULT_PARSE_ERROR;
      goto cleanup;
    }
  }

  free(player->frames);
  player->frames = new_frames;
  player->frame_count = header.frame_count;
  player->cursor = 0u;
  player->loaded = true;
  new_frames = NULL;
  result = WPL_RESULT_OK;

cleanup:
  free(new_frames);
  wpl_free_file_data(&file);
  return result;
}

WplResult
wpl_replay_player_next(WplReplayPlayer* player,
                       WplInputState* out_input,
                       float* out_delta_time,
                       bool* out_has_frame)
{
  WplReplayFrameV1* frame;

  if (out_input != NULL)
    memset(out_input, 0, sizeof(*out_input));
  if (out_delta_time != NULL)
    *out_delta_time = 0.0f;
  if (out_has_frame != NULL)
    *out_has_frame = false;

  if (player == NULL || out_input == NULL || out_delta_time == NULL
      || out_has_frame == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (!player->loaded)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (player->cursor >= player->frame_count)
    return WPL_RESULT_OK;

  frame = &player->frames[player->cursor];
  *out_input = frame->input;
  *out_delta_time = (float)((double)frame->delta_microseconds / 1000000.0);
  *out_has_frame = true;
  player->cursor++;
  return WPL_RESULT_OK;
}
