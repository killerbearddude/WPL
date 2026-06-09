/* wpl_draw.c - fixed-capacity draw list API stubs. */

#include "wpl/wpl_draw.h"

WplResult
wpl_create_draw_list(size_t max_commands, WplDrawList** out_list)
{
  (void)max_commands;

  if (out_list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  *out_list = NULL;
  return WPL_RESULT_UNSUPPORTED;
}

void
wpl_destroy_draw_list(WplDrawList* list)
{
  (void)list;
}

WplResult
wpl_draw_list_clear(WplDrawList* list)
{
  if (list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_UNSUPPORTED;
}

size_t
wpl_draw_list_count(const WplDrawList* list)
{
  (void)list;
  return 0u;
}

size_t
wpl_draw_list_capacity(const WplDrawList* list)
{
  (void)list;
  return 0u;
}

WplResult
wpl_draw_clear(WplDrawList* list, WplColor color)
{
  (void)color;

  if (list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_UNSUPPORTED;
}

WplResult
wpl_draw_rect(WplDrawList* list, WplRect rect, WplColor color)
{
  (void)rect;
  (void)color;

  if (list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_UNSUPPORTED;
}

WplResult
wpl_draw_rect_outline(WplDrawList* list,
                      WplRect rect,
                      WplColor color,
                      float thickness)
{
  (void)rect;
  (void)color;
  (void)thickness;

  if (list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_UNSUPPORTED;
}

WplResult
wpl_draw_line(WplDrawList* list,
              WplVec2 a,
              WplVec2 b,
              WplColor color,
              float thickness)
{
  (void)a;
  (void)b;
  (void)color;
  (void)thickness;

  if (list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_UNSUPPORTED;
}

WplResult
wpl_draw_circle(WplDrawList* list,
                WplVec2 center,
                float radius,
                WplColor color)
{
  (void)center;
  (void)radius;
  (void)color;

  if (list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_UNSUPPORTED;
}

WplResult
wpl_draw_text(WplDrawList* list,
              WplVec2 position,
              const char* text,
              WplColor color)
{
  (void)position;
  (void)color;

  if (list == NULL || text == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  return WPL_RESULT_UNSUPPORTED;
}
