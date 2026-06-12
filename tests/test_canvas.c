/* test_canvas.c - Public canvas math behavior tests. */

#include <wpl/wpl.h>

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>

#define WPL_TEST_EPSILON 0.0001f

static WplVec2
wpl_test_vec2(float x, float y)
{
  WplVec2 value = {x, y};
  return value;
}

static WplCanvasView
wpl_test_view(float pan_x, float pan_y, float zoom)
{
  WplCanvasView view = {{pan_x, pan_y}, zoom, 0.25f, 16.0f};
  return view;
}

static bool
wpl_test_nearly_equal(float a, float b)
{
  float delta = a - b;

  if (delta < 0.0f)
    delta = -delta;

  return delta <= WPL_TEST_EPSILON;
}

static void
wpl_test_assert_vec2_near(WplVec2 actual, WplVec2 expected)
{
  assert(wpl_test_nearly_equal(actual.x, expected.x));
  assert(wpl_test_nearly_equal(actual.y, expected.y));
}

static void
wpl_test_assert_rect_near(WplRect actual, WplRect expected)
{
  assert(wpl_test_nearly_equal(actual.x, expected.x));
  assert(wpl_test_nearly_equal(actual.y, expected.y));
  assert(wpl_test_nearly_equal(actual.w, expected.w));
  assert(wpl_test_nearly_equal(actual.h, expected.h));
}

static void
wpl_test_assert_view_near(WplCanvasView actual, WplCanvasView expected)
{
  wpl_test_assert_vec2_near(actual.pan, expected.pan);
  assert(wpl_test_nearly_equal(actual.zoom, expected.zoom));
  assert(wpl_test_nearly_equal(actual.min_zoom, expected.min_zoom));
  assert(wpl_test_nearly_equal(actual.max_zoom, expected.max_zoom));
}

static void
test_screen_to_canvas_identity(void)
{
  WplCanvasView view = wpl_test_view(0.0f, 0.0f, 1.0f);
  WplVec2 out = {0};

  assert(wpl_screen_to_canvas(&view, wpl_test_vec2(10.0f, 20.0f), &out)
         == WPL_RESULT_OK);
  wpl_test_assert_vec2_near(out, wpl_test_vec2(10.0f, 20.0f));
}

static void
test_canvas_to_screen_identity(void)
{
  WplCanvasView view = wpl_test_view(0.0f, 0.0f, 1.0f);
  WplVec2 out = {0};

  assert(wpl_canvas_to_screen(&view, wpl_test_vec2(10.0f, 20.0f), &out)
         == WPL_RESULT_OK);
  wpl_test_assert_vec2_near(out, wpl_test_vec2(10.0f, 20.0f));
}

static void
test_pan_only_conversions(void)
{
  WplCanvasView view = wpl_test_view(100.0f, 50.0f, 1.0f);
  WplVec2 out = {0};

  assert(wpl_screen_to_canvas(&view, wpl_test_vec2(125.0f, 75.0f), &out)
         == WPL_RESULT_OK);
  wpl_test_assert_vec2_near(out, wpl_test_vec2(25.0f, 25.0f));

  assert(wpl_canvas_to_screen(&view, wpl_test_vec2(25.0f, 25.0f), &out)
         == WPL_RESULT_OK);
  wpl_test_assert_vec2_near(out, wpl_test_vec2(125.0f, 75.0f));
}

static void
test_zoom_only_conversions(void)
{
  WplCanvasView view = wpl_test_view(0.0f, 0.0f, 2.0f);
  WplVec2 out = {0};

  assert(wpl_screen_to_canvas(&view, wpl_test_vec2(20.0f, 40.0f), &out)
         == WPL_RESULT_OK);
  wpl_test_assert_vec2_near(out, wpl_test_vec2(10.0f, 20.0f));

  assert(wpl_canvas_to_screen(&view, wpl_test_vec2(10.0f, 20.0f), &out)
         == WPL_RESULT_OK);
  wpl_test_assert_vec2_near(out, wpl_test_vec2(20.0f, 40.0f));
}

static void
test_pan_and_zoom_conversions(void)
{
  WplCanvasView view = wpl_test_view(10.0f, 20.0f, 4.0f);
  WplVec2 out = {0};

  assert(wpl_screen_to_canvas(&view, wpl_test_vec2(50.0f, 100.0f), &out)
         == WPL_RESULT_OK);
  wpl_test_assert_vec2_near(out, wpl_test_vec2(10.0f, 20.0f));

  assert(wpl_canvas_to_screen(&view, wpl_test_vec2(10.0f, 20.0f), &out)
         == WPL_RESULT_OK);
  wpl_test_assert_vec2_near(out, wpl_test_vec2(50.0f, 100.0f));
}

