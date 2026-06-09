/* test_replay_format.c - Replay v1 binary format tests. */

#include "wpl_replay_internal.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define WPL_TEST_HEADER_MAGIC_OFFSET 0u
#define WPL_TEST_HEADER_VERSION_OFFSET 4u
#define WPL_TEST_HEADER_SIZE_OFFSET 8u
#define WPL_TEST_HEADER_FRAME_SIZE_OFFSET 12u
#define WPL_TEST_HEADER_FRAME_COUNT_OFFSET 16u
#define WPL_TEST_HEADER_FLAGS_OFFSET 24u
#define WPL_TEST_HEADER_RESERVED_OFFSET 28u

#define WPL_TEST_FRAME_INDEX_OFFSET 0u
#define WPL_TEST_FRAME_TIME_OFFSET 8u
#define WPL_TEST_FRAME_DELTA_OFFSET 16u
#define WPL_TEST_FRAME_MOUSE_X_OFFSET 20u
#define WPL_TEST_FRAME_MOUSE_Y_OFFSET 24u
#define WPL_TEST_FRAME_MOUSE_DX_OFFSET 28u
#define WPL_TEST_FRAME_MOUSE_DY_OFFSET 32u
#define WPL_TEST_FRAME_WHEEL_OFFSET 36u
#define WPL_TEST_FRAME_MOUSE_DOWN_OFFSET 40u
#define WPL_TEST_FRAME_MOUSE_PRESSED_OFFSET 43u
#define WPL_TEST_FRAME_MOUSE_RELEASED_OFFSET 46u
#define WPL_TEST_FRAME_KEY_DOWN_OFFSET 49u
#define WPL_TEST_FRAME_KEY_PRESSED_OFFSET 86u
#define WPL_TEST_FRAME_KEY_RELEASED_OFFSET 123u
#define WPL_TEST_FRAME_SHIFT_OFFSET 160u
#define WPL_TEST_FRAME_CTRL_OFFSET 161u
#define WPL_TEST_FRAME_ALT_OFFSET 162u
#define WPL_TEST_FRAME_RESERVED_OFFSET 163u

static uint32_t
wpl_test_read_u32_le(const uint8_t* data)
{
  return ((uint32_t)data[0])
         | ((uint32_t)data[1] << 8)
         | ((uint32_t)data[2] << 16)
         | ((uint32_t)data[3] << 24);
}

static uint64_t
wpl_test_read_u64_le(const uint8_t* data)
{
  return ((uint64_t)data[0])
         | ((uint64_t)data[1] << 8)
         | ((uint64_t)data[2] << 16)
         | ((uint64_t)data[3] << 24)
         | ((uint64_t)data[4] << 32)
         | ((uint64_t)data[5] << 40)
         | ((uint64_t)data[6] << 48)
         | ((uint64_t)data[7] << 56);
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
wpl_test_write_u64_le(uint8_t* data, uint64_t value)
{
  data[0] = (uint8_t)(value & 0xffu);
  data[1] = (uint8_t)((value >> 8) & 0xffu);
  data[2] = (uint8_t)((value >> 16) & 0xffu);
  data[3] = (uint8_t)((value >> 24) & 0xffu);
  data[4] = (uint8_t)((value >> 32) & 0xffu);
  data[5] = (uint8_t)((value >> 40) & 0xffu);
  data[6] = (uint8_t)((value >> 48) & 0xffu);
  data[7] = (uint8_t)((value >> 56) & 0xffu);
}

static void
wpl_test_write_f32_bits_le(uint8_t* data, uint32_t bits)
{
  wpl_test_write_u32_le(data, bits);
}

static WplReplayFrameV1
wpl_test_frame(void)
{
  WplReplayFrameV1 frame;

  memset(&frame, 0, sizeof(frame));
  frame.frame_index = 7u;
  frame.time_microseconds = 123456u;
  frame.delta_microseconds = 16667u;
  frame.input.mouse.position.x = 10.5f;
  frame.input.mouse.position.y = -2.25f;
  frame.input.mouse.delta.x = 3.0f;
  frame.input.mouse.delta.y = -4.0f;
  frame.input.mouse.wheel_delta = 1.0f;
  frame.input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT] = true;
  frame.input.mouse.button_pressed[WPL_MOUSE_BUTTON_RIGHT] = true;
  frame.input.mouse.button_released[WPL_MOUSE_BUTTON_MIDDLE] = true;
  frame.input.keyboard.key_down[WPL_KEY_A] = true;
  frame.input.keyboard.key_pressed[WPL_KEY_ESCAPE] = true;
  frame.input.keyboard.key_released[WPL_KEY_Z] = true;
  frame.input.keyboard.shift_down = true;
  frame.input.keyboard.ctrl_down = true;
  frame.input.keyboard.alt_down = false;

  return frame;
}

