/* wpl_draw.c - Fixed-capacity draw command buffer implementation. */

#include "wpl_draw_internal.h"
#include "wpl_text_internal.h"

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

static bool
wpl_draw_float_is_finite(float value)
{
  return isfinite(value) != 0;
}

static bool
wpl_draw_vec2_is_finite(WplVec2 value)
{
  return (wpl_draw_float_is_finite(value.x)
          && wpl_draw_float_is_finite(value.y));
}

static bool
wpl_draw_rect_is_finite(WplRect rect)
{
  return (wpl_draw_float_is_finite(rect.x)
          && wpl_draw_float_is_finite(rect.y)
          && wpl_draw_float_is_finite(rect.w)
          && wpl_draw_float_is_finite(rect.h));
}

static bool
wpl_draw_color_is_finite(WplColor color)
{
  return (wpl_draw_float_is_finite(color.r)
          && wpl_draw_float_is_finite(color.g)
          && wpl_draw_float_is_finite(color.b)
          && wpl_draw_float_is_finite(color.a));
}

static WplResult
wpl_draw_validate_color(WplColor color)
{
  if (!wpl_draw_color_is_finite(color))
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_OK;
}

static bool
wpl_draw_points_are_finite(const WplVec2* points, size_t point_count)
{
  size_t i;

  if (points == NULL)
    return false;

  for (i = 0u; i < point_count; i++)
    {
      if (!wpl_draw_vec2_is_finite(points[i]))
        return false;
    }

  return true;
}

static WplResult
wpl_draw_validate_dash_pattern(WplDashPattern pattern)
{
  if (!wpl_draw_float_is_finite(pattern.dash_length)
      || !wpl_draw_float_is_finite(pattern.gap_length))
    return WPL_RESULT_INVALID_ARGUMENT;

  if (pattern.dash_length <= 0.0f || pattern.gap_length <= 0.0f)
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_OK;
}

static float
wpl_draw_vec2_distance(WplVec2 a, WplVec2 b)
{
  float dx = b.x - a.x;
  float dy = b.y - a.y;
  return sqrtf(dx * dx + dy * dy);
}

static WplVec2
wpl_draw_vec2_lerp(WplVec2 a, WplVec2 b, float t)
{
  WplVec2 result;
  result.x = a.x + (b.x - a.x) * t;
  result.y = a.y + (b.y - a.y) * t;
  return result;
}