static void
test_round_trips(void)
{
  WplCanvasView view = wpl_test_view(-12.0f, 25.0f, 3.5f);
  WplVec2 screen = wpl_test_vec2(140.0f, 80.0f);
  WplVec2 canvas = wpl_test_vec2(-7.0f, 13.0f);
  WplVec2 tmp = {0};
  WplVec2 out = {0};

  assert(wpl_screen_to_canvas(&view, screen, &tmp) == WPL_RESULT_OK);
  assert(wpl_canvas_to_screen(&view, tmp, &out) == WPL_RESULT_OK);
  wpl_test_assert_vec2_near(out, screen);

  assert(wpl_canvas_to_screen(&view, canvas, &tmp) == WPL_RESULT_OK);
  assert(wpl_screen_to_canvas(&view, tmp, &out) == WPL_RESULT_OK);
  wpl_test_assert_vec2_near(out, canvas);
}

static void
test_pan_by_updates_screen_space_pan(void)
{
  WplCanvasView view = wpl_test_view(10.0f, 20.0f, 2.0f);

  assert(wpl_canvas_pan_by(&view, wpl_test_vec2(5.0f, -8.0f))
         == WPL_RESULT_OK);
  wpl_test_assert_vec2_near(view.pan, wpl_test_vec2(15.0f, 12.0f));
  assert(wpl_test_nearly_equal(view.zoom, 2.0f));
}

static void
test_zoom_around_cursor_anchor(void)
{
  WplCanvasView view = wpl_test_view(10.0f, 20.0f, 2.0f);
  WplVec2 cursor_screen = wpl_test_vec2(110.0f, 220.0f);
  WplVec2 before = {0};
  WplVec2 after = {0};

  assert(wpl_screen_to_canvas(&view, cursor_screen, &before)
         == WPL_RESULT_OK);
  assert(wpl_canvas_zoom_around(&view, cursor_screen, 2.0f)
         == WPL_RESULT_OK);
  assert(wpl_test_nearly_equal(view.zoom, 4.0f));
  assert(wpl_screen_to_canvas(&view, cursor_screen, &after)
         == WPL_RESULT_OK);
  wpl_test_assert_vec2_near(after, before);
}

static void
test_zoom_around_clamps_to_min_and_max(void)
{
  WplCanvasView view = {{0.0f, 0.0f}, 2.0f, 1.0f, 4.0f};

  assert(wpl_canvas_zoom_around(&view, wpl_test_vec2(50.0f, 50.0f), 100.0f)
         == WPL_RESULT_OK);
  assert(wpl_test_nearly_equal(view.zoom, 4.0f));

  assert(wpl_canvas_zoom_around(&view, wpl_test_vec2(50.0f, 50.0f), 0.01f)
         == WPL_RESULT_OK);
  assert(wpl_test_nearly_equal(view.zoom, 1.0f));
}

static void
test_zoom_around_failure_preserves_view(void)
{
  WplCanvasView view = {{0.0f, 0.0f}, 1.0f, 1.0f, FLT_MAX};
  WplCanvasView before = view;

  assert(wpl_canvas_zoom_around(&view, wpl_test_vec2(FLT_MAX, 0.0f), FLT_MAX)
         == WPL_RESULT_INVALID_ARGUMENT);
  wpl_test_assert_view_near(view, before);
}