static int
wpl_test_float_equal(float a, float b)
{
  float d = a - b;

  if (d < 0.0f)
    d = -d;

  return d < 0.0001f;
}

static void
wpl_test_make_valid_header(uint8_t header[WPL_REPLAY_HEADER_SIZE_V1])
{
  assert(wpl_replay_encode_header_v1(3u, header) == WPL_RESULT_OK);
}

static void
wpl_test_make_valid_frame(uint8_t data[WPL_REPLAY_FRAME_SIZE_V1])
{
  WplReplayFrameV1 frame = wpl_test_frame();

  assert(wpl_replay_encode_frame_v1(&frame, data) == WPL_RESULT_OK);
}

static void
test_header_encode_writes_expected_fields(void)
{
  uint8_t header[WPL_REPLAY_HEADER_SIZE_V1] = {0};
  uint64_t frame_count = 0x0000000000054321ull;

  assert(wpl_replay_encode_header_v1(frame_count, header) == WPL_RESULT_OK);
  assert(memcmp(&header[WPL_TEST_HEADER_MAGIC_OFFSET], WPL_REPLAY_MAGIC, 4u)
         == 0);
  assert(wpl_test_read_u32_le(&header[WPL_TEST_HEADER_VERSION_OFFSET])
         == WPL_REPLAY_VERSION);
  assert(wpl_test_read_u32_le(&header[WPL_TEST_HEADER_SIZE_OFFSET])
         == WPL_REPLAY_HEADER_SIZE_V1);
  assert(wpl_test_read_u32_le(&header[WPL_TEST_HEADER_FRAME_SIZE_OFFSET])
         == WPL_REPLAY_FRAME_SIZE_V1);
  assert(wpl_test_read_u64_le(&header[WPL_TEST_HEADER_FRAME_COUNT_OFFSET])
         == frame_count);
  assert(wpl_test_read_u32_le(&header[WPL_TEST_HEADER_FLAGS_OFFSET]) == 0u);
  assert(wpl_test_read_u32_le(&header[WPL_TEST_HEADER_RESERVED_OFFSET]) == 0u);
}

static void
test_header_encode_rejects_invalid_arguments_and_too_many_frames(void)
{
  uint8_t header[WPL_REPLAY_HEADER_SIZE_V1] = {0};

  assert(wpl_replay_encode_header_v1(0u, NULL) ==
         WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_encode_header_v1(WPL_REPLAY_MAX_FRAMES_V1 + 1u,
                                     header) == WPL_RESULT_UNSUPPORTED);
}

static void
test_header_decode_accepts_valid_header(void)
{
  uint8_t header[WPL_REPLAY_HEADER_SIZE_V1] = {0};
  WplReplayHeaderV1 decoded = {0};

  assert(wpl_replay_encode_header_v1(99u, header) == WPL_RESULT_OK);
  assert(wpl_replay_decode_header_v1(header, sizeof(header), &decoded) ==
         WPL_RESULT_OK);
  assert(decoded.frame_count == 99u);
}

