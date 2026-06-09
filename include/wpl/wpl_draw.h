#ifndef WPL_DRAW_H
#define WPL_DRAW_H

#include <stddef.h>

#include "wpl_base.h"
#include "wpl_result.h"
#include "wpl_window.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque fixed-capacity draw command list. */
typedef struct WplDrawList WplDrawList;

WplResult wpl_create_draw_list(size_t max_commands, WplDrawList** out_list);
void wpl_destroy_draw_list(WplDrawList* list);
WplResult wpl_draw_list_clear(WplDrawList* list);
size_t wpl_draw_list_count(const WplDrawList* list);
size_t wpl_draw_list_capacity(const WplDrawList* list);

WplResult wpl_draw_clear(WplDrawList* list, WplColor color);
WplResult wpl_draw_rect(WplDrawList* list, WplRect rect, WplColor color);
WplResult wpl_draw_rect_outline(WplDrawList* list,
                                WplRect rect,
                                WplColor color,
                                float thickness);
WplResult wpl_draw_line(WplDrawList* list,
                        WplVec2 a,
                        WplVec2 b,
                        WplColor color,
                        float thickness);
WplResult wpl_draw_circle(WplDrawList* list,
                          WplVec2 center,
                          float radius,
                          WplColor color);
WplResult wpl_draw_text(WplDrawList* list,
                        WplVec2 position,
                        const char* text,
                        WplColor color);

WplResult wpl_submit_draw_list(WplWindow* window, const WplDrawList* list);

#ifdef __cplusplus
}
#endif

#endif /* WPL_DRAW_H */
