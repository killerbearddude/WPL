/* wpl_canvas.c - backend-independent canvas coordinate math. */

#include "wpl/wpl_canvas.h"

#include <math.h>
#include <stdbool.h>

static bool
wpl_float_is_finite(float value)
{
  return isfinite(value);
}

static bool
wpl_vec2_is_finite(WplVec2 value)
{
  return (wpl_float_is_finite(value.x) && wpl_float_is_finite(value.y));
}

static bool
wpl_rect_is_finite(WplRect rect)
{
  return (wpl_float_is_finite(rect.x)
          && wpl_float_is_finite(rect.y)
          && wpl_float_is_finite(rect.w)
          && wpl_float_is_finite(rect.h));
}

static bool
wpl_rect_get_edges(WplRect rect,
                   float* out_x0,
                   float* out_y0,
                   float* out_x1,
                   float* out_y1)
{
  float x1;
  float y1;

  if (out_x0 == NULL || out_y0 == NULL || out_x1 == NULL || out_y1 == NULL)
    return false;

  *out_x0 = 0.0f;
  *out_y0 = 0.0f;
  *out_x1 = 0.0f;
  *out_y1 = 0.0f;

  if (!wpl_rect_is_finite(rect))
    return false;

  if (rect.w < 0.0f || rect.h < 0.0f)
    return false;

  x1 = rect.x + rect.w;
  y1 = rect.y + rect.h;
  if (!wpl_float_is_finite(x1) || !wpl_float_is_finite(y1))
    return false;

  *out_x0 = rect.x;
  *out_y0 = rect.y;
  *out_x1 = x1;
  *out_y1 = y1;
  return true;
}

static bool
wpl_canvas_view_is_valid(const WplCanvasView* view)
{
  if (view == NULL)
    return false;

  if (!wpl_vec2_is_finite(view->pan))
    return false;

  if (!wpl_float_is_finite(view->zoom)
      || !wpl_float_is_finite(view->min_zoom)
      || !wpl_float_is_finite(view->max_zoom))
    return false;

  if (view->zoom <= 0.0f)
    return false;

  if (view->min_zoom <= 0.0f || view->max_zoom <= 0.0f)
    return false;

  return view->min_zoom <= view->max_zoom;
}

static float
wpl_max_float(float a, float b)
{
  return a > b ? a : b;
}

static float
wpl_min_float(float a, float b)
{
  return a < b ? a : b;
}

static float
wpl_clamp_float(float value, float min_value, float max_value)
{
  if (value < min_value)
    return min_value;

  if (value > max_value)
    return max_value;

  return value;
}

static WplResult
wpl_canvas_validate_view_and_vec2(const WplCanvasView* view, WplVec2 value)
{
  if (!wpl_canvas_view_is_valid(view))
    return WPL_RESULT_INVALID_ARGUMENT;

  if (!wpl_vec2_is_finite(value))
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_OK;
}