static void
test_header_decode_rejects_invalid_inputs(void)
{
  uint8_t header[WPL_REPLAY_HEADER_SIZE_V1] = {0};
  WplReplayHeaderV1 decoded = {123u};

  wpl_test_make_valid_header(header);
  assert(wpl_replay_decode_header_v1(NULL, sizeof(header), &decoded) ==
         WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_decode_header_v1(header, sizeof(header), NULL) ==
         WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_decode_header_v1(header,
                                     WPL_REPLAY_HEADER_SIZE_V1 - 1u,
                                     &decoded) == WPL_RESULT_PARSE_ERROR);
}

static void
test_header_decode_rejects_malformed_fields(void)
{
  uint8_t header[WPL_REPLAY_HEADER_SIZE_V1] = {0};
  WplReplayHeaderV1 decoded = {0};

  wpl_test_make_valid_header(header);
  header[0] = 'X';
  assert(wpl_replay_decode_header_v1(header, sizeof(header), &decoded) ==
         WPL_RESULT_PARSE_ERROR);

  wpl_test_make_valid_header(header);
  wpl_test_write_u32_le(&header[WPL_TEST_HEADER_VERSION_OFFSET], 999u);
  assert(wpl_replay_decode_header_v1(header, sizeof(header), &decoded) ==
         WPL_RESULT_UNSUPPORTED);

  wpl_test_make_valid_header(header);
  wpl_test_write_u32_le(&header[WPL_TEST_HEADER_SIZE_OFFSET], 31u);
  assert(wpl_replay_decode_header_v1(header, sizeof(header), &decoded) ==
         WPL_RESULT_PARSE_ERROR);

  wpl_test_make_valid_header(header);
  wpl_test_write_u32_le(&header[WPL_TEST_HEADER_FRAME_SIZE_OFFSET], 191u);
  assert(wpl_replay_decode_header_v1(header, sizeof(header), &decoded) ==
         WPL_RESULT_PARSE_ERROR);

  wpl_test_make_valid_header(header);
  wpl_test_write_u32_le(&header[WPL_TEST_HEADER_FLAGS_OFFSET], 1u);
  assert(wpl_replay_decode_header_v1(header, sizeof(header), &decoded) ==
         WPL_RESULT_PARSE_ERROR);

  wpl_test_make_valid_header(header);
  wpl_test_write_u32_le(&header[WPL_TEST_HEADER_RESERVED_OFFSET], 1u);
  assert(wpl_replay_decode_header_v1(header, sizeof(header), &decoded) ==
         WPL_RESULT_PARSE_ERROR);

  wpl_test_make_valid_header(header);
  wpl_test_write_u64_le(&header[WPL_TEST_HEADER_FRAME_COUNT_OFFSET],
                        WPL_REPLAY_MAX_FRAMES_V1 + 1u);
  assert(wpl_replay_decode_header_v1(header, sizeof(header), &decoded) ==
         WPL_RESULT_UNSUPPORTED);
}

static void
test_frame_encode_writes_fixed_fields_and_zero_reserved(void)
{
  WplReplayFrameV1 frame = wpl_test_frame();
  uint8_t data[WPL_REPLAY_FRAME_SIZE_V1] = {0};
  size_t i = 0u;

  assert(wpl_replay_encode_frame_v1(&frame, data) == WPL_RESULT_OK);
  assert(wpl_test_read_u64_le(&data[WPL_TEST_FRAME_INDEX_OFFSET]) ==
         frame.frame_index);
  assert(wpl_test_read_u64_le(&data[WPL_TEST_FRAME_TIME_OFFSET]) ==
         frame.time_microseconds);
  assert(wpl_test_read_u32_le(&data[WPL_TEST_FRAME_DELTA_OFFSET]) ==
         frame.delta_microseconds);
  assert(data[WPL_TEST_FRAME_MOUSE_DOWN_OFFSET + WPL_MOUSE_BUTTON_LEFT] == 1u);
  assert(data[WPL_TEST_FRAME_MOUSE_PRESSED_OFFSET + WPL_MOUSE_BUTTON_RIGHT] ==
         1u);
  assert(data[WPL_TEST_FRAME_MOUSE_RELEASED_OFFSET + WPL_MOUSE_BUTTON_MIDDLE] ==
         1u);
  assert(data[WPL_TEST_FRAME_KEY_DOWN_OFFSET + WPL_KEY_A] == 1u);
  assert(data[WPL_TEST_FRAME_KEY_PRESSED_OFFSET + WPL_KEY_ESCAPE] == 1u);
  assert(data[WPL_TEST_FRAME_KEY_RELEASED_OFFSET + WPL_KEY_Z] == 1u);
  assert(data[WPL_TEST_FRAME_SHIFT_OFFSET] == 1u);
  assert(data[WPL_TEST_FRAME_CTRL_OFFSET] == 1u);
  assert(data[WPL_TEST_FRAME_ALT_OFFSET] == 0u);

  for (i = 0u; i < 29u; i++)
    assert(data[WPL_TEST_FRAME_RESERVED_OFFSET + i] == 0u);
}

