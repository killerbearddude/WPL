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

  view->zoom = wpl_clamp_float(view->zoom * zoom_factor,
                               view->min_zoom,
                               view->max_zoom);
  view->pan.x = cursor_screen.x - canvas_before.x * view->zoom;
  view->pan.y = cursor_screen.y - canvas_before.y * view->zoom;

  if (!wpl_canvas_view_is_valid(view))
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_OK;
}

bool
wpl_rect_contains_point(WplRect rect, WplVec2 point)
{
  if (!wpl_rect_is_finite(rect) || !wpl_vec2_is_finite(point))
    return false;

  if (rect.w < 0.0f || rect.h < 0.0f)
    return false;

  return (point.x >= rect.x
          && point.y >= rect.y
          && point.x < rect.x + rect.w
          && point.y < rect.y + rect.h);
}

bool
wpl_rect_intersects(WplRect a, WplRect b)
{
  if (!wpl_rect_is_finite(a) || !wpl_rect_is_finite(b))
    return false;

  if (a.w <= 0.0f || a.h <= 0.0f || b.w <= 0.0f || b.h <= 0.0f)
    return false;

  return (a.x < b.x + b.w
          && a.x + a.w > b.x
          && a.y < b.y + b.h
          && a.y + a.h > b.y);
}

WplRect
wpl_rect_intersection(WplRect a, WplRect b)
{
  WplRect zero = {0.0f, 0.0f, 0.0f, 0.0f};
  float x0;
  float y0;
  float x1;
  float y1;

  if (!wpl_rect_intersects(a, b))
    return zero;

  x0 = wpl_max_float(a.x, b.x);
  y0 = wpl_max_float(a.y, b.y);
  x1 = wpl_min_float(a.x + a.w, b.x + b.w);
  y1 = wpl_min_float(a.y + a.h, b.y + b.h);

  if (x1 <= x0 || y1 <= y0)
    return zero;

  return (WplRect){x0, y0, x1 - x0, y1 - y0};
}
