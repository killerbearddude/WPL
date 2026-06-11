/* wpl_debug.c - append-only debug overlay command generation. */

#include "wpl/wpl_debug.h"

#include "wpl_draw_internal.h"

#include <stdarg.h>
#include <stdio.h>

#define WPL_DEBUG_BASE_COMMAND_COUNT 10u
#define WPL_DEBUG_MAX_EXTRA_LINES 1000000u
#define WPL_DEBUG_X 8.0f
#define WPL_DEBUG_Y 8.0f
#define WPL_DEBUG_LINE_H 10.0f
#define WPL_DEBUG_WIDTH 300.0f
#define WPL_DEBUG_HEIGHT 112.0f

static const WplColor wpl_debug_background_color = {0.0f, 0.0f, 0.0f, 0.70f};
static const WplColor wpl_debug_title_color = {1.0f, 1.0f, 1.0f, 1.0f};
static const WplColor wpl_debug_text_color = {0.90f, 0.92f, 0.95f, 1.0f};

static void
wpl_debug_restore_draw_count(WplDrawList* list, size_t count)
{
  if (list != NULL)
    list->count = count;
}

static WplResult
wpl_debug_require_capacity(WplDrawList* list, size_t command_count)
{
  size_t count = wpl_draw_list_count(list);
  size_t capacity = wpl_draw_list_capacity(list);

  if (count > capacity)
    return WPL_RESULT_ERROR;

  if ((capacity - count) < command_count)
    return WPL_RESULT_CAPACITY_EXCEEDED;

  return WPL_RESULT_OK;
}

