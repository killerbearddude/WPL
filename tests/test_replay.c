/* test_replay.c - Replay recorder/player behavior tests. */

#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700

#include "wpl_replay_internal.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wpl/wpl_file.h"

#define WPL_TEST_REPLAY_HEADER_MAGIC_OFFSET 0u
#define WPL_TEST_REPLAY_HEADER_VERSION_OFFSET 4u
#define WPL_TEST_REPLAY_FRAME_BOOL_OFFSET 40u
#define WPL_TEST_REPLAY_FRAME_RESERVED_OFFSET 163u
#define WPL_TEST_EPSILON 0.0001f

static WplInputState
wpl_test_input(unsigned frame_id)
{
  WplInputState input;

  memset(&input, 0, sizeof(input));
  input.mouse.position.x = 10.0f + (float)frame_id;
  input.mouse.position.y = 20.0f + (float)frame_id;
  input.mouse.delta.x = 1.0f + (float)frame_id;
  input.mouse.delta.y = -1.0f;
  input.mouse.wheel_delta = (float)frame_id;
  input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] = true;
  input.mouse.button_pressed[WPL_MOUSE_BUTTON_RIGHT] = true;
  input.mouse.button_released[WPL_MOUSE_BUTTON_MIDDLE] = (frame_id % 2u) != 0u;
  input.keyboard.key_down[WPL_KEY_A] = true;
  input.keyboard.key_pressed[WPL_KEY_SPACE] = true;
  input.keyboard.key_released[WPL_KEY_ESCAPE] = true;
  input.keyboard.shift_down = true;
  input.keyboard.ctrl_down = (frame_id % 2u) != 0u;
  input.keyboard.alt_down = true;

  return input;
}

static bool
wpl_test_float_equal(float a, float b)
{
  float delta = a - b;

  if (delta < 0.0f)
    delta = -delta;

  return delta <= WPL_TEST_EPSILON;
}

static void
wpl_test_assert_input_equal(WplInputState actual, WplInputState expected)
{
  assert(wpl_test_float_equal(actual.mouse.position.x,
                              expected.mouse.position.x));
  assert(wpl_test_float_equal(actual.mouse.position.y,
                              expected.mouse.position.y));
  assert(wpl_test_float_equal(actual.mouse.delta.x, expected.mouse.delta.x));
  assert(wpl_test_float_equal(actual.mouse.delta.y, expected.mouse.delta.y));
  assert(wpl_test_float_equal(actual.mouse.wheel_delta,
                              expected.mouse.wheel_delta));
  assert(actual.mouse.button_down[WPL_MOUSE_BUTTON_LEFT]
         == expected.mouse.button_down[WPL_MOUSE_BUTTON_LEFT]);
  assert(actual.mouse.button_pressed[WPL_MOUSE_BUTTON_RIGHT]
         == expected.mouse.button_pressed[WPL_MOUSE_BUTTON_RIGHT]);
  assert(actual.mouse.button_released[WPL_MOUSE_BUTTON_MIDDLE]
         == expected.mouse.button_released[WPL_MOUSE_BUTTON_MIDDLE]);
  assert(actual.keyboard.key_down[WPL_KEY_A]
         == expected.keyboard.key_down[WPL_KEY_A]);
  assert(actual.keyboard.key_pressed[WPL_KEY_SPACE]
         == expected.keyboard.key_pressed[WPL_KEY_SPACE]);
  assert(actual.keyboard.key_released[WPL_KEY_ESCAPE]
         == expected.keyboard.key_released[WPL_KEY_ESCAPE]);
  assert(actual.keyboard.shift_down == expected.keyboard.shift_down);
  assert(actual.keyboard.ctrl_down == expected.keyboard.ctrl_down);
  assert(actual.keyboard.alt_down == expected.keyboard.alt_down);
}

static bool
wpl_test_make_temp_path(char path[64])
{
  int fd = -1;

  strcpy(path, "/tmp/wpl_replay_test_XXXXXX");
  fd = mkstemp(path);
  if (fd < 0)
    return false;

  close(fd);
  unlink(path);
  return true;
}

