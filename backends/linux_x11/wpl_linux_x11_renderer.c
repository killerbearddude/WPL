/* wpl_linux_x11_renderer.c - Minimal software renderer and XImage presenter. */

#include "wpl_linux_x11_internal.h"

#include "wpl_draw_internal.h"
#include "wpl_linux_x11_font.h"

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

static int
wpl_linux_x11_floor_to_int(float value)
{
  int truncated = (int)value;

  if ((float)truncated > value)
    truncated--;

  return truncated;
}

static int
wpl_linux_x11_ceil_to_int(float value)
{
  int truncated = (int)value;

  if ((float)truncated < value)
    truncated++;

  return truncated;
}

static int
wpl_linux_x11_clamp_int(int value, int min_value, int max_value)
{
  if (value < min_value)
    return min_value;

  if (value > max_value)
    return max_value;

  return value;
}

static uint8_t
wpl_linux_x11_float_to_u8(float value)
{
  if (value <= 0.0f)
    return 0u;

  if (value >= 1.0f)
    return 255u;

  return (uint8_t)(value * 255.0f + 0.5f);
}

static uint32_t
wpl_linux_x11_pack_opaque_pixel(uint8_t r, uint8_t g, uint8_t b)
{
  return (0xff000000u | ((uint32_t)r << 16u) | ((uint32_t)g << 8u)
          | (uint32_t)b);
}

static uint32_t
wpl_linux_x11_color_to_pixel_opaque(WplColor color)
{
  return wpl_linux_x11_pack_opaque_pixel(
      wpl_linux_x11_float_to_u8(color.r),
      wpl_linux_x11_float_to_u8(color.g),
      wpl_linux_x11_float_to_u8(color.b));
}

static uint32_t
wpl_linux_x11_blend_pixel_opaque(uint32_t dst, WplColor src)
{
  float src_a = src.a;
  uint8_t src_r;
  uint8_t src_g;
  uint8_t src_b;
  uint8_t dst_r;
  uint8_t dst_g;
  uint8_t dst_b;
  uint8_t out_r;
  uint8_t out_g;
  uint8_t out_b;

  if (src_a <= 0.0f)
    return dst | 0xff000000u;

  if (src_a >= 1.0f)
    return wpl_linux_x11_color_to_pixel_opaque(src);

  src_r = wpl_linux_x11_float_to_u8(src.r);
  src_g = wpl_linux_x11_float_to_u8(src.g);
  src_b = wpl_linux_x11_float_to_u8(src.b);

  dst_r = (uint8_t)((dst >> 16u) & 0xffu);
  dst_g = (uint8_t)((dst >> 8u) & 0xffu);
  dst_b = (uint8_t)(dst & 0xffu);

  out_r = (uint8_t)((float)src_r * src_a
                    + (float)dst_r * (1.0f - src_a) + 0.5f);
  out_g = (uint8_t)((float)src_g * src_a
                    + (float)dst_g * (1.0f - src_a) + 0.5f);
  out_b = (uint8_t)((float)src_b * src_a
                    + (float)dst_b * (1.0f - src_a) + 0.5f);

  return wpl_linux_x11_pack_opaque_pixel(out_r, out_g, out_b);
}

static float
wpl_linux_x11_min_float(float a, float b)
{
  return a < b ? a : b;
}

static float
wpl_linux_x11_max_float(float a, float b)
{
  return a > b ? a : b;
}

static void
wpl_linux_x11_blend_pixel_at(WplWindow* window,
                             int x,
                             int y,
                             WplColor color)
{
  uint32_t* pixel;

  if (window == NULL || window->framebuffer == NULL)
    return;

  if (x < 0 || y < 0 || x >= window->framebuffer_width
      || y >= window->framebuffer_height)
    return;

  pixel = &window->framebuffer[((size_t)y
                                * (size_t)window->framebuffer_width)
                               + (size_t)x];
  *pixel = wpl_linux_x11_blend_pixel_opaque(*pixel, color);
}

