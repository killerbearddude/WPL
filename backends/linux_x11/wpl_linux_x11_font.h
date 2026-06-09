/* wpl_linux_x11_font.h - Backend-private embedded bitmap font. */

#ifndef WPL_LINUX_X11_FONT_H
#define WPL_LINUX_X11_FONT_H

#include <stdint.h>

#define WPL_LINUX_X11_FONT_FIRST_CHAR 32
#define WPL_LINUX_X11_FONT_LAST_CHAR 126
#define WPL_LINUX_X11_FONT_GLYPH_WIDTH 5
#define WPL_LINUX_X11_FONT_GLYPH_HEIGHT 7
#define WPL_LINUX_X11_FONT_ADVANCE_X 6
#define WPL_LINUX_X11_FONT_LINE_HEIGHT 9

const uint8_t* wpl_linux_x11_font5x7_glyph(unsigned char c);

#endif /* WPL_LINUX_X11_FONT_H */
