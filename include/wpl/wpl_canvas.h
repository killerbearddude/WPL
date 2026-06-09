#ifndef WPL_CANVAS_H
#define WPL_CANVAS_H

#include "wpl_base.h"
#include "wpl_result.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WplCanvasView {
  WplVec2 pan;
  float zoom;
  float min_zoom;
  float max_zoom;
} WplCanvasView;

WplResult wpl_screen_to_canvas(const WplCanvasView* view,
                               WplVec2 screen,
                               WplVec2* out_canvas);
WplResult wpl_canvas_to_screen(const WplCanvasView* view,
                               WplVec2 canvas,
                               WplVec2* out_screen);
WplResult wpl_canvas_pan_by(WplCanvasView* view, WplVec2 screen_delta);
WplResult wpl_canvas_zoom_around(WplCanvasView* view,
                                 WplVec2 cursor_screen,
                                 float zoom_factor);

#ifdef __cplusplus
}
#endif

#endif /* WPL_CANVAS_H */