static void
wpl_test_write_u32_le(uint8_t* data, uint32_t value)
{
  data[0] = (uint8_t)(value & 0xffu);
  data[1] = (uint8_t)((value >> 8) & 0xffu);
  data[2] = (uint8_t)((value >> 16) & 0xffu);
  data[3] = (uint8_t)((value >> 24) & 0xffu);
}

static void
wpl_test_write_valid_replay_file(const char* path, const WplReplayFrameV1* frame)
{
  uint8_t bytes[WPL_REPLAY_HEADER_SIZE_V1 + WPL_REPLAY_FRAME_SIZE_V1];

  assert(wpl_replay_encode_header_v1(1u, bytes) == WPL_RESULT_OK);
  assert(wpl_replay_encode_frame_v1(frame,
                                    bytes + WPL_REPLAY_HEADER_SIZE_V1)
         == WPL_RESULT_OK);
  assert(wpl_write_entire_file(path, bytes, sizeof(bytes)) == WPL_RESULT_OK);
}

static void
wpl_test_write_zero_frame_replay_file(const char* path)
{
  uint8_t bytes[WPL_REPLAY_HEADER_SIZE_V1];

  assert(wpl_replay_encode_header_v1(0u, bytes) == WPL_RESULT_OK);
  assert(wpl_write_entire_file(path, bytes, sizeof(bytes)) == WPL_RESULT_OK);
}

static void
wpl_test_write_malformed_replay_file(const char* path, unsigned kind)
{
  WplReplayFrameV1 frame;
  uint8_t bytes[WPL_REPLAY_HEADER_SIZE_V1 + WPL_REPLAY_FRAME_SIZE_V1];

  memset(&frame, 0, sizeof(frame));
  frame.input = wpl_test_input(0u);
  assert(wpl_replay_encode_header_v1(1u, bytes) == WPL_RESULT_OK);
  assert(wpl_replay_encode_frame_v1(&frame,
                                    bytes + WPL_REPLAY_HEADER_SIZE_V1)
         == WPL_RESULT_OK);

  if (kind == 0u) {
    bytes[WPL_TEST_REPLAY_HEADER_MAGIC_OFFSET] = 'X';
  } else if (kind == 1u) {
    wpl_test_write_u32_le(&bytes[WPL_TEST_REPLAY_HEADER_VERSION_OFFSET], 999u);
  } else if (kind == 2u) {
    assert(wpl_write_entire_file(path, bytes, WPL_REPLAY_HEADER_SIZE_V1 - 1u)
           == WPL_RESULT_OK);
    return;
  } else if (kind == 3u) {
    bytes[WPL_REPLAY_HEADER_SIZE_V1 + WPL_TEST_REPLAY_FRAME_BOOL_OFFSET] = 2u;
  } else {
    bytes[WPL_REPLAY_HEADER_SIZE_V1 + WPL_TEST_REPLAY_FRAME_RESERVED_OFFSET] = 1u;
  }

  assert(wpl_write_entire_file(path, bytes, sizeof(bytes)) == WPL_RESULT_OK);
}

static void
test_recorder_create_destroy(void)
{
  WplReplayRecorder* recorder = NULL;

  assert(wpl_replay_recorder_create(NULL) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_recorder_create(&recorder) == WPL_RESULT_OK);
  assert(recorder != NULL);
  wpl_replay_recorder_destroy(recorder);
  wpl_replay_recorder_destroy(NULL);
}