static void
test_frame_encode_rejects_invalid_arguments_and_nonfinite_floats(void)
{
  WplReplayFrameV1 frame = wpl_test_frame();
  uint8_t data[WPL_REPLAY_FRAME_SIZE_V1] = {0};

  assert(wpl_replay_encode_frame_v1(NULL, data) ==
         WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_encode_frame_v1(&frame, NULL) ==
         WPL_RESULT_INVALID_ARGUMENT);

  {
    uint32_t nan_bits = 0x7fc00000u;
    memcpy(&frame.input.mouse.position.x, &nan_bits, sizeof(nan_bits));
  }
  assert(wpl_replay_encode_frame_v1(&frame, data) ==
         WPL_RESULT_INVALID_ARGUMENT);
}

static void
test_frame_decode_accepts_valid_frame_and_round_trips_input(void)
{
  WplReplayFrameV1 frame = wpl_test_frame();
  WplReplayFrameV1 decoded;
  uint8_t data[WPL_REPLAY_FRAME_SIZE_V1] = {0};

  assert(wpl_replay_encode_frame_v1(&frame, data) == WPL_RESULT_OK);
  assert(wpl_replay_decode_frame_v1(data, sizeof(data), &decoded) ==
         WPL_RESULT_OK);

  assert(decoded.frame_index == frame.frame_index);
  assert(decoded.time_microseconds == frame.time_microseconds);
  assert(decoded.delta_microseconds == frame.delta_microseconds);
  assert(wpl_test_float_equal(decoded.input.mouse.position.x,
                              frame.input.mouse.position.x));
  assert(wpl_test_float_equal(decoded.input.mouse.position.y,
                              frame.input.mouse.position.y));
  assert(wpl_test_float_equal(decoded.input.mouse.delta.x,
                              frame.input.mouse.delta.x));
  assert(wpl_test_float_equal(decoded.input.mouse.delta.y,
                              frame.input.mouse.delta.y));
  assert(wpl_test_float_equal(decoded.input.mouse.wheel_delta,
                              frame.input.mouse.wheel_delta));
  assert(decoded.input.mouse.button_down[WPL_MOUSE_BUTTON_LEFT]);
  assert(decoded.input.mouse.button_pressed[WPL_MOUSE_BUTTON_RIGHT]);
  assert(decoded.input.mouse.button_released[WPL_MOUSE_BUTTON_MIDDLE]);
  assert(decoded.input.keyboard.key_down[WPL_KEY_A]);
  assert(decoded.input.keyboard.key_pressed[WPL_KEY_ESCAPE]);
  assert(decoded.input.keyboard.key_released[WPL_KEY_Z]);
  assert(decoded.input.keyboard.shift_down);
  assert(decoded.input.keyboard.ctrl_down);
  assert(!decoded.input.keyboard.alt_down);
}

static void
test_frame_decode_rejects_invalid_inputs_and_short_buffer(void)
{
  uint8_t data[WPL_REPLAY_FRAME_SIZE_V1] = {0};
  WplReplayFrameV1 decoded;

  wpl_test_make_valid_frame(data);
  assert(wpl_replay_decode_frame_v1(NULL, sizeof(data), &decoded) ==
         WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_decode_frame_v1(data, sizeof(data), NULL) ==
         WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_replay_decode_frame_v1(data, WPL_REPLAY_FRAME_SIZE_V1 - 1u,
                                    &decoded) == WPL_RESULT_PARSE_ERROR);
}