static void
test_invalid_view_and_output_rejected(void)
{
  WplCanvasView view = wpl_test_view(0.0f, 0.0f, 1.0f);
  WplVec2 out = {0};

  assert(wpl_screen_to_canvas(NULL, wpl_test_vec2(0.0f, 0.0f), &out)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_canvas_to_screen(NULL, wpl_test_vec2(0.0f, 0.0f), &out)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_screen_to_canvas(&view, wpl_test_vec2(0.0f, 0.0f), NULL)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_canvas_to_screen(&view, wpl_test_vec2(0.0f, 0.0f), NULL)
         == WPL_RESULT_INVALID_ARGUMENT);

  view.zoom = 0.0f;
  assert(wpl_screen_to_canvas(&view, wpl_test_vec2(0.0f, 0.0f), &out)
         == WPL_RESULT_INVALID_ARGUMENT);

  view = wpl_test_view(0.0f, 0.0f, -1.0f);
  assert(wpl_canvas_to_screen(&view, wpl_test_vec2(0.0f, 0.0f), &out)
         == WPL_RESULT_INVALID_ARGUMENT);

  view = wpl_test_view(0.0f, 0.0f, 1.0f);
  view.min_zoom = 0.0f;
  assert(wpl_canvas_pan_by(&view, wpl_test_vec2(1.0f, 1.0f))
         == WPL_RESULT_INVALID_ARGUMENT);

  view = wpl_test_view(0.0f, 0.0f, 1.0f);
  view.min_zoom = 2.0f;
  view.max_zoom = 1.0f;
  assert(wpl_canvas_zoom_around(&view, wpl_test_vec2(0.0f, 0.0f), 1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
}

static void
test_nonfinite_inputs_rejected(void)
{
  WplCanvasView view = wpl_test_view(0.0f, 0.0f, 1.0f);
  WplVec2 out = {0};

  view.pan.x = NAN;
  assert(wpl_screen_to_canvas(&view, wpl_test_vec2(0.0f, 0.0f), &out)
         == WPL_RESULT_INVALID_ARGUMENT);

  view = wpl_test_view(0.0f, 0.0f, 1.0f);
  view.zoom = INFINITY;
  assert(wpl_canvas_to_screen(&view, wpl_test_vec2(0.0f, 0.0f), &out)
         == WPL_RESULT_INVALID_ARGUMENT);

  view = wpl_test_view(0.0f, 0.0f, 1.0f);
  assert(wpl_screen_to_canvas(&view, wpl_test_vec2(NAN, 0.0f), &out)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_canvas_to_screen(&view, wpl_test_vec2(0.0f, INFINITY), &out)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_canvas_pan_by(&view, wpl_test_vec2(INFINITY, 0.0f))
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_canvas_zoom_around(&view, wpl_test_vec2(0.0f, NAN), 1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_canvas_zoom_around(&view, wpl_test_vec2(0.0f, 0.0f), NAN)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_canvas_zoom_around(&view, wpl_test_vec2(0.0f, 0.0f), INFINITY)
         == WPL_RESULT_INVALID_ARGUMENT);
}

static void
test_invalid_zoom_factor_rejected(void)
{
  WplCanvasView view = wpl_test_view(0.0f, 0.0f, 1.0f);

  assert(wpl_canvas_zoom_around(&view, wpl_test_vec2(0.0f, 0.0f), 0.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_canvas_zoom_around(&view, wpl_test_vec2(0.0f, 0.0f), -1.0f)
         == WPL_RESULT_INVALID_ARGUMENT);
}

static void
test_rect_contains_point(void)
{
  WplRect rect = {10.0f, 20.0f, 30.0f, 40.0f};

  assert(wpl_rect_contains_point(rect, wpl_test_vec2(10.0f, 20.0f)));
  assert(wpl_rect_contains_point(rect, wpl_test_vec2(39.0f, 59.0f)));
  assert(!wpl_rect_contains_point(rect, wpl_test_vec2(40.0f, 59.0f)));
  assert(!wpl_rect_contains_point(rect, wpl_test_vec2(39.0f, 60.0f)));
  assert(!wpl_rect_contains_point(rect, wpl_test_vec2(9.0f, 20.0f)));

  rect.w = -1.0f;
  assert(!wpl_rect_contains_point(rect, wpl_test_vec2(10.0f, 20.0f)));
}

static void
test_rect_intersection_helpers(void)
{
  WplRect a = {0.0f, 0.0f, 10.0f, 10.0f};
  WplRect b = {5.0f, 6.0f, 10.0f, 10.0f};
  WplRect touching = {10.0f, 0.0f, 5.0f, 5.0f};
  WplRect c;

  assert(wpl_rect_intersects(a, b));
  c = wpl_rect_intersection(a, b);
  wpl_test_assert_rect_near(c, (WplRect){5.0f, 6.0f, 5.0f, 4.0f});

  assert(!wpl_rect_intersects(a, touching));
  c = wpl_rect_intersection(a, touching);
  wpl_test_assert_rect_near(c, (WplRect){0.0f, 0.0f, 0.0f, 0.0f});
}

static void
test_rect_helpers_reject_overflowed_edges(void)
{
  WplRect normal = {0.0f, 0.0f, 10.0f, 10.0f};
  WplRect overflow_x = {FLT_MAX, 0.0f, FLT_MAX, 1.0f};
  WplRect overflow_y = {0.0f, FLT_MAX, 1.0f, FLT_MAX};
  WplRect c;

  assert(!wpl_rect_contains_point(overflow_x, wpl_test_vec2(FLT_MAX, 0.5f)));
  assert(!wpl_rect_intersects(overflow_x, normal));
  c = wpl_rect_intersection(overflow_x, normal);
  wpl_test_assert_rect_near(c, (WplRect){0.0f, 0.0f, 0.0f, 0.0f});

  assert(!wpl_rect_contains_point(overflow_y, wpl_test_vec2(0.5f, FLT_MAX)));
  assert(!wpl_rect_intersects(overflow_y, normal));
  c = wpl_rect_intersection(overflow_y, normal);
  wpl_test_assert_rect_near(c, (WplRect){0.0f, 0.0f, 0.0f, 0.0f});
}

int
main(void)
{
  test_screen_to_canvas_identity();
  test_canvas_to_screen_identity();
  test_pan_only_conversions();
  test_zoom_only_conversions();
  test_pan_and_zoom_conversions();
  test_round_trips();
  test_pan_by_updates_screen_space_pan();
  test_zoom_around_cursor_anchor();
  test_zoom_around_clamps_to_min_and_max();
  test_zoom_around_failure_preserves_view();
  test_invalid_view_and_output_rejected();
  test_nonfinite_inputs_rejected();
  test_invalid_zoom_factor_rejected();
  test_rect_contains_point();
  test_rect_intersection_helpers();
  test_rect_helpers_reject_overflowed_edges();
  return 0;
}
