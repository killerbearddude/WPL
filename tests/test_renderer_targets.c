/* test_renderer_targets.c - Renderer target resize/presentation boundary tests. */

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
wpl_test_prepare_window(WplWindow* window,
                        int width,
                        int height,
                        uint32_t* framebuffer,
                        int framebuffer_width,
                        int framebuffer_height,
                        XImage* ximage)
{
  assert(window != NULL);
  assert(ximage != NULL);

  memset(window, 0, sizeof(*window));
  memset(ximage, 0, sizeof(*ximage));

  ximage->width = width;
  ximage->height = height;

  window->width = width;
  window->height = height;
  window->framebuffer = framebuffer;
  window->framebuffer_width = framebuffer_width;
  window->framebuffer_height = framebuffer_height;
  window->ximage = ximage;
}

static void
wpl_test_release_private_allocations(WplWindow* window)
{
  if (window == NULL)
    return;

  free(window->framebuffer);
  window->framebuffer = NULL;
  window->framebuffer_width = 0;
  window->framebuffer_height = 0;

  free(window->clip_stack);
  window->clip_stack = NULL;
  window->clip_stack_capacity = 0u;
  window->clip_stack_depth = 0u;

  window->ximage = NULL;
  window->ximage_pixels = NULL;
}

static void
test_submit_reuses_matching_manual_targets(void)
{
  WplWindow window;
  XImage ximage;
  uint32_t framebuffer[4] = {0u, 0u, 0u, 0u};
  WplDrawList* list = wpl_test_create_list(1u);
  size_t i;

  wpl_test_prepare_window(&window, 2, 2, framebuffer, 2, 2, &ximage);

  assert(wpl_draw_clear(list, wpl_test_color(1.0f, 0.0f, 0.0f, 1.0f))
         == WPL_RESULT_OK);
  assert(wpl_submit_draw_list(&window, list) == WPL_RESULT_OK);

  assert(window.framebuffer == framebuffer);
  assert(window.framebuffer_width == 2);
  assert(window.framebuffer_height == 2);
  assert(window.ximage == &ximage);
  assert(window.active_clip.x0 == 0);
  assert(window.active_clip.y0 == 0);
  assert(window.active_clip.x1 == 2);
  assert(window.active_clip.y1 == 2);

  for (i = 0u; i < 4u; i++)
    assert(framebuffer[i] == 0xffff0000u);

  wpl_destroy_draw_list(list);
  free(window.clip_stack);
}

static void
test_submit_resizes_framebuffer_to_window_dimensions(void)
{
  WplWindow window;
  XImage ximage;
  uint32_t* old_framebuffer = (uint32_t*)malloc(sizeof(uint32_t));
  WplDrawList* list = wpl_test_create_list(1u);
  size_t i;

  assert(old_framebuffer != NULL);
  old_framebuffer[0] = 0xdeadbeefu;

  wpl_test_prepare_window(&window, 3, 2, old_framebuffer, 1, 1, &ximage);

  assert(wpl_draw_clear(list, wpl_test_color(0.0f, 1.0f, 0.0f, 1.0f))
         == WPL_RESULT_OK);
  assert(wpl_submit_draw_list(&window, list) == WPL_RESULT_OK);

  assert(window.framebuffer != NULL);
  assert(window.framebuffer_width == 3);
  assert(window.framebuffer_height == 2);
  assert(window.ximage == &ximage);
  assert(window.active_clip.x0 == 0);
  assert(window.active_clip.y0 == 0);
  assert(window.active_clip.x1 == 3);
  assert(window.active_clip.y1 == 2);

  for (i = 0u; i < 6u; i++)
    assert(window.framebuffer[i] == 0xff00ff00u);

  wpl_destroy_draw_list(list);
  wpl_test_release_private_allocations(&window);
}

static void
test_submit_zero_sized_window_is_target_noop(void)
{
  WplWindow window;
  XImage ximage;
  uint32_t framebuffer[1] = {0x12345678u};
  WplDrawList* list = wpl_test_create_list(1u);

  wpl_test_prepare_window(&window, 0, 4, framebuffer, 1, 1, &ximage);

  assert(wpl_draw_clear(list, wpl_test_color(1.0f, 1.0f, 1.0f, 1.0f))
         == WPL_RESULT_OK);
  assert(wpl_submit_draw_list(&window, list) == WPL_RESULT_OK);

  assert(window.framebuffer == framebuffer);
  assert(window.framebuffer_width == 1);
  assert(window.framebuffer_height == 1);
  assert(window.ximage == &ximage);
  assert(framebuffer[0] == 0x12345678u);

  wpl_destroy_draw_list(list);
}

static void
test_end_frame_rejects_null_and_noops_zero_size(void)
{
  WplWindow window;
  XImage ximage;
  uint32_t framebuffer[1] = {0x11111111u};
  uint32_t ximage_pixels[1] = {0x22222222u};

  assert(wpl_end_frame(NULL) == WPL_RESULT_INVALID_ARGUMENT);

  wpl_test_prepare_window(&window, 0, 4, framebuffer, 1, 1, &ximage);
  window.ximage_pixels = ximage_pixels;

  assert(wpl_end_frame(&window) == WPL_RESULT_OK);
  assert(window.framebuffer == framebuffer);
  assert(window.ximage == &ximage);
  assert(window.ximage_pixels == ximage_pixels);
  assert(framebuffer[0] == 0x11111111u);
  assert(ximage_pixels[0] == 0x22222222u);
}

static void
test_end_frame_positive_no_display_reports_invalid_argument(void)
{
  WplWindow window;

  memset(&window, 0, sizeof(window));
  window.width = 2;
  window.height = 2;

  assert(wpl_end_frame(&window) == WPL_RESULT_INVALID_ARGUMENT);
  assert(window.ximage == NULL);

  wpl_test_release_private_allocations(&window);
}

int
main(void)
{
  test_submit_reuses_matching_manual_targets();
  test_submit_resizes_framebuffer_to_window_dimensions();
  test_submit_zero_sized_window_is_target_noop();
  test_end_frame_rejects_null_and_noops_zero_size();
  test_end_frame_positive_no_display_reports_invalid_argument();
  return 0;
}