static void
test_frame_decode_rejects_malformed_booleans(void)
{
  uint8_t data[WPL_REPLAY_FRAME_SIZE_V1] = {0};
  WplReplayFrameV1 decoded;

  wpl_test_make_valid_frame(data);
  data[WPL_TEST_FRAME_MOUSE_DOWN_OFFSET] = 2u;
  assert(wpl_replay_decode_frame_v1(data, sizeof(data), &decoded) ==
         WPL_RESULT_PARSE_ERROR);

  wpl_test_make_valid_frame(data);
  data[WPL_TEST_FRAME_KEY_DOWN_OFFSET + WPL_KEY_A] = 2u;
  assert(wpl_replay_decode_frame_v1(data, sizeof(data), &decoded) ==
         WPL_RESULT_PARSE_ERROR);

  wpl_test_make_valid_frame(data);
  data[WPL_TEST_FRAME_SHIFT_OFFSET] = 2u;
  assert(wpl_replay_decode_frame_v1(data, sizeof(data), &decoded) ==
         WPL_RESULT_PARSE_ERROR);
}

static void
test_frame_decode_rejects_nonzero_reserved_and_nonfinite_float(void)
{
  uint8_t data[WPL_REPLAY_FRAME_SIZE_V1] = {0};
  WplReplayFrameV1 decoded;

  wpl_test_make_valid_frame(data);
  data[WPL_TEST_FRAME_RESERVED_OFFSET] = 1u;
  assert(wpl_replay_decode_frame_v1(data, sizeof(data), &decoded) ==
         WPL_RESULT_PARSE_ERROR);

  wpl_test_make_valid_frame(data);
  wpl_test_write_f32_bits_le(&data[WPL_TEST_FRAME_MOUSE_X_OFFSET], 0x7fc00000u);
  assert(wpl_replay_decode_frame_v1(data, sizeof(data), &decoded) ==
         WPL_RESULT_PARSE_ERROR);

  wpl_test_make_valid_frame(data);
  wpl_test_write_f32_bits_le(&data[WPL_TEST_FRAME_WHEEL_OFFSET], 0x7f800000u);
  assert(wpl_replay_decode_frame_v1(data, sizeof(data), &decoded) ==
         WPL_RESULT_PARSE_ERROR);
}

static void
test_file_size_validation(void)
{
  size_t exact_size = (size_t)WPL_REPLAY_HEADER_SIZE_V1
                      + (size_t)(3u * WPL_REPLAY_FRAME_SIZE_V1);

  assert(wpl_replay_validate_file_size_v1(3u, exact_size) == WPL_RESULT_OK);
  assert(wpl_replay_validate_file_size_v1(3u, exact_size - 1u) ==
         WPL_RESULT_PARSE_ERROR);
  assert(wpl_replay_validate_file_size_v1(WPL_REPLAY_MAX_FRAMES_V1 + 1u,
                                          exact_size) == WPL_RESULT_UNSUPPORTED);
  assert(wpl_replay_validate_file_size_v1(
           0u,
           (size_t)WPL_MAX_FILE_SIZE_V0_1 + 1u) == WPL_RESULT_UNSUPPORTED);
}

int
main(void)
{
  test_header_encode_writes_expected_fields();
  test_header_encode_rejects_invalid_arguments_and_too_many_frames();
  test_header_decode_accepts_valid_header();
  test_header_decode_rejects_invalid_inputs();
  test_header_decode_rejects_malformed_fields();
  test_frame_encode_writes_fixed_fields_and_zero_reserved();
  test_frame_encode_rejects_invalid_arguments_and_nonfinite_floats();
  test_frame_decode_accepts_valid_frame_and_round_trips_input();
  test_frame_decode_rejects_invalid_inputs_and_short_buffer();
  test_frame_decode_rejects_malformed_booleans();
  test_frame_decode_rejects_nonzero_reserved_and_nonfinite_float();
  test_file_size_validation();

  return 0;
}
