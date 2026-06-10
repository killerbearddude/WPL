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

static void
test_zero_sized_shapes_are_accepted(void)
{
  WplDrawList* list = wpl_test_create_list(4u);
  WplRect rect = {0.0f, 0.0f, 0.0f, 0.0f};

  assert(wpl_draw_rect(list, rect, wpl_test_color()) == WPL_RESULT_OK);
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
  assert(wpl_draw_list_count(list) == 4u);

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
  test_zero_sized_shapes_are_accepted();
  return 0;
}
