/* test_draw_list.c - Public draw-list behavior tests. */

#include <wpl/wpl.h>

#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <string.h>

static WplColor
wpl_test_color(void)
{
  WplColor color = {0.1f, 0.2f, 0.3f, 1.0f};
  return color;
}

static WplPanelStyle
wpl_test_panel_style(void)
{
  WplPanelStyle style;
  style.fill_color = wpl_test_color();
  style.border_color = (WplColor){0.8f, 0.7f, 0.6f, 1.0f};
  style.border_thickness = 1.0f;
  style.corner_radius = 3.0f;
  return style;
}

static WplRect
wpl_test_rect(void)
{
  WplRect rect = {1.0f, 2.0f, 3.0f, 4.0f};
  return rect;
}

static WplVec2
wpl_test_vec2(float x, float y)
{
  WplVec2 value = {x, y};
  return value;
}

static WplDrawList*
wpl_test_create_list(size_t capacity)
{
  WplDrawList* list = NULL;
  WplResult result = wpl_create_draw_list(capacity, &list);

  assert(result == WPL_RESULT_OK);
  assert(list != NULL);
  return list;
}

static void
test_create_validation(void)
{
  WplDrawList* list = (WplDrawList*)1;
  WplResult result;

  result = wpl_create_draw_list(1u, NULL);
  assert(result == WPL_RESULT_INVALID_ARGUMENT);

  result = wpl_create_draw_list(0u, &list);
  assert(result == WPL_RESULT_INVALID_ARGUMENT);
  assert(list == NULL);

  result = wpl_create_draw_list(4u, &list);
  assert(result == WPL_RESULT_OK);
  assert(list != NULL);
  assert(wpl_draw_list_count(list) == 0u);
  assert(wpl_draw_list_capacity(list) == 4u);

  wpl_destroy_draw_list(list);
  wpl_destroy_draw_list(NULL);
}

static void
test_null_accessors_and_clear(void)
{
  assert(wpl_draw_list_count(NULL) == 0u);
  assert(wpl_draw_list_capacity(NULL) == 0u);
  assert(wpl_draw_list_clear(NULL) == WPL_RESULT_INVALID_ARGUMENT);
}

static void
test_clear_resets_count_preserves_capacity(void)
{
  WplDrawList* list = wpl_test_create_list(3u);

  assert(wpl_draw_rect(list, wpl_test_rect(), wpl_test_color())
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);
  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 0u);
  assert(wpl_draw_list_capacity(list) == 3u);

  wpl_destroy_draw_list(list);
}

static void
test_append_increments_count(void)
{
  WplDrawList* list = wpl_test_create_list(8u);

  assert(wpl_draw_clear(list, wpl_test_color()) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);

  assert(wpl_draw_rect(list, wpl_test_rect(), wpl_test_color())
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 2u);

  assert(wpl_draw_rect_outline(list,
                               wpl_test_rect(),
                               wpl_test_color(),
                               1.0f)
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 3u);

  assert(wpl_draw_line(list,
                       wpl_test_vec2(0.0f, 0.0f),
                       wpl_test_vec2(1.0f, 1.0f),
                       wpl_test_color(),
                       1.0f)
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 4u);

  assert(wpl_draw_circle(list,
                         wpl_test_vec2(2.0f, 2.0f),
                         3.0f,
                         wpl_test_color())
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 5u);

  assert(wpl_draw_text(list,
                       wpl_test_vec2(4.0f, 4.0f),
                       "draw text",
                       wpl_test_color())
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 6u);

  wpl_destroy_draw_list(list);
}

static void
test_capacity_failure_preserves_count(void)
{
  WplDrawList* list = wpl_test_create_list(1u);

  assert(wpl_draw_clear(list, wpl_test_color()) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);

  assert(wpl_draw_rect(list, wpl_test_rect(), wpl_test_color())
         == WPL_RESULT_CAPACITY_EXCEEDED);
  assert(wpl_draw_list_count(list) == 1u);

  wpl_destroy_draw_list(list);
}

