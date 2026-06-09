#ifndef WPL_BASE_H
#define WPL_BASE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WPL_MAX_WINDOW_DIMENSION 16384
#define WPL_MAX_FILE_SIZE_V0_1 (64u * 1024u * 1024u)

/* Floating-point 2D vector used for positions, sizes, and deltas. */
typedef struct WplVec2 {
  float x;
  float y;
} WplVec2;

/* Integer 2D vector used for pixel-sized quantities where needed. */
typedef struct WplVec2i {
  int x;
  int y;
} WplVec2i;

/* Rectangle with top-left origin and positive width/height by convention. */
typedef struct WplRect {
  float x;
  float y;
  float w;
  float h;
} WplRect;

/* Straight-alpha color. Channels are expected in the range [0, 1]. */
typedef struct WplColor {
  float r;
  float g;
  float b;
  float a;
} WplColor;

#ifdef __cplusplus
}
#endif

#endif /* WPL_BASE_H */
