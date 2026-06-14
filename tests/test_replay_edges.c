/* test_replay_edges.c - Replay recorder/player edge behavior tests. */

#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700

#include "wpl_replay_internal.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "wpl/wpl_file.h"

#define WPL_TEST_EPSILON 0.0001f

static WplInputState
wpl_test_input(unsigned frame_id)
{
  WplInputState input;

  memset(&input, 0, sizeof(input));
  input.mouse.position.x = 20.0f + (float)frame_id;
  input.mouse.position.y = -10.0f - (float)frame_id;
  input.mouse.delta.x = 0.5f + (float)frame_id;
  input.mouse.delta.y = -0.5f;
  input.mouse.wheel_delta = (float)frame_id;
  input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] = true;
  input.mouse.button_pressed[WPL_MOUSE_BUTTON_RIGHT] = (frame_id % 2u) != 0u;
  input.mouse.button_released[WPL_MOUSE_BUTTON_MIDDLE] = true;
  input.keyboard.key_down[WPL_KEY_A] = true;
  input.keyboard.key_pressed[WPL_KEY_SPACE] = true;
  input.keyboard.key_released[WPL_KEY_ESCAPE] = (frame_id % 2u) == 0u;
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

static void
wpl_test_assert_input_zeroed(WplInputState input)
{
  size_t i;

  assert(input.mouse.position.x == 0.0f);
  assert(input.mouse.position.y == 0.0f);
  assert(input.mouse.delta.x == 0.0f);
  assert(input.mouse.delta.y == 0.0f);
  assert(input.mouse.wheel_delta == 0.0f);

  for (i = 0u; i < WPL_MOUSE_BUTTON_COUNT; i++) {
    assert(!input.mouse.button_down[i]);
    assert(!input.mouse.button_pressed[i]);
    assert(!input.mouse.button_released[i]);
  }

  for (i = 0u; i < WPL_KEY_COUNT; i++) {
    assert(!input.keyboard.key_down[i]);
    assert(!input.keyboard.key_pressed[i]);
    assert(!input.keyboard.key_released[i]);
  }

  assert(!input.keyboard.shift_down);
  assert(!input.keyboard.ctrl_down);
  assert(!input.keyboard.alt_down);
}

static bool
wpl_test_make_temp_path(char path[64])
{
  int fd = -1;

  strcpy(path, "/tmp/wpl_replay_edge_test_XXXXXX");
  fd = mkstemp(path);
  if (fd < 0)
    return false;

  close(fd);
  unlink(path);
  return true;
}

static void
wpl_test_write_one_frame_replay_file(const char* path,
                                     const WplReplayFrameV1* frame)
{
  uint8_t bytes[WPL_REPLAY_HEADER_SIZE_V1 + WPL_REPLAY_FRAME_SIZE_V1];

  assert(wpl_replay_encode_header_v1(1u, bytes) == WPL_RESULT_OK);
  assert(wpl_replay_encode_frame_v1(frame,
                                    bytes + WPL_REPLAY_HEADER_SIZE_V1)
         == WPL_RESULT_OK);
  assert(wpl_write_entire_file(path, bytes, sizeof(bytes)) == WPL_RESULT_OK);
}

static void
wpl_test_write_two_frame_replay_file(const char* path,
                                     const WplReplayFrameV1 frames[2])
{
  uint8_t bytes[WPL_REPLAY_HEADER_SIZE_V1 + (2u * WPL_REPLAY_FRAME_SIZE_V1)];

  assert(wpl_replay_encode_header_v1(2u, bytes) == WPL_RESULT_OK);
  assert(wpl_replay_encode_frame_v1(&frames[0],
                                    bytes + WPL_REPLAY_HEADER_SIZE_V1)
         == WPL_RESULT_OK);
  assert(wpl_replay_encode_frame_v1(
           &frames[1],
           bytes + WPL_REPLAY_HEADER_SIZE_V1 + WPL_REPLAY_FRAME_SIZE_V1)
         == WPL_RESULT_OK);
  assert(wpl_write_entire_file(path, bytes, sizeof(bytes)) == WPL_RESULT_OK);
}

static void
wpl_test_write_trailing_byte_replay_file(const char* path)
{
  uint8_t bytes[WPL_REPLAY_HEADER_SIZE_V1 + 1u];

  assert(wpl_replay_encode_header_v1(0u, bytes) == WPL_RESULT_OK);
  bytes[WPL_REPLAY_HEADER_SIZE_V1] = 0u;
  assert(wpl_write_entire_file(path, bytes, sizeof(bytes)) == WPL_RESULT_OK);
}

static void
test_player_next_invalid_resets_outputs(void)
{
  WplReplayPlayer* player = NULL;
  WplInputState input = wpl_test_input(9u);
  float delta = 123.0f;
  bool has_frame = true;

  assert(wpl_replay_player_create(&player) == WPL_RESULT_OK);
  assert(wpl_replay_player_next(player, &input, &delta, &has_frame)
         == WPL_RESULT_INVALID_ARGUMENT);
  wpl_test_assert_input_zeroed(input);
  assert(delta == 0.0f);
  assert(!has_frame);

  input = wpl_test_input(8u);
  delta = 123.0f;
  has_frame = true;
  assert(wpl_replay_player_next(NULL, &input, &delta, &has_frame)
         == WPL_RESULT_INVALID_ARGUMENT);
  wpl_test_assert_input_zeroed(input);
  assert(delta == 0.0f);
  assert(!has_frame);

  wpl_replay_player_destroy(player);
}