static WplResult
wpl_draw_dashed_line_segment_count_limited(WplVec2 a,
                                           WplVec2 b,
                                           WplDashPattern pattern,
                                           size_t max_segments,
                                           size_t* out_count)
{
  float length;
  float period;
  float distance;
  size_t count = 0u;

  if (out_count == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  *out_count = 0u;

  length = wpl_draw_vec2_distance(a, b);
  if (!wpl_draw_float_is_finite(length))
    return WPL_RESULT_UNSUPPORTED;

  if (length <= 0.0f)
    return WPL_RESULT_OK;

  period = pattern.dash_length + pattern.gap_length;
  if (!wpl_draw_float_is_finite(period) || period <= 0.0f)
    return WPL_RESULT_UNSUPPORTED;

  distance = 0.0f;
  while (distance < length)
    {
      float next_distance;

      if (count >= max_segments)
        return WPL_RESULT_CAPACITY_EXCEEDED;

      count++;
      next_distance = distance + period;
      if (!wpl_draw_float_is_finite(next_distance) || next_distance <= distance)
        return WPL_RESULT_UNSUPPORTED;

      distance = next_distance;
    }

  *out_count = count;
  return WPL_RESULT_OK;
}

static WplResult
wpl_draw_validate_rect(WplRect rect)
{
  float x1;
  float y1;

  if (!wpl_draw_rect_is_finite(rect))
    return WPL_RESULT_INVALID_ARGUMENT;

  if (rect.w < 0.0f || rect.h < 0.0f)
    return WPL_RESULT_INVALID_ARGUMENT;

  x1 = rect.x + rect.w;
  y1 = rect.y + rect.h;
  if (!wpl_draw_float_is_finite(x1) || !wpl_draw_float_is_finite(y1))
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_OK;
}

static WplResult
wpl_draw_validate_thickness(float thickness)
{
  if (!wpl_draw_float_is_finite(thickness))
    return WPL_RESULT_INVALID_ARGUMENT;

  if (thickness < 0.0f)
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_OK;
}

static WplResult
wpl_draw_validate_radius(float radius)
{
  if (!wpl_draw_float_is_finite(radius))
    return WPL_RESULT_INVALID_ARGUMENT;

  if (radius < 0.0f)
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_OK;
}

static WplResult
wpl_draw_validate_line_geometry(WplVec2 a, WplVec2 b, float thickness)
{
  float dx;
  float dy;
  float length_sq;
  float radius;
  float radius_sq;
  float min_x;
  float max_x;
  float min_y;
  float max_y;
  float bound_x0;
  float bound_x1;
  float bound_y0;
  float bound_y1;
  WplResult result;

  if (!wpl_draw_vec2_is_finite(a) || !wpl_draw_vec2_is_finite(b))
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_draw_validate_thickness(thickness);
  if (result != WPL_RESULT_OK)
    return result;

  dx = b.x - a.x;
  dy = b.y - a.y;
  if (!wpl_draw_float_is_finite(dx) || !wpl_draw_float_is_finite(dy))
    return WPL_RESULT_INVALID_ARGUMENT;

  length_sq = dx * dx + dy * dy;
  if (!wpl_draw_float_is_finite(length_sq))
    return WPL_RESULT_INVALID_ARGUMENT;

  radius = thickness * 0.5f;
  if (!wpl_draw_float_is_finite(radius))
    return WPL_RESULT_INVALID_ARGUMENT;

  radius_sq = radius * radius;
  if (!wpl_draw_float_is_finite(radius_sq))
    return WPL_RESULT_INVALID_ARGUMENT;

  min_x = a.x < b.x ? a.x : b.x;
  max_x = a.x > b.x ? a.x : b.x;
  min_y = a.y < b.y ? a.y : b.y;
  max_y = a.y > b.y ? a.y : b.y;

  bound_x0 = min_x - radius;
  bound_x1 = max_x + radius;
  bound_y0 = min_y - radius;
  bound_y1 = max_y + radius;
  if (!wpl_draw_float_is_finite(bound_x0)
      || !wpl_draw_float_is_finite(bound_x1)
      || !wpl_draw_float_is_finite(bound_y0)
      || !wpl_draw_float_is_finite(bound_y1))
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_OK;
}

static WplResult
wpl_draw_validate_polyline_geometry(const WplVec2* points,
                                    size_t point_count,
                                    float thickness)
{
  size_t i;
  WplResult result;

  if (points == NULL || point_count < 2u)
    return WPL_RESULT_INVALID_ARGUMENT;

  for (i = 0u; i + 1u < point_count; i++)
    {
      result = wpl_draw_validate_line_geometry(points[i],
                                               points[i + 1u],
                                               thickness);
      if (result != WPL_RESULT_OK)
        return result;
    }

  return WPL_RESULT_OK;
}

static WplResult
wpl_draw_validate_circle_geometry(WplVec2 center, float radius)
{
  float radius_sq;
  float x0;
  float x1;
  float y0;
  float y1;
  WplResult result;

  if (!wpl_draw_vec2_is_finite(center))
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_draw_validate_radius(radius);
  if (result != WPL_RESULT_OK)
    return result;

  radius_sq = radius * radius;
  if (!wpl_draw_float_is_finite(radius_sq))
    return WPL_RESULT_INVALID_ARGUMENT;

  x0 = center.x - radius;
  x1 = center.x + radius;
  y0 = center.y - radius;
  y1 = center.y + radius;
  if (!wpl_draw_float_is_finite(x0) || !wpl_draw_float_is_finite(x1)
      || !wpl_draw_float_is_finite(y0) || !wpl_draw_float_is_finite(y1))
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_OK;
}

static WplResult
wpl_draw_append_command(WplDrawList* list, const WplDrawCommand* command)
{
  if (list == NULL || command == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (list->count >= list->capacity)
    return WPL_RESULT_CAPACITY_EXCEEDED;

  list->commands[list->count] = *command;
  list->count++;
  return WPL_RESULT_OK;
}

static WplResult
wpl_draw_copy_text(char dst[WPL_DRAW_TEXT_MAX_BYTES + 1u], const char* src)
{
  size_t i = 0u;

  if (dst == NULL || src == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  while (i <= WPL_DRAW_TEXT_MAX_BYTES && src[i] != '\0')
    {
      if (i == WPL_DRAW_TEXT_MAX_BYTES)
        return WPL_RESULT_TRUNCATED;

      dst[i] = src[i];
      i++;
    }

  dst[i] = '\0';
  return WPL_RESULT_OK;
}

static WplResult
wpl_text_add_columns(size_t* columns, size_t amount)
{
  if (columns == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (*columns > SIZE_MAX - amount)
    return WPL_RESULT_UNSUPPORTED;

  *columns += amount;
  return WPL_RESULT_OK;
}

static WplResult
wpl_text_add_line(size_t* line_count)
{
  if (line_count == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (*line_count == SIZE_MAX)
    return WPL_RESULT_UNSUPPORTED;

  (*line_count)++;
  return WPL_RESULT_OK;
}

static bool
wpl_text_count_fits_float(size_t count, float scale)
{
  return ((long double)count <= ((long double)FLT_MAX / (long double)scale));
}

float
wpl_text_line_height(void)
{
  return WPL_TEXT_LINE_HEIGHT;
}

float
wpl_text_glyph_advance_x(void)
{
  return WPL_TEXT_GLYPH_ADVANCE_X;
}

WplResult
wpl_measure_text(const char* text, WplTextMetrics* out_metrics)
{
  size_t current_columns = 0u;
  size_t max_columns = 0u;
  size_t line_count = 1u;
  size_t i;
  WplResult result;

  if (out_metrics != NULL)
    *out_metrics = (WplTextMetrics){0};

  if (text == NULL || out_metrics == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  for (i = 0u; text[i] != '\0'; i++)
    {
      unsigned char c = (unsigned char)text[i];

      if (c == (unsigned char)'\n')
        {
          if (current_columns > max_columns)
            max_columns = current_columns;

          current_columns = 0u;
          result = wpl_text_add_line(&line_count);
          if (result != WPL_RESULT_OK)
            return result;
          continue;
        }

      if (c == (unsigned char)'\r')
        continue;

      if (c == (unsigned char)'\t')
        {
          result = wpl_text_add_columns(&current_columns, 4u);
          if (result != WPL_RESULT_OK)
            return result;
          continue;
        }

      result = wpl_text_add_columns(&current_columns, 1u);
      if (result != WPL_RESULT_OK)
        return result;
    }

  if (current_columns > max_columns)
    max_columns = current_columns;

  if (!wpl_text_count_fits_float(max_columns, WPL_TEXT_GLYPH_ADVANCE_X)
      || !wpl_text_count_fits_float(line_count, WPL_TEXT_LINE_HEIGHT))
    return WPL_RESULT_UNSUPPORTED;

  out_metrics->width = (float)max_columns * WPL_TEXT_GLYPH_ADVANCE_X;
  out_metrics->height = (float)line_count * WPL_TEXT_LINE_HEIGHT;
  out_metrics->line_height = WPL_TEXT_LINE_HEIGHT;
  return WPL_RESULT_OK;
}

WplResult
wpl_create_draw_list(size_t max_commands, WplDrawList** out_list)
{
  WplDrawList* list;

  if (out_list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  *out_list = NULL;

  if (max_commands == 0u)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (max_commands > (SIZE_MAX / sizeof(WplDrawCommand)))
    return WPL_RESULT_OUT_OF_MEMORY;

  list = (WplDrawList*)malloc(sizeof(*list));
  if (list == NULL)
    return WPL_RESULT_OUT_OF_MEMORY;

  list->commands = (WplDrawCommand*)calloc(max_commands,
                                           sizeof(list->commands[0]));
  if (list->commands == NULL)
    {
      free(list);
      return WPL_RESULT_OUT_OF_MEMORY;
    }

  list->count = 0u;
  list->capacity = max_commands;
  list->clip_depth = 0u;
  *out_list = list;
  return WPL_RESULT_OK;
}

void
wpl_destroy_draw_list(WplDrawList* list)
{
  if (list == NULL)
    return;

  free(list->commands);
  free(list);
}

WplResult
wpl_draw_list_clear(WplDrawList* list)
{
  if (list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  list->count = 0u;
  list->clip_depth = 0u;
  return WPL_RESULT_OK;
}

size_t
wpl_draw_list_count(const WplDrawList* list)
{
  if (list == NULL)
    return 0u;

  return list->count;
}

size_t
wpl_draw_list_capacity(const WplDrawList* list)
{
  if (list == NULL)
    return 0u;

  return list->capacity;
}

WplResult
wpl_draw_clear(WplDrawList* list, WplColor color)
{
  WplDrawCommand command = {0};
  WplResult result;

  if (list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_draw_validate_color(color);
  if (result != WPL_RESULT_OK)
    return result;

  command.type = WPL_DRAW_COMMAND_CLEAR;
  command.color = color;
  return wpl_draw_append_command(list, &command);
}

WplResult
wpl_draw_rect(WplDrawList* list, WplRect rect, WplColor color)
{
  WplDrawCommand command = {0};
  WplResult result;

  if (list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_draw_validate_rect(rect);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_validate_color(color);
  if (result != WPL_RESULT_OK)
    return result;

  command.type = WPL_DRAW_COMMAND_RECT;
  command.rect = rect;
  command.color = color;
  return wpl_draw_append_command(list, &command);
}

WplResult
wpl_draw_rounded_rect(WplDrawList* list,
                      WplRect rect,
                      float radius,
                      WplColor color)
{
  WplDrawCommand command = {0};
  WplResult result;

  if (list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_draw_validate_rect(rect);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_validate_radius(radius);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_validate_color(color);
  if (result != WPL_RESULT_OK)
    return result;

  command.type = WPL_DRAW_COMMAND_ROUNDED_RECT;
  command.rect = rect;
  command.radius = radius;
  command.color = color;
  return wpl_draw_append_command(list, &command);
}

WplResult
wpl_draw_panel(WplDrawList* list, WplRect rect, WplPanelStyle style)
{
  size_t count_before;
  size_t required_commands;
  WplRect inner;
  float inner_radius;
  WplResult result;

  if (list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_draw_validate_rect(rect);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_validate_color(style.fill_color);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_validate_color(style.border_color);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_validate_thickness(style.border_thickness);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_validate_radius(style.corner_radius);
  if (result != WPL_RESULT_OK)
    return result;

  required_commands = 1u;
  inner = rect;
  inner_radius = style.corner_radius;

  if (style.border_thickness > 0.0f)
    {
      float inset = style.border_thickness;
      inner.x = rect.x + inset;
      inner.y = rect.y + inset;
      inner.w = rect.w - (inset * 2.0f);
      inner.h = rect.h - (inset * 2.0f);
      inner_radius = style.corner_radius - inset;
      if (inner_radius < 0.0f)
        inner_radius = 0.0f;

      if (inner.w > 0.0f && inner.h > 0.0f)
        required_commands = 2u;
    }

  count_before = list->count;

  if (count_before > list->capacity)
    return WPL_RESULT_ERROR;

  if ((list->capacity - count_before) < required_commands)
    return WPL_RESULT_CAPACITY_EXCEEDED;

  if (style.border_thickness > 0.0f)
    result = wpl_draw_rounded_rect(list,
                                   rect,
                                   style.corner_radius,
                                   style.border_color);
  else
    result = wpl_draw_rounded_rect(list,
                                   rect,
                                   style.corner_radius,
                                   style.fill_color);

  if (result != WPL_RESULT_OK)
    {
      list->count = count_before;
      return result;
    }

  if (required_commands == 2u)
    {
      result = wpl_draw_rounded_rect(list,
                                     inner,
                                     inner_radius,
                                     style.fill_color);
      if (result != WPL_RESULT_OK)
        {
          list->count = count_before;
          return result;
        }
    }

  return WPL_RESULT_OK;
}

WplResult
wpl_draw_rect_outline(WplDrawList* list,
                      WplRect rect,
                      WplColor color,
                      float thickness)
{
  WplDrawCommand command = {0};
  WplResult result;

  if (list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_draw_validate_rect(rect);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_validate_color(color);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_validate_thickness(thickness);
  if (result != WPL_RESULT_OK)
    return result;

  command.type = WPL_DRAW_COMMAND_RECT_OUTLINE;
  command.rect = rect;
  command.color = color;
  command.thickness = thickness;
  return wpl_draw_append_command(list, &command);
}

WplResult
wpl_draw_line(WplDrawList* list,
              WplVec2 a,
              WplVec2 b,
              WplColor color,
              float thickness)
{
  WplDrawCommand command = {0};
  WplResult result;

  if (list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_draw_validate_line_geometry(a, b, thickness);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_validate_color(color);
  if (result != WPL_RESULT_OK)
    return result;

  command.type = WPL_DRAW_COMMAND_LINE;
  command.a = a;
  command.b = b;
  command.color = color;
  command.thickness = thickness;
  return wpl_draw_append_command(list, &command);
}

WplResult
wpl_draw_polyline(WplDrawList* list,
                  const WplVec2* points,
                  size_t point_count,
                  WplColor color,
                  float thickness)
{
  size_t count_before;
  size_t capacity;
  size_t required_segments;
  size_t i;
  WplResult result;

  if (list == NULL || points == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (point_count < 2u)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (!wpl_draw_points_are_finite(points, point_count))
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_draw_validate_color(color);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_validate_thickness(thickness);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_validate_polyline_geometry(points, point_count, thickness);
  if (result != WPL_RESULT_OK)
    return result;

  count_before = list->count;
  capacity = list->capacity;
  required_segments = point_count - 1u;

  if (count_before > capacity)
    return WPL_RESULT_ERROR;

  if ((capacity - count_before) < required_segments)
    return WPL_RESULT_CAPACITY_EXCEEDED;

  for (i = 0u; i + 1u < point_count; i++)
    {
      result = wpl_draw_line(list,
                             points[i],
                             points[i + 1u],
                             color,
                             thickness);
      if (result != WPL_RESULT_OK)
        {
          list->count = count_before;
          return result;
        }
    }

  return WPL_RESULT_OK;
}

WplResult
wpl_draw_dashed_line(WplDrawList* list,
                     WplVec2 a,
                     WplVec2 b,
                     WplColor color,
                     float thickness,
                     WplDashPattern pattern)
{
  size_t count_before;
  size_t capacity;
  size_t remaining_capacity;
  size_t required_segments = 0u;
  size_t appended_segments = 0u;
  float length;
  float period;
  float distance;
  WplResult result;

  if (list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_draw_validate_line_geometry(a, b, thickness);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_validate_color(color);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_validate_dash_pattern(pattern);
  if (result != WPL_RESULT_OK)
    return result;

  count_before = list->count;
  capacity = list->capacity;

  if (count_before > capacity)
    return WPL_RESULT_ERROR;

  remaining_capacity = capacity - count_before;

  result = wpl_draw_dashed_line_segment_count_limited(a,
                                                      b,
                                                      pattern,
                                                      remaining_capacity,
                                                      &required_segments);
  if (result != WPL_RESULT_OK)
    return result;

  if (required_segments == 0u)
    return WPL_RESULT_OK;

  length = wpl_draw_vec2_distance(a, b);
  if (!wpl_draw_float_is_finite(length))
    return WPL_RESULT_UNSUPPORTED;

  period = pattern.dash_length + pattern.gap_length;
  if (!wpl_draw_float_is_finite(period) || period <= 0.0f)
    return WPL_RESULT_UNSUPPORTED;

  distance = 0.0f;
  while (distance < length)
    {
      float dash_end = distance + pattern.dash_length;
      float next_distance = distance + period;
      float t0;
      float t1;
      WplVec2 p0;
      WplVec2 p1;

      if (!wpl_draw_float_is_finite(dash_end) || dash_end > length)
        dash_end = length;

      t0 = distance / length;
      t1 = dash_end / length;
      p0 = wpl_draw_vec2_lerp(a, b, t0);
      p1 = wpl_draw_vec2_lerp(a, b, t1);

      if (appended_segments >= required_segments)
        {
          list->count = count_before;
          return WPL_RESULT_ERROR;
        }

      result = wpl_draw_line(list, p0, p1, color, thickness);
      if (result != WPL_RESULT_OK)
        {
          list->count = count_before;
          return result;
        }

      appended_segments++;

      if (!wpl_draw_float_is_finite(next_distance) || next_distance <= distance)
        {
          list->count = count_before;
          return WPL_RESULT_UNSUPPORTED;
        }

      distance = next_distance;
    }

  if (appended_segments != required_segments)
    {
      list->count = count_before;
      return WPL_RESULT_ERROR;
    }

  return WPL_RESULT_OK;
}

WplResult
wpl_draw_push_clip(WplDrawList* list, WplRect rect)
{
  WplDrawCommand command = {0};
  WplResult result;

  if (list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_draw_validate_rect(rect);
  if (result != WPL_RESULT_OK)
    return result;

  command.type = WPL_DRAW_COMMAND_PUSH_CLIP;
  command.rect = rect;

  result = wpl_draw_append_command(list, &command);
  if (result != WPL_RESULT_OK)
    return result;

  list->clip_depth++;
  return WPL_RESULT_OK;
}

WplResult
wpl_draw_pop_clip(WplDrawList* list)
{
  WplDrawCommand command = {0};
  WplResult result;

  if (list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (list->clip_depth == 0u)
    return WPL_RESULT_INVALID_ARGUMENT;

  command.type = WPL_DRAW_COMMAND_POP_CLIP;

  result = wpl_draw_append_command(list, &command);
  if (result != WPL_RESULT_OK)
    return result;

  list->clip_depth--;
  return WPL_RESULT_OK;
}

WplResult
wpl_draw_circle(WplDrawList* list,
                WplVec2 center,
                float radius,
                WplColor color)
{
  WplDrawCommand command = {0};
  WplResult result;

  if (list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_draw_validate_circle_geometry(center, radius);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_validate_color(color);
  if (result != WPL_RESULT_OK)
    return result;

  command.type = WPL_DRAW_COMMAND_CIRCLE;
  command.a = center;
  command.radius = radius;
  command.color = color;
  return wpl_draw_append_command(list, &command);
}

WplResult
wpl_draw_text(WplDrawList* list,
              WplVec2 position,
              const char* text,
              WplColor color)
{
  WplDrawCommand command = {0};
  WplResult result;

  if (list == NULL || text == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (!wpl_draw_vec2_is_finite(position))
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_draw_validate_color(color);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_copy_text(command.text, text);
  if (result != WPL_RESULT_OK)
    return result;

  command.type = WPL_DRAW_COMMAND_TEXT;
  command.a = position;
  command.color = color;
  return wpl_draw_append_command(list, &command);
}
