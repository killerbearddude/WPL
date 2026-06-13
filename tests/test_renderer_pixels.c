/* test_renderer_pixels.c - Software renderer framebuffer/color/clip tests. */

#include <wpl/wpl.h>

#include "wpl_linux_x11_internal.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static WplColor
wpl_test_color(float r, float g, float b, float a)
{
  WplColor color = {r, g, b, a};
  return color;
}

static WplRect
wpl_test_rect(float x, float y, float w, float h)
{
  WplRect rect = {x, y, w, h};
  return rect;
}

static WplDrawList*
wpl_test_create_list(size_t capacity)
{
  WplDrawList* list = NULL;
  WplResult result = wpl_create_draw_list(capacity, &list);

  assert(result == WPL_RESULT_OK);
  assert(list != NULL);
  return list;
}

static void
wpl_test_prepare_manual_window(WplWindow* window,
                               int width,
                               int height,
                               uint32_t* framebuffer,
                               XImage* ximage)
{
  assert(window != NULL);
  assert(framebuffer != NULL);
  assert(ximage != NULL);

  memset(window, 0, sizeof(*window));
  memset(ximage, 0, sizeof(*ximage));

  ximage->width = width;
  ximage->height = height;

  window->width = width;
  window->height = height;
  window->framebuffer = framebuffer;
  window->framebuffer_width = width;
  window->framebuffer_height = height;
  window->ximage = ximage;
}

static void
wpl_test_clear_manual_window(WplWindow* window)
{
  if (window == NULL)
    return;

  free(window->clip_stack);
  window->clip_stack = NULL;
  window->clip_stack_capacity = 0u;
  window->clip_stack_depth = 0u;
  window->ximage = NULL;
  window->framebuffer = NULL;
}

static uint32_t
wpl_test_pixel_at(const uint32_t* framebuffer, int width, int x, int y)
{
  assert(framebuffer != NULL);
  assert(width > 0);
  assert(x >= 0);
  assert(y >= 0);

  return framebuffer[((size_t)y * (size_t)width) + (size_t)x];
}

static void
test_clear_clamps_color_channels(void)
{
  WplWindow window;
  XImage ximage;
  uint32_t framebuffer[4] = {0u, 0u, 0u, 0u};
  WplDrawList* list = wpl_test_create_list(1u);
  size_t i;

  wpl_test_prepare_manual_window(&window, 2, 2, framebuffer, &ximage);

  assert(wpl_draw_clear(list, wpl_test_color(-1.0f, 0.5f, 2.0f, 1.0f))
         == WPL_RESULT_OK);
  assert(wpl_submit_draw_list(&window, list) == WPL_RESULT_OK);

  for (i = 0u; i < 4u; i++)
    assert(framebuffer[i] == 0xff0080ffu);

  wpl_destroy_draw_list(list);
  wpl_test_clear_manual_window(&window);
}

static void
test_alpha_blending_edges(void)
{
  WplWindow window;
  XImage ximage;
  uint32_t framebuffer[4] = {0u, 0u, 0u, 0u};
  WplDrawList* list = wpl_test_create_list(4u);
  size_t i;

  wpl_test_prepare_manual_window(&window, 2, 2, framebuffer, &ximage);

  assert(wpl_draw_clear(list, wpl_test_color(0.0f, 0.0f, 0.0f, 1.0f))
         == WPL_RESULT_OK);
  assert(wpl_draw_rect(list,
                       wpl_test_rect(0.0f, 0.0f, 2.0f, 2.0f),
                       wpl_test_color(1.0f, 1.0f, 1.0f, 0.5f))
         == WPL_RESULT_OK);
  assert(wpl_submit_draw_list(&window, list) == WPL_RESULT_OK);

  for (i = 0u; i < 4u; i++)
    assert(framebuffer[i] == 0xff808080u);

  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  assert(wpl_draw_clear(list, wpl_test_color(1.0f, 0.0f, 0.0f, 1.0f))
         == WPL_RESULT_OK);
  assert(wpl_draw_rect(list,
                       wpl_test_rect(0.0f, 0.0f, 2.0f, 2.0f),
                       wpl_test_color(0.0f, 1.0f, 0.0f, 0.0f))
         == WPL_RESULT_OK);
  assert(wpl_submit_draw_list(&window, list) == WPL_RESULT_OK);

  for (i = 0u; i < 4u; i++)
    assert(framebuffer[i] == 0xffff0000u);

  wpl_destroy_draw_list(list);
  wpl_test_clear_manual_window(&window);
}

