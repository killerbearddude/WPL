/* test_draw_derived_geometry.c - Extreme derived draw geometry validation. */

#include <wpl/wpl.h>

#include <assert.h>
#include <float.h>

static WplColor
wpl_test_color(void)
{
  WplColor color = {0.1f, 0.2f, 0.3f, 1.0f};
  return color;
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
test_line_derived_geometry_must_be_finite(void)
{
  WplDrawList* list = wpl_test_create_list(8u);

  assert(wpl_draw_line(list,
                       wpl_test_vec2(-FLT_MAX, 0.0f),
                       wpl_test_vec2(FLT_MAX, 0.0f),
                       wpl_test_color(),
                       1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_line(list,
                       wpl_test_vec2(0.0f, 0.0f),
                       wpl_test_vec2(1.0e20f, 0.0f),
                       wpl_test_color(),
                       1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_line(list,
                       wpl_test_vec2(0.0f, 0.0f),
                       wpl_test_vec2(0.0f, 0.0f),
                       wpl_test_color(),
                       FLT_MAX)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_line(list,
                       wpl_test_vec2(-FLT_MAX, 0.0f),
                       wpl_test_vec2(-FLT_MAX, 0.0f),
                       wpl_test_color(),
                       FLT_MAX)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  wpl_destroy_draw_list(list);
}

static void
test_polyline_derived_geometry_precedes_capacity_failure(void)
{
  WplDrawList* list = wpl_test_create_list(1u);
  WplVec2 points[3] = {
    {-FLT_MAX, 0.0f},
    {FLT_MAX, 0.0f},
    {0.0f, 0.0f}
  };

  assert(wpl_draw_polyline(list, points, 3u, wpl_test_color(), 1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  wpl_destroy_draw_list(list);
}

static void
test_dashed_line_derived_geometry_must_be_finite(void)
{
  WplDrawList* list = wpl_test_create_list(8u);
  WplDashPattern pattern = {2.0f, 2.0f};

  assert(wpl_draw_dashed_line(list,
                              wpl_test_vec2(-FLT_MAX, 0.0f),
                              wpl_test_vec2(FLT_MAX, 0.0f),
                              wpl_test_color(),
                              1.0f,
                              pattern)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_dashed_line(list,
                              wpl_test_vec2(0.0f, 0.0f),
                              wpl_test_vec2(1.0e20f, 0.0f),
                              wpl_test_color(),
                              1.0f,
                              pattern)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_dashed_line(list,
                              wpl_test_vec2(0.0f, 0.0f),
                              wpl_test_vec2(0.0f, 0.0f),
                              wpl_test_color(),
                              FLT_MAX,
                              pattern)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  wpl_destroy_draw_list(list);
}

static void
test_circle_derived_geometry_must_be_finite(void)
{
  WplDrawList* list = wpl_test_create_list(8u);

  assert(wpl_draw_circle(list,
                         wpl_test_vec2(0.0f, 0.0f),
                         1.0e20f,
                         wpl_test_color())
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  assert(wpl_draw_circle(list,
                         wpl_test_vec2(FLT_MAX, 0.0f),
                         FLT_MAX,
                         wpl_test_color())
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_draw_list_count(list) == 0u);

  wpl_destroy_draw_list(list);
}

int
main(void)
{
  test_line_derived_geometry_must_be_finite();
  test_polyline_derived_geometry_precedes_capacity_failure();
  test_dashed_line_derived_geometry_must_be_finite();
  test_circle_derived_geometry_must_be_finite();
  return 0;
}
