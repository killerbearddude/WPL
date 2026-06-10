#ifndef WPL_DRAW_INTERNAL_H
#define WPL_DRAW_INTERNAL_H

/* wpl_draw_internal.h - Private draw command storage for core/renderers.

   This header is deliberately outside include/wpl.  It lets WPL implementation
   files execute draw lists without exposing command storage as public API. */

#include <stddef.h>

#include "wpl/wpl_draw.h"

typedef enum WplDrawCommandType
{
  WPL_DRAW_COMMAND_CLEAR = 0,
  WPL_DRAW_COMMAND_RECT,
  WPL_DRAW_COMMAND_RECT_OUTLINE,
  WPL_DRAW_COMMAND_LINE,
  WPL_DRAW_COMMAND_CIRCLE,
  WPL_DRAW_COMMAND_TEXT
} WplDrawCommandType;

typedef struct WplDrawCommand
{
  WplDrawCommandType type;

  WplRect rect;
  WplVec2 a;
  WplVec2 b;

  WplColor color;

  float thickness;
  float radius;

  char text[WPL_DRAW_TEXT_MAX_BYTES + 1u];
} WplDrawCommand;

struct WplDrawList
{
  WplDrawCommand* commands;
  size_t count;
  size_t capacity;
};

#endif /* WPL_DRAW_INTERNAL_H */
