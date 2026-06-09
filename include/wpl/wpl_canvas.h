#ifndef WPL_CANVAS_H
#define WPL_CANVAS_H

#include "wpl_base.h"
#include "wpl_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Canvas view transform state.

   pan is a screen-space pixel offset.  zoom is pixels per canvas unit. */
typedef struct WplCanvasView {
  WplVec2 pan;
  float zoom;
  float min_zoom;
  float max_zoom;
} WplCanvasView;

/* Convert SCREEN framebuffer coordinates to canvas coordinates. */
WplResult wpl_screen_to_canvas(const WplCanvasView* view,
                               WplVec2 screen,
                               WplVec2* out_canvas);

/* Convert CANVAS coordinates to screen framebuffer coordinates. */
WplResult wpl_canvas_to_screen(const WplCanvasView* view,
                               WplVec2 canvas,
                               WplVec2* out_screen);

/* Pan the view by a screen-space pixel delta. */
WplResult wpl_canvas_pan_by(WplCanvasView* view, WplVec2 screen_delta);

/* Zoom around CURSOR_SCREEN while preserving the canvas point under it. */
WplResult wpl_canvas_zoom_around(WplCanvasView* view,
                                 WplVec2 cursor_screen,
                                 float zoom_factor);

/* Return true when POINT lies inside RECT.  Left/top edges are inclusive;
   right/bottom edges are exclusive.  Negative-size rectangles are empty. */
bool wpl_rect_contains_point(WplRect rect, WplVec2 point);

/* Return true when A and B have positive-area overlap.  Touching edges do
   not count as intersection. */
bool wpl_rect_intersects(WplRect a, WplRect b);

/* Return the positive-area intersection of A and B, or a zero rectangle when
   no positive-area overlap exists. */
WplRect wpl_rect_intersection(WplRect a, WplRect b);

#ifdef __cplusplus
}
#endif

#endif /* WPL_CANVAS_H */