static WplResult
wpl_debug_overlay_height(size_t extra_line_count, float* out_height)
{
  if (out_height == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (extra_line_count > WPL_DEBUG_MAX_EXTRA_LINES)
    return WPL_RESULT_UNSUPPORTED;

  *out_height = WPL_DEBUG_HEIGHT
                + ((float)extra_line_count * WPL_DEBUG_LINE_H);
  return WPL_RESULT_OK;
}

static WplResult
wpl_debug_validate_custom_lines(const WplDebugLine* lines, size_t line_count)
{
  size_t i;

  if (lines == NULL && line_count > 0u)
    return WPL_RESULT_INVALID_ARGUMENT;

  for (i = 0u; i < line_count; i++)
    {
      char line[128];
      int written = 0;

      if (lines[i].label == NULL || lines[i].value == NULL)
        return WPL_RESULT_INVALID_ARGUMENT;

      written = snprintf(line,
                         sizeof(line),
                         "%s: %s",
                         lines[i].label,
                         lines[i].value);
      if (written < 0)
        return WPL_RESULT_ERROR;

      if ((size_t)written >= sizeof(line))
        return WPL_RESULT_TRUNCATED;
    }

  return WPL_RESULT_OK;
}

static WplResult
wpl_debug_append_text(WplDrawList* list,
                      float x,
                      float y,
                      const char* text,
                      WplColor color)
{
  return wpl_draw_text(list, (WplVec2){x, y}, text, color);
}

static WplResult
wpl_debug_append_format_line(WplDrawList* list,
                             float x,
                             float y,
                             WplColor color,
                             const char* format,
                             ...)
{
  char line[128];
  va_list args;
  int written = 0;

  va_start(args, format);
  written = vsnprintf(line, sizeof(line), format, args);
  va_end(args);

  if (written < 0)
    return WPL_RESULT_ERROR;

  if ((size_t)written >= sizeof(line))
    return WPL_RESULT_TRUNCATED;

  return wpl_debug_append_text(list, x, y, line, color);
}

WplResult
wpl_debug_draw_overlay(WplDrawList* list,
                       const WplDebugStats* stats,
                       const WplInputState* input)
{
  return wpl_debug_draw_overlay_ex(list, stats, input, NULL, 0u);
}

WplResult
wpl_debug_draw_overlay_ex(WplDrawList* list,
                          const WplDebugStats* stats,
                          const WplInputState* input,
                          const WplDebugLine* lines,
                          size_t line_count)
{
  const char* backend_name = NULL;
  size_t count_before = 0;
  size_t command_count = 0;
  size_t i = 0u;
  float x = WPL_DEBUG_X;
  float y = WPL_DEBUG_Y;
  float background_height = WPL_DEBUG_HEIGHT;
  WplResult result = WPL_RESULT_OK;

  if (list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (stats == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (input == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  result = wpl_debug_validate_custom_lines(lines, line_count);
  if (result != WPL_RESULT_OK)
    return result;

  if (line_count > ((size_t)-1) - WPL_DEBUG_BASE_COMMAND_COUNT)
    return WPL_RESULT_UNSUPPORTED;

  command_count = WPL_DEBUG_BASE_COMMAND_COUNT + line_count;

  result = wpl_debug_require_capacity(list, command_count);
  if (result != WPL_RESULT_OK)
    return result;

  result = wpl_debug_overlay_height(line_count, &background_height);
  if (result != WPL_RESULT_OK)
    return result;

  backend_name = (stats->backend_name != NULL) ? stats->backend_name : "unknown";
  count_before = wpl_draw_list_count(list);

#define WPL_DEBUG_APPEND(call_expr)                       \
  do {                                                    \
    result = (call_expr);                                 \
    if (result != WPL_RESULT_OK) {                        \
      wpl_debug_restore_draw_count(list, count_before);   \
      return result;                                      \
    }                                                     \
  } while (0)

  WPL_DEBUG_APPEND(wpl_draw_rect(list,
                                 (WplRect){WPL_DEBUG_X,
                                           WPL_DEBUG_Y,
                                           WPL_DEBUG_WIDTH,
                                           background_height},
                                 wpl_debug_background_color));

  WPL_DEBUG_APPEND(wpl_debug_append_text(list,
                                         x,
                                         y,
                                         "WPL debug",
                                         wpl_debug_title_color));
  y += WPL_DEBUG_LINE_H;

  WPL_DEBUG_APPEND(wpl_debug_append_format_line(list,
                                                x,
                                                y,
                                                wpl_debug_text_color,
                                                "Backend: %s",
                                                backend_name));
  y += WPL_DEBUG_LINE_H;

  WPL_DEBUG_APPEND(wpl_debug_append_format_line(list,
                                                x,
                                                y,
                                                wpl_debug_text_color,
                                                "FPS: %.1f  Frame: %.2f ms",
                                                stats->fps,
                                                stats->frame_ms));
  y += WPL_DEBUG_LINE_H;

  WPL_DEBUG_APPEND(wpl_debug_append_format_line(list,
                                                x,
                                                y,
                                                wpl_debug_text_color,
                                                "Window: %d x %d",
                                                stats->window_width,
                                                stats->window_height));
  y += WPL_DEBUG_LINE_H;

  WPL_DEBUG_APPEND(wpl_debug_append_format_line(list,
                                                x,
                                                y,
                                                wpl_debug_text_color,
                                                "Mouse: %.1f, %.1f",
                                                input->mouse.position.x,
                                                input->mouse.position.y));
  y += WPL_DEBUG_LINE_H;

  WPL_DEBUG_APPEND(wpl_debug_append_format_line(list,
                                                x,
                                                y,
                                                wpl_debug_text_color,
                                                "Delta: %.1f, %.1f",
                                                input->mouse.delta.x,
                                                input->mouse.delta.y));
  y += WPL_DEBUG_LINE_H;

  WPL_DEBUG_APPEND(wpl_debug_append_format_line(
    list,
    x,
    y,
    wpl_debug_text_color,
    "Buttons: L=%d M=%d R=%d",
    input->mouse.button_down[WPL_MOUSE_BUTTON_LEFT] ? 1 : 0,
    input->mouse.button_down[WPL_MOUSE_BUTTON_MIDDLE] ? 1 : 0,
    input->mouse.button_down[WPL_MOUSE_BUTTON_RIGHT] ? 1 : 0));
  y += WPL_DEBUG_LINE_H;

  WPL_DEBUG_APPEND(wpl_debug_append_format_line(list,
                                                x,
                                                y,
                                                wpl_debug_text_color,
                                                "Wheel: %.1f",
                                                input->mouse.wheel_delta));
  y += WPL_DEBUG_LINE_H;

  WPL_DEBUG_APPEND(wpl_debug_append_format_line(list,
                                                x,
                                                y,
                                                wpl_debug_text_color,
                                                "Draw commands: %zu",
                                                stats->draw_command_count));
  y += WPL_DEBUG_LINE_H;

  for (i = 0u; i < line_count; i++)
    {
      WPL_DEBUG_APPEND(wpl_debug_append_format_line(list,
                                                    x,
                                                    y,
                                                    wpl_debug_text_color,
                                                    "%s: %s",
                                                    lines[i].label,
                                                    lines[i].value));
      y += WPL_DEBUG_LINE_H;
    }

#undef WPL_DEBUG_APPEND

  return WPL_RESULT_OK;
}
