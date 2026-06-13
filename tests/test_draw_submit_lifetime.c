/* test_draw_submit_lifetime.c - Draw submission/lifetime boundary tests. */

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

static void
test_submit_rejects_null_arguments(void)
{
  WplWindow window = {0};
  WplDrawList* list = wpl_test_create_list(1u);

  assert(wpl_submit_draw_list(NULL, NULL) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_submit_draw_list(&window, NULL) == WPL_RESULT_INVALID_ARGUMENT);
  assert(wpl_submit_draw_list(NULL, list) == WPL_RESULT_INVALID_ARGUMENT);

  wpl_destroy_draw_list(list);
}

static void
test_zero_sized_window_submit_is_noop_and_keeps_list_owned(void)
{
  WplWindow window = {0};
  WplDrawList* list = wpl_test_create_list(4u);
  size_t capacity;

  window.width = 0;
  window.height = 8;

  assert(wpl_draw_clear(list, wpl_test_color(0.25f, 0.5f, 0.75f, 1.0f))
         == WPL_RESULT_OK);
  assert(wpl_draw_rect(list,
                       wpl_test_rect(0.0f, 0.0f, 1.0f, 1.0f),
                       wpl_test_color(1.0f, 0.0f, 0.0f, 1.0f))
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 2u);
  capacity = wpl_draw_list_capacity(list);

  assert(wpl_submit_draw_list(&window, list) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 2u);
  assert(wpl_draw_list_capacity(list) == capacity);
  assert(window.framebuffer == NULL);
  assert(window.ximage == NULL);
  assert(window.clip_stack == NULL);

  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 0u);
  assert(wpl_draw_rect(list,
                       wpl_test_rect(1.0f, 1.0f, 2.0f, 2.0f),
                       wpl_test_color(0.0f, 1.0f, 0.0f, 1.0f))
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);

  window.width = 8;
  window.height = 0;
  assert(wpl_submit_draw_list(&window, list) == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);

  wpl_destroy_draw_list(list);
}

static void
test_manual_render_submit_keeps_draw_list_reusable(void)
{
  WplWindow window;
  XImage ximage;
  uint32_t framebuffer[4] = {0u, 0u, 0u, 0u};
  WplDrawList* list = wpl_test_create_list(4u);
  size_t capacity;

  wpl_test_prepare_manual_window(&window, 2, 2, framebuffer, &ximage);

  assert(wpl_draw_clear(list, wpl_test_color(1.0f, 0.0f, 0.0f, 1.0f))
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);
  capacity = wpl_draw_list_capacity(list);

  assert(wpl_submit_draw_list(&window, list) == WPL_RESULT_OK);
  assert(framebuffer[0] == 0xffff0000u);
  assert(framebuffer[1] == 0xffff0000u);
  assert(framebuffer[2] == 0xffff0000u);
  assert(framebuffer[3] == 0xffff0000u);
  assert(wpl_draw_list_count(list) == 1u);
  assert(wpl_draw_list_capacity(list) == capacity);

  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  assert(wpl_draw_rect(list,
                       wpl_test_rect(0.0f, 0.0f, 1.0f, 1.0f),
                       wpl_test_color(0.0f, 1.0f, 0.0f, 1.0f))
         == WPL_RESULT_OK);
  assert(wpl_draw_list_count(list) == 1u);

  wpl_destroy_draw_list(list);
  wpl_test_clear_manual_window(&window);
}

static void
test_submit_resets_backend_clip_state_each_call(void)
{
  WplWindow window;
  XImage ximage;
  uint32_t framebuffer[16];
  WplDrawList* list = wpl_test_create_list(4u);
  size_t i;

  for (i = 0u; i < 16u; i++)
    framebuffer[i] = 0u;

  wpl_test_prepare_manual_window(&window, 4, 4, framebuffer, &ximage);

  assert(wpl_draw_push_clip(list, wpl_test_rect(0.0f, 0.0f, 2.0f, 2.0f))
         == WPL_RESULT_OK);
  assert(wpl_submit_draw_list(&window, list) == WPL_RESULT_OK);
  assert(window.clip_stack_depth == 1u);
  assert(window.active_clip.x0 == 0);
  assert(window.active_clip.y0 == 0);
  assert(window.active_clip.x1 == 2);
  assert(window.active_clip.y1 == 2);

  assert(wpl_draw_list_clear(list) == WPL_RESULT_OK);
  assert(wpl_submit_draw_list(&window, list) == WPL_RESULT_OK);
  assert(window.clip_stack_depth == 0u);
  assert(window.active_clip.x0 == 0);
  assert(window.active_clip.y0 == 0);
  assert(window.active_clip.x1 == 4);
  assert(window.active_clip.y1 == 4);

  wpl_destroy_draw_list(list);
  wpl_test_clear_manual_window(&window);
}

int
main(void)
{
  test_submit_rejects_null_arguments();
  test_zero_sized_window_submit_is_noop_and_keeps_list_owned();
  test_manual_render_submit_keeps_draw_list_reusable();
  test_submit_resets_backend_clip_state_each_call();
  return 0;
}
