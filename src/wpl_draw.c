/* wpl_draw.c - Fixed-capacity draw command buffer implementation. */

#include "wpl_draw_internal.h"

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

static WplResult
wpl_draw_validate_rect(WplRect rect)
{
  if (!wpl_draw_rect_is_finite(rect))
    return WPL_RESULT_INVALID_ARGUMENT;

  if (rect.w < 0.0f || rect.h < 0.0f)
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
wpl_draw_copy_text(char dst[256], const char* src)
{
  size_t i = 0u;

  if (dst == NULL || src == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  while (i < 256u && src[i] != '\0')
    {
      if (i == 255u)
        return WPL_RESULT_TRUNCATED;

      dst[i] = src[i];
      i++;
    }

  dst[i] = '\0';
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

  if (!wpl_draw_vec2_is_finite(a) || !wpl_draw_vec2_is_finite(b))
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_draw_validate_color(color);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_draw_validate_thickness(thickness);
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
wpl_draw_circle(WplDrawList* list,
                WplVec2 center,
                float radius,
                WplColor color)
{
  WplDrawCommand command = {0};
  WplResult result;

  if (list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (!wpl_draw_vec2_is_finite(center))
    return WPL_RESULT_INVALID_ARGUMENT;

  if (!wpl_draw_float_is_finite(radius) || radius < 0.0f)
    return WPL_RESULT_INVALID_ARGUMENT;

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
