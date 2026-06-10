#ifndef WPL_TEXT_INTERNAL_H
#define WPL_TEXT_INTERNAL_H

/* Canonical v0.1 ASCII bitmap text metrics shared by measurement and
   rendering.  Glyph bitmap storage remains backend-private. */

#define WPL_TEXT_GLYPH_WIDTH_PIXELS 5
#define WPL_TEXT_GLYPH_HEIGHT_PIXELS 7
#define WPL_TEXT_GLYPH_ADVANCE_X_PIXELS 6
#define WPL_TEXT_LINE_HEIGHT_PIXELS 9

#define WPL_TEXT_GLYPH_WIDTH ((float)WPL_TEXT_GLYPH_WIDTH_PIXELS)
#define WPL_TEXT_GLYPH_HEIGHT ((float)WPL_TEXT_GLYPH_HEIGHT_PIXELS)
#define WPL_TEXT_GLYPH_ADVANCE_X ((float)WPL_TEXT_GLYPH_ADVANCE_X_PIXELS)
#define WPL_TEXT_LINE_HEIGHT ((float)WPL_TEXT_LINE_HEIGHT_PIXELS)

#endif /* WPL_TEXT_INTERNAL_H */
