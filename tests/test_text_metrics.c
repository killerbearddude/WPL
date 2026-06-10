/* test_text_metrics.c - Public ASCII text measurement behavior tests. */

#include <wpl/wpl.h>

#include <assert.h>
#include <math.h>
#include <stdint.h>

static int
wpl_test_float_eq(float a, float b)
{
  return fabsf(a - b) < 0.0001f;
}

static void
test_constants_are_positive(void)
{
  assert(wpl_text_glyph_advance_x() > 0.0f);
  assert(wpl_text_line_height() > 0.0f);
}

static void
test_null_validation(void)
{
  WplTextMetrics metrics = {1.0f, 2.0f, 3.0f};

  assert(wpl_measure_text(NULL, &metrics) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_test_float_eq(metrics.width, 0.0f));
  assert(wpl_test_float_eq(metrics.height, 0.0f));
  assert(wpl_test_float_eq(metrics.line_height, 0.0f));

  assert(wpl_measure_text("text", NULL) == WPL_RESULT_INVALID_ARGUMENT);
}

static WplTextMetrics
wpl_test_measure_ok(const char* text)
{
  WplTextMetrics metrics = {0};
  assert(wpl_measure_text(text, &metrics) == WPL_RESULT_OK);
  assert(wpl_test_float_eq(metrics.line_height, wpl_text_line_height()));
  return metrics;
}

static void
test_empty_string(void)
{
  WplTextMetrics metrics = wpl_test_measure_ok("");
  assert(wpl_test_float_eq(metrics.width, 0.0f));
  assert(wpl_test_float_eq(metrics.height, wpl_text_line_height()));
}

static void
test_single_character_and_word(void)
{
  WplTextMetrics metrics = wpl_test_measure_ok("A");
  assert(wpl_test_float_eq(metrics.width, wpl_text_glyph_advance_x()));
  assert(wpl_test_float_eq(metrics.height, wpl_text_line_height()));

  metrics = wpl_test_measure_ok("WPL");
  assert(wpl_test_float_eq(metrics.width,
                           3.0f * wpl_text_glyph_advance_x()));
  assert(wpl_test_float_eq(metrics.height, wpl_text_line_height()));
}

static void
test_space_and_unsupported_byte_contribute_one_advance(void)
{
  char text[2];
  WplTextMetrics metrics = wpl_test_measure_ok(" ");
  assert(wpl_test_float_eq(metrics.width, wpl_text_glyph_advance_x()));

  text[0] = (char)0x01;
  text[1] = '\0';
  metrics = wpl_test_measure_ok(text);
  assert(wpl_test_float_eq(metrics.width, wpl_text_glyph_advance_x()));
}

static void
test_newline_and_trailing_newline(void)
{
  WplTextMetrics metrics = wpl_test_measure_ok("abc\nde");
  assert(wpl_test_float_eq(metrics.width,
                           3.0f * wpl_text_glyph_advance_x()));
  assert(wpl_test_float_eq(metrics.height,
                           2.0f * wpl_text_line_height()));

  metrics = wpl_test_measure_ok("abc\n");
  assert(wpl_test_float_eq(metrics.width,
                           3.0f * wpl_text_glyph_advance_x()));
  assert(wpl_test_float_eq(metrics.height,
                           2.0f * wpl_text_line_height()));
}

static void
test_carriage_return_is_ignored(void)
{
  WplTextMetrics metrics = wpl_test_measure_ok("a\rb");
  assert(wpl_test_float_eq(metrics.width,
                           2.0f * wpl_text_glyph_advance_x()));
  assert(wpl_test_float_eq(metrics.height, wpl_text_line_height()));
}

static void
test_tab_advances_four_spaces(void)
{
  WplTextMetrics metrics = wpl_test_measure_ok("a\tb");
  assert(wpl_test_float_eq(metrics.width,
                           6.0f * wpl_text_glyph_advance_x()));
  assert(wpl_test_float_eq(metrics.height, wpl_text_line_height()));
}

static void
test_multiple_lines_use_max_line_width(void)
{
  WplTextMetrics metrics = wpl_test_measure_ok("a\nabcde\nxy");
  assert(wpl_test_float_eq(metrics.width,
                           5.0f * wpl_text_glyph_advance_x()));
  assert(wpl_test_float_eq(metrics.height,
                           3.0f * wpl_text_line_height()));
}

static void
test_measurement_is_not_limited_to_draw_command_bytes(void)
{
  char text[WPL_DRAW_TEXT_MAX_BYTES + 3u];
  size_t i;
  WplTextMetrics metrics;

  for (i = 0u; i < WPL_DRAW_TEXT_MAX_BYTES + 1u; i++)
    text[i] = 'a';
  text[WPL_DRAW_TEXT_MAX_BYTES + 1u] = '\0';

  assert(wpl_measure_text(text, &metrics) == WPL_RESULT_OK);
  assert(wpl_test_float_eq(metrics.width,
                           (float)(WPL_DRAW_TEXT_MAX_BYTES + 1u)
                           * wpl_text_glyph_advance_x()));
}

int
main(void)
{
  test_constants_are_positive();
  test_null_validation();
  test_empty_string();
  test_single_character_and_word();
  test_space_and_unsupported_byte_contribute_one_advance();
  test_newline_and_trailing_newline();
  test_carriage_return_is_ignored();
  test_tab_advances_four_spaces();
  test_multiple_lines_use_max_line_width();
  test_measurement_is_not_limited_to_draw_command_bytes();
  return 0;
}