static void
test_recorder_zero_delta_round_trips(void)
{
  WplReplayRecorder* recorder = NULL;
  WplReplayPlayer* player = NULL;
  WplInputState input = wpl_test_input(1u);
  WplInputState played;
  float delta = 1.0f;
  bool has_frame = false;
  char path[64];

  assert(wpl_test_make_temp_path(path));
  assert(wpl_replay_recorder_create(&recorder) == WPL_RESULT_OK);
  assert(wpl_replay_recorder_begin(recorder) == WPL_RESULT_OK);
  assert(wpl_replay_recorder_record_frame(recorder, &input, 0.0f)
         == WPL_RESULT_OK);
  assert(wpl_replay_recorder_save(recorder, path) == WPL_RESULT_OK);

  assert(wpl_replay_player_create(&player) == WPL_RESULT_OK);
  assert(wpl_replay_player_load(player, path) == WPL_RESULT_OK);
  assert(wpl_replay_player_next(player, &played, &delta, &has_frame)
         == WPL_RESULT_OK);
  assert(has_frame);
  assert(delta == 0.0f);
  wpl_test_assert_input_equal(played, input);
  assert(wpl_replay_player_next(player, &played, &delta, &has_frame)
         == WPL_RESULT_OK);
  assert(!has_frame);

  unlink(path);
  wpl_replay_player_destroy(player);
  wpl_replay_recorder_destroy(recorder);
}

static void
test_trailing_byte_load_rejected_and_preserves_previous_replay(void)
{
  WplReplayPlayer* player = NULL;
  WplReplayFrameV1 frame;
  WplInputState input;
  float delta = 0.0f;
  bool has_frame = false;
  char valid_path[64];
  char bad_path[64];

  memset(&frame, 0, sizeof(frame));
  frame.frame_index = 0u;
  frame.delta_microseconds = 12000u;
  frame.time_microseconds = 12000u;
  frame.input = wpl_test_input(2u);

  assert(wpl_test_make_temp_path(valid_path));
  assert(wpl_test_make_temp_path(bad_path));
  wpl_test_write_one_frame_replay_file(valid_path, &frame);
  wpl_test_write_trailing_byte_replay_file(bad_path);

  assert(wpl_replay_player_create(&player) == WPL_RESULT_OK);
  assert(wpl_replay_player_load(player, valid_path) == WPL_RESULT_OK);
  assert(wpl_replay_player_load(player, bad_path) == WPL_RESULT_PARSE_ERROR);
  assert(wpl_replay_player_next(player, &input, &delta, &has_frame)
         == WPL_RESULT_OK);
  assert(has_frame);
  assert(wpl_test_float_equal(delta, 0.012f));
  wpl_test_assert_input_equal(input, frame.input);

  unlink(valid_path);
  unlink(bad_path);
  wpl_replay_player_destroy(player);
}

static void
test_failed_load_preserves_cursor_position(void)
{
  WplReplayPlayer* player = NULL;
  WplReplayFrameV1 frames[2];
  WplInputState input;
  float delta = 0.0f;
  bool has_frame = false;
  char valid_path[64];
  char bad_path[64];

  memset(frames, 0, sizeof(frames));
  frames[0].frame_index = 0u;
  frames[0].delta_microseconds = 10000u;
  frames[0].time_microseconds = 10000u;
  frames[0].input = wpl_test_input(3u);
  frames[1].frame_index = 1u;
  frames[1].delta_microseconds = 25000u;
  frames[1].time_microseconds = 35000u;
  frames[1].input = wpl_test_input(4u);

  assert(wpl_test_make_temp_path(valid_path));
  assert(wpl_test_make_temp_path(bad_path));
  wpl_test_write_two_frame_replay_file(valid_path, frames);
  wpl_test_write_trailing_byte_replay_file(bad_path);

  assert(wpl_replay_player_create(&player) == WPL_RESULT_OK);
  assert(wpl_replay_player_load(player, valid_path) == WPL_RESULT_OK);
  assert(wpl_replay_player_next(player, &input, &delta, &has_frame)
         == WPL_RESULT_OK);
  assert(has_frame);
  wpl_test_assert_input_equal(input, frames[0].input);

  assert(wpl_replay_player_load(player, bad_path) == WPL_RESULT_PARSE_ERROR);
  assert(wpl_replay_player_next(player, &input, &delta, &has_frame)
         == WPL_RESULT_OK);
  assert(has_frame);
  assert(wpl_test_float_equal(delta, 0.025f));
  wpl_test_assert_input_equal(input, frames[1].input);

  unlink(valid_path);
  unlink(bad_path);
  wpl_replay_player_destroy(player);
}

int
main(void)
{
  test_player_next_invalid_resets_outputs();
  test_recorder_zero_delta_round_trips();
  test_trailing_byte_load_rejected_and_preserves_previous_replay();
  test_failed_load_preserves_cursor_position();

  return 0;
}