static void
test_null_list_appends_are_invalid(void)
{
  assert(wpl_draw_clear(NULL, wpl_test_color())
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_rect(NULL, wpl_test_rect(), wpl_test_color())
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_rounded_rect(NULL, wpl_test_rect(), 1.0f, wpl_test_color())
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_panel(NULL, wpl_test_rect(), wpl_test_panel_style())
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_rect_outline(NULL, wpl_test_rect(), wpl_test_color(), 1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_line(NULL,
                       wpl_test_vec2(0.0f, 0.0f),
                       wpl_test_vec2(1.0f, 1.0f),
                       wpl_test_color(),
                       1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_circle(NULL, wpl_test_vec2(0.0f, 0.0f), 1.0f,
                         wpl_test_color())
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_text(NULL, wpl_test_vec2(0.0f, 0.0f), "text",
                       wpl_test_color())
         == WPL_RESULT_INVALID_ARGUMENT);
}

static void
test_invalid_geometry_preserves_count(void)
{
  WplDrawList* list = wpl_test_create_list(8u);
  WplRect rect = wpl_test_rect();

  rect.w = -1.0f;
  assert(wpl_draw_rect(list, rect, wpl_test_color())
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  rect = wpl_test_rect();
  rect.h = -1.0f;
  assert(wpl_draw_rect(list, rect, wpl_test_color())
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_rect_outline(list, wpl_test_rect(), wpl_test_color(), -1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_line(list,
                       wpl_test_vec2(0.0f, 0.0f),
                       wpl_test_vec2(1.0f, 1.0f),
                       wpl_test_color(),
                       -1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_circle(list, wpl_test_vec2(0.0f, 0.0f), -1.0f,
                         wpl_test_color())
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  wpl_destroy_draw_list(list);
}

static void
test_nonfinite_values_are_rejected(void)
{
  WplDrawList* list = wpl_test_create_list(8u);
  WplColor color = wpl_test_color();
  WplRect rect = wpl_test_rect();

  color.g = NAN;
  assert(wpl_draw_clear(list, color) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  color = wpl_test_color();
  color.b = INFINITY;
  assert(wpl_draw_clear(list, color) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  rect.x = INFINITY;
  assert(wpl_draw_rect(list, rect, wpl_test_color())
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_line(list,
                       wpl_test_vec2(NAN, 0.0f),
                       wpl_test_vec2(1.0f, 1.0f),
                       wpl_test_color(),
                       1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_text(list,
                       wpl_test_vec2(0.0f, INFINITY),
                       "text",
                       wpl_test_color())
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  wpl_destroy_draw_list(list);
}

static void
test_out_of_range_finite_color_is_accepted(void)
{
  WplDrawList* list = wpl_test_create_list(1u);
  WplColor color = {-1.0f, 2.0f, 0.5f, 4.0f};

  assert(wpl_draw_clear(list, color) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);

  wpl_destroy_draw_list(list);
}

static void
test_text_validation(void)
{
  char accepted_text[WPL_DRAW_TEXT_MAX_BYTES + 1u];
  char rejected_text[WPL_DRAW_TEXT_MAX_BYTES + 2u];
  WplDrawList* list = wpl_test_create_list(2u);

  memset(accepted_text, 'a', sizeof(accepted_text));
  accepted_text[WPL_DRAW_TEXT_MAX_BYTES] = '\0';

  memset(rejected_text, 'b', sizeof(rejected_text));
  rejected_text[WPL_DRAW_TEXT_MAX_BYTES + 1u] = '\0';

  assert(wpl_draw_text(list, wpl_test_vec2(0.0f, 0.0f), NULL,
                       wpl_test_color())
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_text(list, wpl_test_vec2(0.0f, 0.0f), accepted_text,
                       wpl_test_color())
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);

  assert(wpl_draw_text(list, wpl_test_vec2(0.0f, 0.0f), rejected_text,
                       wpl_test_color())
         == WPL_RESULT_TRUNCATED);
  assert(wpl_draw_list_count(list) == 1u);

  wpl_destroy_draw_list(list);
}

static void
test_polyline_validation(void)
{
  WplDrawList* list = wpl_test_create_list(8u);
  WplVec2 points[3] = {
    {0.0f, 0.0f},
    {10.0f, 10.0f},
    {20.0f, 0.0f}
  };
  WplColor color = wpl_test_color();

  assert(wpl_draw_polyline(NULL, points, 3u, color, 1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_polyline(list, NULL, 3u, color, 1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_polyline(list, points, 0u, color, 1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_polyline(list, points, 1u, color, 1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  points[1].x = NAN;
  assert(wpl_draw_polyline(list, points, 3u, color, 1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
  points[1].x = 10.0f;
  assert(wpl_draw_list_count(list) == 0u);

  points[1].y = INFINITY;
  assert(wpl_draw_polyline(list, points, 3u, color, 1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
  points[1].y = 10.0f;
  assert(wpl_draw_list_count(list) == 0u);

  color = wpl_test_color();
  color.r = NAN;
  assert(wpl_draw_polyline(list, points, 3u, color, 1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  color = wpl_test_color();
  color.a = INFINITY;
  assert(wpl_draw_polyline(list, points, 3u, color, 1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_polyline(list, points, 3u, wpl_test_color(), -1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_polyline(list, points, 3u, wpl_test_color(), 0.0f)
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 2u);

  wpl_destroy_draw_list(list);
}

static void
test_polyline_success_counts_and_append_position(void)
{
  WplDrawList* list = wpl_test_create_list(4u);
  WplVec2 two_points[2] = {
    {0.0f, 0.0f},
    {10.0f, 10.0f}
  };
  WplVec2 three_points[3] = {
    {0.0f, 0.0f},
    {10.0f, 10.0f},
    {20.0f, 0.0f}
  };

  assert(wpl_draw_polyline(list,
                           two_points,
                           2u,
                           wpl_test_color(),
                           1.0f)
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);

  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  assert(wpl_draw_polyline(list,
                           three_points,
                           3u,
                           wpl_test_color(),
                           1.0f)
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 2u);

  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  assert(wpl_draw_clear(list, wpl_test_color()) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);
  assert(wpl_draw_polyline(list,
                           three_points,
                           3u,
                           wpl_test_color(),
                           1.0f)
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 3u);

  wpl_destroy_draw_list(list);
}

static void
test_polyline_capacity_failure_preserves_count(void)
{
  WplDrawList* list = wpl_test_create_list(1u);
  WplVec2 points[3] = {
    {0.0f, 0.0f},
    {10.0f, 10.0f},
    {20.0f, 0.0f}
  };
  size_t before = wpl_draw_list_count(list);

  assert(wpl_draw_polyline(list, points, 3u, wpl_test_color(), 1.0f)
         == WPL_RESULT_CAPACITY_EXCEEDED);
  assert(wpl_draw_list_count(list) == before);

  assert(wpl_draw_line(list, points[0], points[1], wpl_test_color(), 1.0f)
         == WPL_RESULT_OK);
  before = wpl_draw_list_count(list);

  assert(wpl_draw_polyline(list, points, 2u, wpl_test_color(), 1.0f)
         == WPL_RESULT_CAPACITY_EXCEEDED);
  assert(wpl_draw_list_count(list) == before);

  wpl_destroy_draw_list(list);
}

static WplDashPattern
wpl_test_dash_pattern(float dash_length, float gap_length)
{
  WplDashPattern pattern;
  pattern.dash_length = dash_length;
  pattern.gap_length = gap_length;
  return pattern;
}

static void
test_dashed_line_validation(void)
{
  WplDrawList* list = wpl_test_create_list(8u);
  WplVec2 a = wpl_test_vec2(0.0f, 0.0f);
  WplVec2 b = wpl_test_vec2(10.0f, 0.0f);
  WplColor color = wpl_test_color();
  WplDashPattern pattern = wpl_test_dash_pattern(2.0f, 2.0f);

  assert(wpl_draw_dashed_line(NULL, a, b, color, 1.0f, pattern)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  a.x = NAN;
  assert(wpl_draw_dashed_line(list, a, b, color, 1.0f, pattern)
         == WPL_RESULT_INVALID_ARGUMENT);
  a.x = 0.0f;
  assert(wpl_draw_list_count(list) == 0u);

  b.y = INFINITY;
  assert(wpl_draw_dashed_line(list, a, b, color, 1.0f, pattern)
         == WPL_RESULT_INVALID_ARGUMENT);
  b.y = 0.0f;
  assert(wpl_draw_list_count(list) == 0u);

  color = wpl_test_color();
  color.g = NAN;
  assert(wpl_draw_dashed_line(list, a, b, color, 1.0f, pattern)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  color = wpl_test_color();
  color.a = INFINITY;
  assert(wpl_draw_dashed_line(list, a, b, color, 1.0f, pattern)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_dashed_line(list,
                              a,
                              b,
                              wpl_test_color(),
                              NAN,
                              pattern)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_dashed_line(list,
                              a,
                              b,
                              wpl_test_color(),
                              INFINITY,
                              pattern)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_dashed_line(list,
                              a,
                              b,
                              wpl_test_color(),
                              -1.0f,
                              pattern)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  wpl_destroy_draw_list(list);
}

static void
test_dashed_line_pattern_validation(void)
{
  WplDrawList* list = wpl_test_create_list(8u);
  WplVec2 a = wpl_test_vec2(0.0f, 0.0f);
  WplVec2 b = wpl_test_vec2(10.0f, 0.0f);
  WplColor color = wpl_test_color();

  assert(wpl_draw_dashed_line(list,
                              a,
                              b,
                              color,
                              1.0f,
                              wpl_test_dash_pattern(0.0f, 2.0f))
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_dashed_line(list,
                              a,
                              b,
                              color,
                              1.0f,
                              wpl_test_dash_pattern(-1.0f, 2.0f))
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_dashed_line(list,
                              a,
                              b,
                              color,
                              1.0f,
                              wpl_test_dash_pattern(2.0f, 0.0f))
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_dashed_line(list,
                              a,
                              b,
                              color,
                              1.0f,
                              wpl_test_dash_pattern(2.0f, -1.0f))
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_dashed_line(list,
                              a,
                              b,
                              color,
                              1.0f,
                              wpl_test_dash_pattern(NAN, 2.0f))
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_dashed_line(list,
                              a,
                              b,
                              color,
                              1.0f,
                              wpl_test_dash_pattern(INFINITY, 2.0f))
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_dashed_line(list,
                              a,
                              b,
                              color,
                              1.0f,
                              wpl_test_dash_pattern(2.0f, NAN))
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_dashed_line(list,
                              a,
                              b,
                              color,
                              1.0f,
                              wpl_test_dash_pattern(2.0f, INFINITY))
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  wpl_destroy_draw_list(list);
}

static void
test_dashed_line_degenerate_line_appends_zero(void)
{
  WplDrawList* list = wpl_test_create_list(2u);
  WplVec2 p = wpl_test_vec2(5.0f, 5.0f);

  assert(wpl_draw_dashed_line(list,
                              p,
                              p,
                              wpl_test_color(),
                              1.0f,
                              wpl_test_dash_pattern(2.0f, 2.0f))
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 0u);

  wpl_destroy_draw_list(list);
}

static void
test_dashed_line_success_counts_and_append_position(void)
{
  WplDrawList* list = wpl_test_create_list(8u);
  WplVec2 a = wpl_test_vec2(0.0f, 0.0f);
  WplVec2 b = wpl_test_vec2(10.0f, 0.0f);

  assert(wpl_draw_dashed_line(list,
                              a,
                              b,
                              wpl_test_color(),
                              1.0f,
                              wpl_test_dash_pattern(20.0f, 5.0f))
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);

  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  assert(wpl_draw_dashed_line(list,
                              a,
                              b,
                              wpl_test_color(),
                              1.0f,
                              wpl_test_dash_pattern(2.0f, 2.0f))
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 3u);

  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  b.x = 9.0f;
  assert(wpl_draw_dashed_line(list,
                              a,
                              b,
                              wpl_test_color(),
                              1.0f,
                              wpl_test_dash_pattern(2.0f, 2.0f))
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 3u);

  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  assert(wpl_draw_clear(list, wpl_test_color()) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);
  b.x = 10.0f;
  assert(wpl_draw_dashed_line(list,
                              a,
                              b,
                              wpl_test_color(),
                              0.0f,
                              wpl_test_dash_pattern(20.0f, 5.0f))
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 2u);

  wpl_destroy_draw_list(list);
}

static void
test_dashed_line_capacity_failure_preserves_count(void)
{
  WplDrawList* list = wpl_test_create_list(2u);
  WplVec2 a = wpl_test_vec2(0.0f, 0.0f);
  WplVec2 b = wpl_test_vec2(10.0f, 0.0f);
  size_t before = wpl_draw_list_count(list);

  assert(wpl_draw_dashed_line(list,
                              a,
                              b,
                              wpl_test_color(),
                              1.0f,
                              wpl_test_dash_pattern(2.0f, 2.0f))
         == WPL_RESULT_CAPACITY_EXCEEDED);
  assert(wpl_draw_list_count(list) == before);

  assert(wpl_draw_line(list,
                       a,
                       b,
                       wpl_test_color(),
                       1.0f)
         == WPL_RESULT_OK);
  before = wpl_draw_list_count(list);

  assert(wpl_draw_dashed_line(list,
                              a,
                              b,
                              wpl_test_color(),
                              1.0f,
                              wpl_test_dash_pattern(20.0f, 5.0f))
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == before + 1u);

  before = wpl_draw_list_count(list);
  assert(wpl_draw_dashed_line(list,
                              a,
                              b,
                              wpl_test_color(),
                              1.0f,
                              wpl_test_dash_pattern(20.0f, 5.0f))
         == WPL_RESULT_CAPACITY_EXCEEDED);
  assert(wpl_draw_list_count(list) == before);

  wpl_destroy_draw_list(list);
}


static void
test_clip_validation(void)
{
  WplDrawList* list = wpl_test_create_list(8u);
  WplRect rect = wpl_test_rect();

  assert(wpl_draw_push_clip(NULL, rect) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_pop_clip(NULL) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_pop_clip(list) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  rect = wpl_test_rect();
  rect.x = NAN;
  assert(wpl_draw_push_clip(list, rect) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  rect = wpl_test_rect();
  rect.y = INFINITY;
  assert(wpl_draw_push_clip(list, rect) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  rect = wpl_test_rect();
  rect.w = -1.0f;
  assert(wpl_draw_push_clip(list, rect) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  rect = wpl_test_rect();
  rect.h = -1.0f;
  assert(wpl_draw_push_clip(list, rect) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  rect = wpl_test_rect();
  rect.w = 0.0f;
  assert(wpl_draw_push_clip(list, rect) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);
  assert(wpl_draw_pop_clip(list) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 2u);

  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  rect = wpl_test_rect();
  rect.h = 0.0f;
  assert(wpl_draw_push_clip(list, rect) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);
  assert(wpl_draw_pop_clip(list) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 2u);

  wpl_destroy_draw_list(list);
}

static void
test_clip_push_pop_counts_and_clear(void)
{
  WplDrawList* list = wpl_test_create_list(8u);

  assert(wpl_draw_push_clip(list, wpl_test_rect()) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);
  assert(wpl_draw_pop_clip(list) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 2u);

  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  assert(wpl_draw_push_clip(list, wpl_test_rect()) == WPL_RESULT_OK);
  assert(wpl_draw_push_clip(list, wpl_test_rect()) == WPL_RESULT_OK);
  assert(wpl_draw_pop_clip(list) == WPL_RESULT_OK);
  assert(wpl_draw_pop_clip(list) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 4u);
  assert(wpl_draw_pop_clip(list) == WPL_RESULT_INVALID_ARGUMENT);

  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  assert(wpl_draw_push_clip(list, wpl_test_rect()) == WPL_RESULT_OK);
  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  assert(wpl_draw_pop_clip(list) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  wpl_destroy_draw_list(list);
}

static void
test_clip_capacity_failures_preserve_state(void)
{
  WplDrawList* list = wpl_test_create_list(1u);
  size_t before;

  assert(wpl_draw_clear(list, wpl_test_color()) == WPL_RESULT_OK);
  before = wpl_draw_list_count(list);
  assert(wpl_draw_push_clip(list, wpl_test_rect())
         == WPL_RESULT_CAPACITY_EXCEEDED);
  assert(wpl_draw_list_count(list) == before);

  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  assert(wpl_draw_push_clip(list, wpl_test_rect()) == WPL_RESULT_OK);
  before = wpl_draw_list_count(list);
  assert(wpl_draw_pop_clip(list) == WPL_RESULT_CAPACITY_EXCEEDED);
  assert(wpl_draw_list_count(list) == before);

  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  assert(wpl_draw_pop_clip(list) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  wpl_destroy_draw_list(list);
}


static void
test_rounded_rect_validation(void)
{
  WplDrawList* list = wpl_test_create_list(8u);
  WplRect rect = wpl_test_rect();
  WplColor color = wpl_test_color();

  assert(wpl_draw_rounded_rect(NULL, rect, 1.0f, color)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  rect.x = NAN;
  assert(wpl_draw_rounded_rect(list, rect, 1.0f, color)
         == WPL_RESULT_INVALID_ARGUMENT);
  rect = wpl_test_rect();
  assert(wpl_draw_list_count(list) == 0u);

  rect.y = INFINITY;
  assert(wpl_draw_rounded_rect(list, rect, 1.0f, color)
         == WPL_RESULT_INVALID_ARGUMENT);
  rect = wpl_test_rect();
  assert(wpl_draw_list_count(list) == 0u);

  rect.w = -1.0f;
  assert(wpl_draw_rounded_rect(list, rect, 1.0f, color)
         == WPL_RESULT_INVALID_ARGUMENT);
  rect = wpl_test_rect();
  assert(wpl_draw_list_count(list) == 0u);

  rect.h = -1.0f;
  assert(wpl_draw_rounded_rect(list, rect, 1.0f, color)
         == WPL_RESULT_INVALID_ARGUMENT);
  rect = wpl_test_rect();
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_rounded_rect(list, rect, NAN, color)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_rounded_rect(list, rect, INFINITY, color)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_rounded_rect(list, rect, -1.0f, color)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  color = wpl_test_color();
  color.r = NAN;
  assert(wpl_draw_rounded_rect(list, rect, 1.0f, color)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  color = wpl_test_color();
  color.a = INFINITY;
  assert(wpl_draw_rounded_rect(list, rect, 1.0f, color)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  wpl_destroy_draw_list(list);
}

static void
test_rounded_rect_success_and_capacity(void)
{
  WplDrawList* list = wpl_test_create_list(4u);
  WplRect zero_width = wpl_test_rect();
  WplRect zero_height = wpl_test_rect();

  zero_width.w = 0.0f;
  zero_height.h = 0.0f;

  assert(wpl_draw_rounded_rect(list, wpl_test_rect(), 0.0f, wpl_test_color())
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);

  assert(wpl_draw_rounded_rect(list, wpl_test_rect(), 1000.0f, wpl_test_color())
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 2u);

  assert(wpl_draw_rounded_rect(list, zero_width, 1.0f, wpl_test_color())
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 3u);

  assert(wpl_draw_rounded_rect(list, zero_height, 1.0f, wpl_test_color())
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 4u);

  assert(wpl_draw_rounded_rect(list, wpl_test_rect(), 1.0f, wpl_test_color())
         == WPL_RESULT_CAPACITY_EXCEEDED);
  assert(wpl_draw_list_count(list) == 4u);

  wpl_destroy_draw_list(list);
}

static void
test_panel_validation(void)
{
  WplDrawList* list = wpl_test_create_list(8u);
  WplRect rect = wpl_test_rect();
  WplPanelStyle style = wpl_test_panel_style();

  assert(wpl_draw_panel(NULL, rect, style) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  rect.x = NAN;
  assert(wpl_draw_panel(list, rect, style) == WPL_RESULT_INVALID_ARGUMENT);
  rect = wpl_test_rect();
  assert(wpl_draw_list_count(list) == 0u);

  rect.w = -1.0f;
  assert(wpl_draw_panel(list, rect, style) == WPL_RESULT_INVALID_ARGUMENT);
  rect = wpl_test_rect();
  assert(wpl_draw_list_count(list) == 0u);

  style = wpl_test_panel_style();
  style.border_thickness = -1.0f;
  assert(wpl_draw_panel(list, rect, style) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  style = wpl_test_panel_style();
  style.border_thickness = NAN;
  assert(wpl_draw_panel(list, rect, style) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  style = wpl_test_panel_style();
  style.corner_radius = -1.0f;
  assert(wpl_draw_panel(list, rect, style) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  style = wpl_test_panel_style();
  style.corner_radius = INFINITY;
  assert(wpl_draw_panel(list, rect, style) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  style = wpl_test_panel_style();
  style.fill_color.g = NAN;
  assert(wpl_draw_panel(list, rect, style) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  style = wpl_test_panel_style();
  style.border_color.b = INFINITY;
  assert(wpl_draw_panel(list, rect, style) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  wpl_destroy_draw_list(list);
}

static void
test_panel_success_counts_and_append_position(void)
{
  WplDrawList* list = wpl_test_create_list(8u);
  WplPanelStyle style = wpl_test_panel_style();
  WplRect tiny_rect = {0.0f, 0.0f, 1.0f, 1.0f};

  style.border_thickness = 0.0f;
  assert(wpl_draw_panel(list, wpl_test_rect(), style) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);

  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  style = wpl_test_panel_style();
  assert(wpl_draw_panel(list, wpl_test_rect(), style) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 2u);

  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  style = wpl_test_panel_style();
  style.border_thickness = 2.0f;
  assert(wpl_draw_panel(list, tiny_rect, style) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);

  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  assert(wpl_draw_clear(list, wpl_test_color()) == WPL_RESULT_OK);
  style = wpl_test_panel_style();
  assert(wpl_draw_panel(list, wpl_test_rect(), style) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 3u);

  wpl_destroy_draw_list(list);
}

static void
test_panel_capacity_failure_preserves_count(void)
{
  WplDrawList* list = wpl_test_create_list(2u);
  WplPanelStyle style = wpl_test_panel_style();
  size_t before;

  assert(wpl_draw_clear(list, wpl_test_color()) == WPL_RESULT_OK);
  before = wpl_draw_list_count(list);
  assert(wpl_draw_panel(list, wpl_test_rect(), style)
         == WPL_RESULT_CAPACITY_EXCEEDED);
  assert(wpl_draw_list_count(list) == before);

  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  style.border_thickness = 0.0f;
  assert(wpl_draw_panel(list, wpl_test_rect(), style) == WPL_RESULT_OK);
  assert(wpl_draw_panel(list, wpl_test_rect(), style) == WPL_RESULT_OK);
  before = wpl_draw_list_count(list);
  assert(wpl_draw_panel(list, wpl_test_rect(), style)
         == WPL_RESULT_CAPACITY_EXCEEDED);
  assert(wpl_draw_list_count(list) == before);

  wpl_destroy_draw_list(list);
}

static void
test_zero_sized_shapes_are_accepted(void)
{
  WplDrawList* list = wpl_test_create_list(5u);
  WplRect rect = {0.0f, 0.0f, 0.0f, 0.0f};

  assert(wpl_draw_rect(list, rect, wpl_test_color()) == WPL_RESULT_OK);
  assert(wpl_draw_rounded_rect(list, rect, 1.0f, wpl_test_color())
         == WPL_RESULT_OK);
  assert(wpl_draw_rect_outline(list, rect, wpl_test_color(), 0.0f)
         == WPL_RESULT_OK);
  assert(wpl_draw_line(list,
                       wpl_test_vec2(0.0f, 0.0f),
                       wpl_test_vec2(0.0f, 0.0f),
                       wpl_test_color(),
                       0.0f)
         == WPL_RESULT_OK);
  assert(wpl_draw_circle(list, wpl_test_vec2(0.0f, 0.0f), 0.0f,
                         wpl_test_color())
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 5u);

  wpl_destroy_draw_list(list);
}

int
main(void)
{
  test_create_validation();
  test_null_accessors_and_clear();
  test_clear_resets_count_preserves_capacity();
  test_append_increments_count();
  test_capacity_failure_preserves_count();
  test_null_list_appends_are_invalid();
  test_invalid_geometry_preserves_count();
  test_nonfinite_values_are_rejected();
  test_out_of_range_finite_color_is_accepted();
  test_text_validation();
  test_polyline_validation();
  test_polyline_success_counts_and_append_position();
  test_polyline_capacity_failure_preserves_count();
  test_dashed_line_validation();
  test_dashed_line_pattern_validation();
  test_dashed_line_degenerate_line_appends_zero();
  test_dashed_line_success_counts_and_append_position();
  test_dashed_line_capacity_failure_preserves_count();
  test_clip_validation();
  test_clip_push_pop_counts_and_clear();
  test_clip_capacity_failures_preserve_state();
  test_rounded_rect_validation();
  test_rounded_rect_success_and_capacity();
  test_panel_validation();
  test_panel_success_counts_and_append_position();
  test_panel_capacity_failure_preserves_count();
  test_zero_sized_shapes_are_accepted();
  return 0;
}