WplResult
wpl_screen_to_canvas(const WplCanvasView* view,
                     WplVec2 screen,
                     WplVec2* out_canvas)
{
  WplResult result;

  if (out_canvas == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_canvas_validate_view_and_vec2(view, screen);
  if (result != WPL_RESULT_OK)
    return result;

  out_canvas->x = (screen.x - view->pan.x) / view->zoom;
  out_canvas->y = (screen.y - view->pan.y) / view->zoom;
  return WPL_RESULT_OK;
}

WplResult
wpl_canvas_to_screen(const WplCanvasView* view,
                     WplVec2 canvas,
                     WplVec2* out_screen)
{
  WplResult result;

  if (out_screen == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_canvas_validate_view_and_vec2(view, canvas);
  if (result != WPL_RESULT_OK)
    return result;

  out_screen->x = canvas.x * view->zoom + view->pan.x;
  out_screen->y = canvas.y * view->zoom + view->pan.y;
  return WPL_RESULT_OK;
}

WplResult
wpl_canvas_pan_by(WplCanvasView* view, WplVec2 screen_delta)
{
  WplVec2 new_pan;

  if (!wpl_canvas_view_is_valid(view))
    return WPL_RESULT_INVALID_ARGUMENT;

  if (!wpl_vec2_is_finite(screen_delta))
    return WPL_RESULT_INVALID_ARGUMENT;

  new_pan.x = view->pan.x + screen_delta.x;
  new_pan.y = view->pan.y + screen_delta.y;
  if (!wpl_vec2_is_finite(new_pan))
    return WPL_RESULT_INVALID_ARGUMENT;

  view->pan = new_pan;
  return WPL_RESULT_OK;
}

WplResult
wpl_canvas_zoom_around(WplCanvasView* view,
                       WplVec2 cursor_screen,
                       float zoom_factor)
{
  WplCanvasView next;
  WplVec2 canvas_before;
  WplResult result;

  if (!wpl_canvas_view_is_valid(view))
    return WPL_RESULT_INVALID_ARGUMENT;

  if (!wpl_vec2_is_finite(cursor_screen))
    return WPL_RESULT_INVALID_ARGUMENT;

  if (!wpl_float_is_finite(zoom_factor) || zoom_factor <= 0.0f)
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_screen_to_canvas(view, cursor_screen, &canvas_before);
  if (result != WPL_RESULT_OK)
    return result;

  next = *view;
  next.zoom = wpl_clamp_float(view->zoom * zoom_factor,
                              view->min_zoom,
                              view->max_zoom);
  next.pan.x = cursor_screen.x - canvas_before.x * next.zoom;
  next.pan.y = cursor_screen.y - canvas_before.y * next.zoom;

  if (!wpl_canvas_view_is_valid(&next))
    return WPL_RESULT_INVALID_ARGUMENT;

  *view = next;
  return WPL_RESULT_OK;
}

bool
wpl_rect_contains_point(WplRect rect, WplVec2 point)
{
  float x0;
  float y0;
  float x1;
  float y1;

  if (!wpl_vec2_is_finite(point))
    return false;

  if (!wpl_rect_get_edges(rect, &x0, &y0, &x1, &y1))
    return false;

  return (point.x >= x0
          && point.y >= y0
          && point.x < x1
          && point.y < y1);
}

bool
wpl_rect_intersects(WplRect a, WplRect b)
{
  float ax0;
  float ay0;
  float ax1;
  float ay1;
  float bx0;
  float by0;
  float bx1;
  float by1;

  if (!wpl_rect_get_edges(a, &ax0, &ay0, &ax1, &ay1)
      || !wpl_rect_get_edges(b, &bx0, &by0, &bx1, &by1))
    return false;

  if (ax0 >= ax1 || ay0 >= ay1 || bx0 >= bx1 || by0 >= by1)
    return false;

  return (ax0 < bx1
          && ax1 > bx0
          && ay0 < by1
          && ay1 > by0);
}

WplRect
wpl_rect_intersection(WplRect a, WplRect b)
{
  WplRect zero = {0.0f, 0.0f, 0.0f, 0.0f};
  float ax0;
  float ay0;
  float ax1;
  float ay1;
  float bx0;
  float by0;
  float bx1;
  float by1;
  float x0;
  float y0;
  float x1;
  float y1;

  if (!wpl_rect_get_edges(a, &ax0, &ay0, &ax1, &ay1)
      || !wpl_rect_get_edges(b, &bx0, &by0, &bx1, &by1))
    return zero;

  if (ax0 >= ax1 || ay0 >= ay1 || bx0 >= bx1 || by0 >= by1)
    return zero;

  x0 = wpl_max_float(ax0, bx0);
  y0 = wpl_max_float(ay0, by0);
  x1 = wpl_min_float(ax1, bx1);
  y1 = wpl_min_float(ay1, by1);

  if (x1 <= x0 || y1 <= y0)
    return zero;

  return (WplRect){x0, y0, x1 - x0, y1 - y0};
}