static float
wpl_linux_x11_distance_sq_point_segment(float px,
                                        float py,
                                        WplVec2 a,
                                        WplVec2 b)
{
  float ab_x = b.x - a.x;
  float ab_y = b.y - a.y;
  float ap_x = px - a.x;
  float ap_y = py - a.y;
  float ab_len_sq = ab_x * ab_x + ab_y * ab_y;
  float t;
  float closest_x;
  float closest_y;
  float dx;
  float dy;

  if (ab_len_sq <= 0.0f)
    {
      dx = px - a.x;
      dy = py - a.y;
      return dx * dx + dy * dy;
    }

  t = (ap_x * ab_x + ap_y * ab_y) / ab_len_sq;
  if (t < 0.0f)
    t = 0.0f;
  else if (t > 1.0f)
    t = 1.0f;

  closest_x = a.x + ab_x * t;
  closest_y = a.y + ab_y * t;
  dx = px - closest_x;
  dy = py - closest_y;
  return dx * dx + dy * dy;
}

static void
wpl_linux_x11_destroy_framebuffer(WplWindow* window)
{
  if (window == NULL)
    return;

  free(window->framebuffer);
  window->framebuffer = NULL;
  window->framebuffer_width = 0;
  window->framebuffer_height = 0;
}

static WplResult
wpl_linux_x11_resize_framebuffer(WplWindow* window, int width, int height)
{
  uint32_t* pixels;
  size_t pixel_count;

  if (window == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (width <= 0 || height <= 0)
    {
      wpl_linux_x11_destroy_framebuffer(window);
      return WPL_RESULT_OK;
    }

  if (window->framebuffer != NULL && window->framebuffer_width == width
      && window->framebuffer_height == height)
    return WPL_RESULT_OK;

  pixel_count = (size_t)width * (size_t)height;
  if (pixel_count > (SIZE_MAX / sizeof(uint32_t)))
    return WPL_RESULT_OUT_OF_MEMORY;

  pixels = (uint32_t*)calloc(pixel_count, sizeof(pixels[0]));
  if (pixels == NULL)
    return WPL_RESULT_OUT_OF_MEMORY;

  wpl_linux_x11_destroy_framebuffer(window);
  window->framebuffer = pixels;
  window->framebuffer_width = width;
  window->framebuffer_height = height;
  return WPL_RESULT_OK;
}

static void
wpl_linux_x11_destroy_ximage(WplWindow* window)
{
  if (window == NULL)
    return;

  if (window->ximage != NULL)
    {
      /* XImage owns ximage->data after successful XCreateImage. */
      XDestroyImage(window->ximage);
      window->ximage = NULL;
      window->ximage_pixels = NULL;
    }
}

static WplResult
wpl_linux_x11_resize_ximage(WplWindow* window, int width, int height)
{
  uint32_t* pixels;
  XImage* ximage;
  int screen;
  int depth;
  size_t pixel_count;

  if (window == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (width <= 0 || height <= 0)
    {
      wpl_linux_x11_destroy_ximage(window);
      return WPL_RESULT_OK;
    }

  if (window->ximage != NULL && window->ximage->width == width
      && window->ximage->height == height)
    return WPL_RESULT_OK;

  if (window->display == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  screen = DefaultScreen(window->display);
  depth = DefaultDepth(window->display, screen);
  if (depth != 24 && depth != 32)
    return WPL_RESULT_UNSUPPORTED;

  pixel_count = (size_t)width * (size_t)height;
  if (pixel_count > (SIZE_MAX / sizeof(uint32_t)))
    return WPL_RESULT_OUT_OF_MEMORY;

  pixels = (uint32_t*)calloc(pixel_count, sizeof(pixels[0]));
  if (pixels == NULL)
    return WPL_RESULT_OUT_OF_MEMORY;

  ximage = XCreateImage(window->display,
                        DefaultVisual(window->display, screen),
                        (unsigned int)depth,
                        ZPixmap,
                        0,
                        (char*)pixels,
                        (unsigned int)width,
                        (unsigned int)height,
                        32,
                        0);
  if (ximage == NULL)
    {
      free(pixels);
      return WPL_RESULT_PLATFORM_ERROR;
    }

  if (ximage->bits_per_pixel != 32)
    {
      XDestroyImage(ximage);
      return WPL_RESULT_UNSUPPORTED;
    }

  wpl_linux_x11_destroy_ximage(window);
  window->ximage = ximage;
  window->ximage_pixels = pixels;
  return WPL_RESULT_OK;
}

static WplResult
wpl_linux_x11_ensure_render_targets(WplWindow* window)
{
  WplResult result;

  if (window == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (window->width <= 0 || window->height <= 0)
    return WPL_RESULT_OK;

  result = wpl_linux_x11_resize_framebuffer(window,
                                            window->width,
                                            window->height);
  if (result != WPL_RESULT_OK)
    return result;

  return wpl_linux_x11_resize_ximage(window, window->width, window->height);
}

static void
wpl_linux_x11_render_clear(WplWindow* window, WplColor color)
{
  uint32_t pixel;
  size_t pixel_count;
  size_t i;

  if (window == NULL || window->framebuffer == NULL)
    return;

  pixel = wpl_linux_x11_color_to_pixel_opaque(color);
  pixel_count = ((size_t)window->framebuffer_width
                 * (size_t)window->framebuffer_height);

  for (i = 0u; i < pixel_count; i++)
    window->framebuffer[i] = pixel;
}

static void
wpl_linux_x11_render_rect(WplWindow* window, WplRect rect, WplColor color)
{
  int x0;
  int y0;
  int x1;
  int y1;
  int x;
  int y;

  if (window == NULL || window->framebuffer == NULL)
    return;

  if (rect.w <= 0.0f || rect.h <= 0.0f)
    return;

  x0 = wpl_linux_x11_floor_to_int(rect.x);
  y0 = wpl_linux_x11_floor_to_int(rect.y);
  x1 = wpl_linux_x11_ceil_to_int(rect.x + rect.w);
  y1 = wpl_linux_x11_ceil_to_int(rect.y + rect.h);

  x0 = wpl_linux_x11_clamp_int(x0, 0, window->framebuffer_width);
  y0 = wpl_linux_x11_clamp_int(y0, 0, window->framebuffer_height);
  x1 = wpl_linux_x11_clamp_int(x1, 0, window->framebuffer_width);
  y1 = wpl_linux_x11_clamp_int(y1, 0, window->framebuffer_height);

  if (x0 >= x1 || y0 >= y1)
    return;

  for (y = y0; y < y1; y++)
    {
      for (x = x0; x < x1; x++)
        wpl_linux_x11_blend_pixel_at(window, x, y, color);
    }
}

static void
wpl_linux_x11_render_rect_outline(WplWindow* window,
                                  WplRect rect,
                                  WplColor color,
                                  float thickness)
{
  int x0;
  int y0;
  int x1;
  int y1;
  int ix0;
  int iy0;
  int ix1;
  int iy1;
  int clamped_x0;
  int clamped_y0;
  int clamped_x1;
  int clamped_y1;
  int x;
  int y;

  if (window == NULL || window->framebuffer == NULL)
    return;

  if (thickness <= 0.0f || rect.w <= 0.0f || rect.h <= 0.0f)
    return;

  x0 = wpl_linux_x11_floor_to_int(rect.x);
  y0 = wpl_linux_x11_floor_to_int(rect.y);
  x1 = wpl_linux_x11_ceil_to_int(rect.x + rect.w);
  y1 = wpl_linux_x11_ceil_to_int(rect.y + rect.h);

  ix0 = wpl_linux_x11_floor_to_int(rect.x + thickness);
  iy0 = wpl_linux_x11_floor_to_int(rect.y + thickness);
  ix1 = wpl_linux_x11_ceil_to_int(rect.x + rect.w - thickness);
  iy1 = wpl_linux_x11_ceil_to_int(rect.y + rect.h - thickness);

  clamped_x0 = wpl_linux_x11_clamp_int(x0, 0, window->framebuffer_width);
  clamped_y0 = wpl_linux_x11_clamp_int(y0, 0, window->framebuffer_height);
  clamped_x1 = wpl_linux_x11_clamp_int(x1, 0, window->framebuffer_width);
  clamped_y1 = wpl_linux_x11_clamp_int(y1, 0, window->framebuffer_height);

  if (clamped_x0 >= clamped_x1 || clamped_y0 >= clamped_y1)
    return;

  for (y = clamped_y0; y < clamped_y1; y++)
    {
      for (x = clamped_x0; x < clamped_x1; x++)
        {
          bool inside_inner = (ix0 < ix1 && iy0 < iy1 && x >= ix0
                               && x < ix1 && y >= iy0 && y < iy1);

          if (!inside_inner)
            wpl_linux_x11_blend_pixel_at(window, x, y, color);
        }
    }
}

static void
wpl_linux_x11_render_line(WplWindow* window,
                          WplVec2 a,
                          WplVec2 b,
                          WplColor color,
                          float thickness)
{
  float radius;
  float radius_sq;
  int x0;
  int y0;
  int x1;
  int y1;
  int x;
  int y;

  if (window == NULL || window->framebuffer == NULL)
    return;

  if (thickness <= 0.0f)
    return;

  radius = thickness * 0.5f;
  radius_sq = radius * radius;

  x0 = wpl_linux_x11_floor_to_int(wpl_linux_x11_min_float(a.x, b.x)
                                  - radius);
  y0 = wpl_linux_x11_floor_to_int(wpl_linux_x11_min_float(a.y, b.y)
                                  - radius);
  x1 = wpl_linux_x11_ceil_to_int(wpl_linux_x11_max_float(a.x, b.x)
                                 + radius);
  y1 = wpl_linux_x11_ceil_to_int(wpl_linux_x11_max_float(a.y, b.y)
                                 + radius);

  x0 = wpl_linux_x11_clamp_int(x0, 0, window->framebuffer_width);
  y0 = wpl_linux_x11_clamp_int(y0, 0, window->framebuffer_height);
  x1 = wpl_linux_x11_clamp_int(x1, 0, window->framebuffer_width);
  y1 = wpl_linux_x11_clamp_int(y1, 0, window->framebuffer_height);

  if (x0 >= x1 || y0 >= y1)
    return;

  for (y = y0; y < y1; y++)
    {
      for (x = x0; x < x1; x++)
        {
          float px = (float)x + 0.5f;
          float py = (float)y + 0.5f;
          float distance_sq = wpl_linux_x11_distance_sq_point_segment(px,
                                                                      py,
                                                                      a,
                                                                      b);
          if (distance_sq <= radius_sq)
            wpl_linux_x11_blend_pixel_at(window, x, y, color);
        }
    }
}

static void
wpl_linux_x11_render_circle(WplWindow* window,
                            WplVec2 center,
                            float radius,
                            WplColor color)
{
  float radius_sq;
  int x0;
  int y0;
  int x1;
  int y1;
  int x;
  int y;

  if (window == NULL || window->framebuffer == NULL)
    return;

  if (radius <= 0.0f)
    return;

  radius_sq = radius * radius;
  x0 = wpl_linux_x11_floor_to_int(center.x - radius);
  y0 = wpl_linux_x11_floor_to_int(center.y - radius);
  x1 = wpl_linux_x11_ceil_to_int(center.x + radius);
  y1 = wpl_linux_x11_ceil_to_int(center.y + radius);

  x0 = wpl_linux_x11_clamp_int(x0, 0, window->framebuffer_width);
  y0 = wpl_linux_x11_clamp_int(y0, 0, window->framebuffer_height);
  x1 = wpl_linux_x11_clamp_int(x1, 0, window->framebuffer_width);
  y1 = wpl_linux_x11_clamp_int(y1, 0, window->framebuffer_height);

  if (x0 >= x1 || y0 >= y1)
    return;

  for (y = y0; y < y1; y++)
    {
      for (x = x0; x < x1; x++)
        {
          float dx = ((float)x + 0.5f) - center.x;
          float dy = ((float)y + 0.5f) - center.y;
          if ((dx * dx + dy * dy) <= radius_sq)
            wpl_linux_x11_blend_pixel_at(window, x, y, color);
        }
    }
}

static void
wpl_linux_x11_render_glyph(WplWindow* window,
                          int x,
                          int y,
                          unsigned char c,
                          WplColor color)
{
  const uint8_t* glyph;
  int row;
  int col;

  if (window == NULL || window->framebuffer == NULL)
    return;

  glyph = wpl_linux_x11_font5x7_glyph(c);

  for (row = 0; row < WPL_LINUX_X11_FONT_GLYPH_HEIGHT; row++)
    {
      uint8_t bits = glyph[row];

      for (col = 0; col < WPL_LINUX_X11_FONT_GLYPH_WIDTH; col++)
        {
          uint8_t mask = (uint8_t)(1u << (WPL_LINUX_X11_FONT_GLYPH_WIDTH
                                          - 1 - col));
          if ((bits & mask) != 0u)
            wpl_linux_x11_blend_pixel_at(window, x + col, y + row, color);
        }
    }
}

static void
wpl_linux_x11_render_text(WplWindow* window,
                          WplVec2 position,
                          const char* text,
                          WplColor color)
{
  int base_x;
  int x;
  int y;
  const char* cursor;

  if (window == NULL || window->framebuffer == NULL || text == NULL)
    return;

  base_x = wpl_linux_x11_floor_to_int(position.x);
  x = base_x;
  y = wpl_linux_x11_floor_to_int(position.y);

  for (cursor = text; *cursor != '\0'; cursor++)
    {
      unsigned char c = (unsigned char)*cursor;

      if (c == (unsigned char)'\n')
        {
          x = base_x;
          y += WPL_LINUX_X11_FONT_LINE_HEIGHT;
          continue;
        }

      if (c == (unsigned char)'\r')
        continue;

      if (c == (unsigned char)'\t')
        {
          x += WPL_LINUX_X11_FONT_ADVANCE_X * 4;
          continue;
        }

      wpl_linux_x11_render_glyph(window, x, y, c, color);
      x += WPL_LINUX_X11_FONT_ADVANCE_X;
    }
}

static WplResult
wpl_linux_x11_copy_framebuffer_to_ximage(WplWindow* window)
{
  size_t pixel_count;
  size_t i;

  if (window == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (window->framebuffer == NULL || window->ximage_pixels == NULL)
    return WPL_RESULT_OK;

  if (window->framebuffer_width != window->width
      || window->framebuffer_height != window->height)
    return WPL_RESULT_PLATFORM_ERROR;

  pixel_count = ((size_t)window->framebuffer_width
                 * (size_t)window->framebuffer_height);
  for (i = 0u; i < pixel_count; i++)
    window->ximage_pixels[i] = window->framebuffer[i];

  return WPL_RESULT_OK;
}

static WplResult
wpl_linux_x11_present(WplWindow* window)
{
  WplResult result;

  if (window == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (window->width <= 0 || window->height <= 0)
    return WPL_RESULT_OK;

  result = wpl_linux_x11_ensure_render_targets(window);
  if (result != WPL_RESULT_OK)
    return result;

  if (window->framebuffer == NULL || window->ximage == NULL)
    return WPL_RESULT_OK;

  result = wpl_linux_x11_copy_framebuffer_to_ximage(window);
  if (result != WPL_RESULT_OK)
    return result;

  /* XPutImage queues a request to the X server.  The default X11 error
     handler is still responsible for asynchronous protocol errors in v0.1. */
  XPutImage(window->display,
            window->window,
            window->gc,
            window->ximage,
            0,
            0,
            0,
            0,
            (unsigned int)window->width,
            (unsigned int)window->height);

  XFlush(window->display);
  return WPL_RESULT_OK;
}

void
wpl_linux_x11_destroy_renderer_resources(WplWindow* window)
{
  wpl_linux_x11_destroy_ximage(window);
  wpl_linux_x11_destroy_framebuffer(window);
}

WplResult
wpl_linux_x11_present_frame(WplWindow* window)
{
  return wpl_linux_x11_present(window);
}

WplResult
wpl_submit_draw_list(WplWindow* window, const WplDrawList* list)
{
  size_t i;
  WplResult result;

  if (window == NULL || list == NULL)
    return WPL_RESULT_INVALID_ARGUMENT;

  if (window->width <= 0 || window->height <= 0)
    return WPL_RESULT_OK;

  result = wpl_linux_x11_ensure_render_targets(window);
  if (result != WPL_RESULT_OK)
    return result;

  for (i = 0u; i < list->count; i++)
    {
      const WplDrawCommand* command = &list->commands[i];

      switch (command->type)
        {
        case WPL_DRAW_COMMAND_CLEAR:
          wpl_linux_x11_render_clear(window, command->color);
          break;

        case WPL_DRAW_COMMAND_RECT:
          wpl_linux_x11_render_rect(window, command->rect, command->color);
          break;

        case WPL_DRAW_COMMAND_RECT_OUTLINE:
          wpl_linux_x11_render_rect_outline(window,
                                            command->rect,
                                            command->color,
                                            command->thickness);
          break;

        case WPL_DRAW_COMMAND_LINE:
          wpl_linux_x11_render_line(window,
                                    command->a,
                                    command->b,
                                    command->color,
                                    command->thickness);
          break;

        case WPL_DRAW_COMMAND_CIRCLE:
          wpl_linux_x11_render_circle(window,
                                      command->a,
                                      command->radius,
                                      command->color);
          break;

        case WPL_DRAW_COMMAND_TEXT:
          wpl_linux_x11_render_text(window,
                                    command->a,
                                    command->text,
                                    command->color);
          break;

        default:
          return WPL_RESULT_UNSUPPORTED;
        }
    }

  return WPL_RESULT_OK;
}