static void
test_rect_bounds_are_clamped_to_framebuffer(void)
{
  WplWindow window;
  XImage ximage;
  uint32_t framebuffer[4] = {0u, 0u, 0u, 0u};
  WplDrawList* list = wpl_test_create_list(2u);

  wpl_test_prepare_manual_window(&window, 2, 2, framebuffer, &ximage);

  assert(wpl_draw_clear(list, wpl_test_color(0.0f, 0.0f, 0.0f, 1.0f))
         == WPL_RESULT_OK);
  assert(wpl_draw_rect(list,
                       wpl_test_rect(-1.0f, -1.0f, 2.0f, 2.0f),
                       wpl_test_color(0.0f, 1.0f, 0.0f, 1.0f))
         == WPL_RESULT_OK);
  assert(wpl_submit_draw_list(&window, list) == WPL_RESULT_OK);

  assert(wpl_test_pixel_at(framebuffer, 2, 0, 0) == 0xff00ff00u);
  assert(wpl_test_pixel_at(framebuffer, 2, 1, 0) == 0xff000000u);
  assert(wpl_test_pixel_at(framebuffer, 2, 0, 1) == 0xff000000u);
  assert(wpl_test_pixel_at(framebuffer, 2, 1, 1) == 0xff000000u);

  wpl_destroy_draw_list(list);
  wpl_test_clear_manual_window(&window);
}

static void
test_clip_restricts_rect_raster_output(void)
{
  WplWindow window;
  XImage ximage;
  uint32_t framebuffer[16];
  WplDrawList* list = wpl_test_create_list(4u);
  int x;
  int y;

  for (y = 0; y < 4; y++)
    {
      for (x = 0; x < 4; x++)
        framebuffer[((size_t)y * 4u) + (size_t)x] = 0u;
    }

  wpl_test_prepare_manual_window(&window, 4, 4, framebuffer, &ximage);

  assert(wpl_draw_clear(list, wpl_test_color(0.0f, 0.0f, 0.0f, 1.0f))
         == WPL_RESULT_OK);
  assert(wpl_draw_push_clip(list, wpl_test_rect(1.0f, 1.0f, 2.0f, 2.0f))
         == WPL_RESULT_OK);
  assert(wpl_draw_rect(list,
                       wpl_test_rect(0.0f, 0.0f, 4.0f, 4.0f),
                       wpl_test_color(0.0f, 0.0f, 1.0f, 1.0f))
         == WPL_RESULT_OK);
  assert(wpl_draw_pop_clip(list) == WPL_RESULT_OK);
  assert(wpl_submit_draw_list(&window, list) == WPL_RESULT_OK);

  for (y = 0; y < 4; y++)
    {
      for (x = 0; x < 4; x++)
        {
          uint32_t expected = (x >= 1 && x < 3 && y >= 1 && y < 3)
                              ? 0xff0000ffu
                              : 0xff000000u;
          assert(wpl_test_pixel_at(framebuffer, 4, x, y) == expected);
        }
    }

  assert(window.clip_stack_depth == 0u);
  assert(window.active_clip.x0 == 0);
  assert(window.active_clip.y0 == 0);
  assert(window.active_clip.x1 == 4);
  assert(window.active_clip.y1 == 4);

  wpl_destroy_draw_list(list);
  wpl_test_clear_manual_window(&window);
}

static void
test_empty_clip_suppresses_output(void)
{
  WplWindow window;
  XImage ximage;
  uint32_t framebuffer[4] = {0u, 0u, 0u, 0u};
  WplDrawList* list = wpl_test_create_list(4u);
  size_t i;

  wpl_test_prepare_manual_window(&window, 2, 2, framebuffer, &ximage);

  assert(wpl_draw_clear(list, wpl_test_color(0.0f, 0.0f, 0.0f, 1.0f))
         == WPL_RESULT_OK);
  assert(wpl_draw_push_clip(list, wpl_test_rect(1.0f, 1.0f, 0.0f, 0.0f))
         == WPL_RESULT_OK);
  assert(wpl_draw_rect(list,
                       wpl_test_rect(0.0f, 0.0f, 2.0f, 2.0f),
                       wpl_test_color(1.0f, 0.0f, 0.0f, 1.0f))
         == WPL_RESULT_OK);
  assert(wpl_draw_pop_clip(list) == WPL_RESULT_OK);
  assert(wpl_submit_draw_list(&window, list) == WPL_RESULT_OK);

  for (i = 0u; i < 4u; i++)
    assert(framebuffer[i] == 0xff000000u);

  wpl_destroy_draw_list(list);
  wpl_test_clear_manual_window(&window);
}

int
main(void)
{
  test_clear_clamps_color_channels();
  test_alpha_blending_edges();
  test_rect_bounds_are_clamped_to_framebuffer();
  test_clip_restricts_rect_raster_output();
  test_empty_clip_suppresses_output();
  return 0;
}