static void
 test_recorder_record_validation(void)
{
  WplReplayRecorder* recorder = NULL;
  WplInputState input = wpl_test_input(0u);
  float nan_value = 0.0f / 0.0f;
  float inf_value = 1.0f / 0.0f;

  assert(wpl_replay_recorder_create(&recorder) == WPL_RESULT_OK);
  assert(wpl_replay_recorder_record_frame(NULL, &input, 0.016f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_recorder_record_frame(recorder, NULL, 0.016f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_recorder_record_frame(recorder, &input, 0.016f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_recorder_begin(recorder) == WPL_RESULT_OK);
  assert(wpl_replay_recorder_record_frame(recorder, &input, -0.1f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_recorder_record_frame(recorder, &input, nan_value)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_recorder_record_frame(recorder, &input, inf_value)
         == WPL_RESULT_INVALID_ARGUMENT);
  wpl_replay_recorder_destroy(recorder);
}

static void
 test_recorder_save_validation_and_zero_frame(void)
{
  WplReplayRecorder* recorder = NULL;
  char path[64];
  WplFileData data = {0};

  assert(wpl_test_make_temp_path(path));
  assert(wpl_replay_recorder_create(&recorder) == WPL_RESULT_OK);
  assert(wpl_replay_recorder_save(NULL, path) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_recorder_save(recorder, NULL) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_recorder_save(recorder, "") == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_recorder_save(recorder, path) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_recorder_begin(recorder) == WPL_RESULT_OK);
  assert(wpl_replay_recorder_save(recorder, path) == WPL_RESULT_OK);
  assert(wpl_read_entire_file(path, &data) == WPL_RESULT_OK);
  assert(data.size == WPL_REPLAY_HEADER_SIZE_V1);
  assert(memcmp(data.data, WPL_REPLAY_MAGIC, 4u) == 0);
  wpl_free_file_data(&data);
  unlink(path);
  wpl_replay_recorder_destroy(recorder);
}

static void
 test_recorder_records_and_saves_one_frame(void)
{
  WplReplayRecorder* recorder = NULL;
  WplInputState input = wpl_test_input(1u);
  char path[64];
  WplFileData data = {0};
  WplReplayHeaderV1 header = {0};
  WplReplayFrameV1 frame;

  assert(wpl_test_make_temp_path(path));
  assert(wpl_replay_recorder_create(&recorder) == WPL_RESULT_OK);
  assert(wpl_replay_recorder_begin(recorder) == WPL_RESULT_OK);
  assert(wpl_replay_recorder_record_frame(recorder, &input, 0.016667f)
         == WPL_RESULT_OK);
  assert(wpl_replay_recorder_save(recorder, path) == WPL_RESULT_OK);
  assert(wpl_read_entire_file(path, &data) == WPL_RESULT_OK);
  assert(data.size == WPL_REPLAY_HEADER_SIZE_V1 + WPL_REPLAY_FRAME_SIZE_V1);
  assert(memcmp(data.data, WPL_REPLAY_MAGIC, 4u) == 0);
  assert(wpl_replay_decode_header_v1((const uint8_t*)data.data,
                                     data.size,
                                     &header) == WPL_RESULT_OK);
  assert(header.frame_count == 1u);
  assert(wpl_replay_decode_frame_v1(((const uint8_t*)data.data)
                                      + WPL_REPLAY_HEADER_SIZE_V1,
                                    data.size - WPL_REPLAY_HEADER_SIZE_V1,
                                    &frame) == WPL_RESULT_OK);
  assert(frame.frame_index == 0u);
  assert(frame.time_microseconds == 16667u);
  assert(frame.delta_microseconds == 16667u);
  wpl_test_assert_input_equal(frame.input, input);
  wpl_free_file_data(&data);
  unlink(path);
  wpl_replay_recorder_destroy(recorder);
}

static void
 test_begin_resets_recorder_state(void)
{
  WplReplayRecorder* recorder = NULL;
  WplInputState input = wpl_test_input(0u);
  char path[64];
  WplFileData data = {0};
  WplReplayHeaderV1 header = {0};
  WplReplayFrameV1 frame;

  assert(wpl_test_make_temp_path(path));
  assert(wpl_replay_recorder_create(&recorder) == WPL_RESULT_OK);
  assert(wpl_replay_recorder_begin(recorder) == WPL_RESULT_OK);
  assert(wpl_replay_recorder_record_frame(recorder, &input, 0.1f)
         == WPL_RESULT_OK);
  assert(wpl_replay_recorder_begin(recorder) == WPL_RESULT_OK);
  assert(wpl_replay_recorder_record_frame(recorder, &input, 0.002f)
         == WPL_RESULT_OK);
  assert(wpl_replay_recorder_save(recorder, path) == WPL_RESULT_OK);
  assert(wpl_read_entire_file(path, &data) == WPL_RESULT_OK);
  assert(wpl_replay_decode_header_v1((const uint8_t*)data.data,
                                     data.size,
                                     &header) == WPL_RESULT_OK);
  assert(header.frame_count == 1u);
  assert(wpl_replay_decode_frame_v1(((const uint8_t*)data.data)
                                      + WPL_REPLAY_HEADER_SIZE_V1,
                                    data.size - WPL_REPLAY_HEADER_SIZE_V1,
                                    &frame) == WPL_RESULT_OK);
  assert(frame.frame_index == 0u);
  assert(frame.time_microseconds == 2000u);
  assert(frame.delta_microseconds == 2000u);
  wpl_free_file_data(&data);
  unlink(path);
  wpl_replay_recorder_destroy(recorder);
}

static void
 test_player_create_destroy_and_next_validation(void)
{
  WplReplayPlayer* player = NULL;
  WplInputState input;
  float delta = 1.0f;
  bool has_frame = true;

  assert(wpl_replay_player_create(NULL) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_player_create(&player) == WPL_RESULT_OK);
  assert(player != NULL);
  assert(wpl_replay_player_next(NULL, &input, &delta, &has_frame)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_player_next(player, NULL, &delta, &has_frame)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_player_next(player, &input, NULL, &has_frame)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_player_next(player, &input, &delta, NULL)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_player_next(player, &input, &delta, &has_frame)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(delta == 0.0f);
  assert(has_frame == false);
  wpl_replay_player_destroy(player);
  wpl_replay_player_destroy(NULL);
}

static void
 test_player_load_validation_and_zero_frame(void)
{
  WplReplayPlayer* player = NULL;
  char path[64];
  WplInputState input;
  float delta = 1.0f;
  bool has_frame = true;

  assert(wpl_test_make_temp_path(path));
  assert(wpl_replay_player_create(&player) == WPL_RESULT_OK);
  assert(wpl_replay_player_load(NULL, path) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_player_load(player, NULL) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_player_load(player, "") == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_player_load(player, path) == WPL_RESULT_IO_ERROR);
  wpl_test_write_zero_frame_replay_file(path);
  assert(wpl_replay_player_load(player, path) == WPL_RESULT_OK);
  assert(wpl_replay_player_next(player, &input, &delta, &has_frame)
         == WPL_RESULT_OK);
  assert(!has_frame);
  assert(delta == 0.0f);
  unlink(path);
  wpl_replay_player_destroy(player);
}

static void
 test_player_load_one_frame_and_next(void)
{
  WplReplayPlayer* player = NULL;
  WplReplayFrameV1 frame;
  WplInputState input;
  float delta = 0.0f;
  bool has_frame = false;
  char path[64];

  memset(&frame, 0, sizeof(frame));
  frame.frame_index = 0u;
  frame.time_microseconds = 33333u;
  frame.delta_microseconds = 33333u;
  frame.input = wpl_test_input(2u);

  assert(wpl_test_make_temp_path(path));
  wpl_test_write_valid_replay_file(path, &frame);
  assert(wpl_replay_player_create(&player) == WPL_RESULT_OK);
  assert(wpl_replay_player_load(player, path) == WPL_RESULT_OK);
  assert(wpl_replay_player_next(player, &input, &delta, &has_frame)
         == WPL_RESULT_OK);
  assert(has_frame);
  assert(wpl_test_float_equal(delta, 0.033333f));
  wpl_test_assert_input_equal(input, frame.input);
  assert(wpl_replay_player_next(player, &input, &delta, &has_frame)
         == WPL_RESULT_OK);
  assert(!has_frame);
  assert(delta == 0.0f);
  assert(wpl_replay_player_next(player, &input, &delta, &has_frame)
         == WPL_RESULT_OK);
  assert(!has_frame);
  unlink(path);
  wpl_replay_player_destroy(player);
}

static void
 test_roundtrip_two_frames(void)
{
  WplReplayRecorder* recorder = NULL;
  WplReplayPlayer* player = NULL;
  WplInputState input0 = wpl_test_input(0u);
  WplInputState input1 = wpl_test_input(1u);
  WplInputState played;
  float delta = 0.0f;
  bool has_frame = false;
  char path[64];

  assert(wpl_test_make_temp_path(path));
  assert(wpl_replay_recorder_create(&recorder) == WPL_RESULT_OK);
  assert(wpl_replay_recorder_begin(recorder) == WPL_RESULT_OK);
  assert(wpl_replay_recorder_record_frame(recorder, &input0, 0.010f)
         == WPL_RESULT_OK);
  assert(wpl_replay_recorder_record_frame(recorder, &input1, 0.025f)
         == WPL_RESULT_OK);
  assert(wpl_replay_recorder_save(recorder, path) == WPL_RESULT_OK);

  assert(wpl_replay_player_create(&player) == WPL_RESULT_OK);
  assert(wpl_replay_player_load(player, path) == WPL_RESULT_OK);
  assert(wpl_replay_player_next(player, &played, &delta, &has_frame)
         == WPL_RESULT_OK);
  assert(has_frame);
  assert(wpl_test_float_equal(delta, 0.010f));
  wpl_test_assert_input_equal(played, input0);
  assert(wpl_replay_player_next(player, &played, &delta, &has_frame)
         == WPL_RESULT_OK);
  assert(has_frame);
  assert(wpl_test_float_equal(delta, 0.025f));
  wpl_test_assert_input_equal(played, input1);
  assert(wpl_replay_player_next(player, &played, &delta, &has_frame)
         == WPL_RESULT_OK);
  assert(!has_frame);

  unlink(path);
  wpl_replay_player_destroy(player);
  wpl_replay_recorder_destroy(recorder);
}

static void
 test_malformed_loads_and_preserves_previous_replay(void)
{
  WplReplayPlayer* player = NULL;
  WplReplayFrameV1 frame;
  WplInputState input;
  float delta = 0.0f;
  bool has_frame = false;
  char valid_path[64];
  char bad_path[64];

  memset(&frame, 0, sizeof(frame));
  frame.delta_microseconds = 12000u;
  frame.time_microseconds = 12000u;
  frame.input = wpl_test_input(3u);

  assert(wpl_test_make_temp_path(valid_path));
  assert(wpl_test_make_temp_path(bad_path));
  wpl_test_write_valid_replay_file(valid_path, &frame);
  assert(wpl_replay_player_create(&player) == WPL_RESULT_OK);
  assert(wpl_replay_player_load(player, valid_path) == WPL_RESULT_OK);

  wpl_test_write_malformed_replay_file(bad_path, 0u);
  assert(wpl_replay_player_load(player, bad_path) == WPL_RESULT_PARSE_ERROR);
  assert(wpl_replay_player_next(player, &input, &delta, &has_frame)
         == WPL_RESULT_OK);
  assert(has_frame);
  wpl_test_assert_input_equal(input, frame.input);

  unlink(valid_path);
  unlink(bad_path);
  wpl_replay_player_destroy(player);
}

static void
 test_malformed_load_results(void)
{
  WplReplayPlayer* player = NULL;
  char path[64];

  assert(wpl_test_make_temp_path(path));
  assert(wpl_replay_player_create(&player) == WPL_RESULT_OK);

  wpl_test_write_malformed_replay_file(path, 0u);
  assert(wpl_replay_player_load(player, path) == WPL_RESULT_PARSE_ERROR);

  wpl_test_write_malformed_replay_file(path, 1u);
  assert(wpl_replay_player_load(player, path) == WPL_RESULT_UNSUPPORTED);

  wpl_test_write_malformed_replay_file(path, 2u);
  assert(wpl_replay_player_load(player, path) == WPL_RESULT_PARSE_ERROR);

  wpl_test_write_malformed_replay_file(path, 3u);
  assert(wpl_replay_player_load(player, path) == WPL_RESULT_PARSE_ERROR);

  wpl_test_write_malformed_replay_file(path, 4u);
  assert(wpl_replay_player_load(player, path) == WPL_RESULT_PARSE_ERROR);

  unlink(path);
  wpl_replay_player_destroy(player);
}

int
main(void)
{
  test_recorder_create_destroy();
  test_recorder_record_validation();
  test_recorder_save_validation_and_zero_frame();
  test_recorder_records_and_saves_one_frame();
  test_begin_resets_recorder_state();
  test_player_create_destroy_and_next_validation();
  test_player_load_validation_and_zero_frame();
  test_player_load_one_frame_and_next();
  test_roundtrip_two_frames();
  test_malformed_loads_and_preserves_previous_replay();
  test_malformed_load_results();

  return 0;
}
