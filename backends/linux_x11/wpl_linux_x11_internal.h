#ifndef WPL_LINUX_X11_INTERNAL_H
#define WPL_LINUX_X11_INTERNAL_H

/*
 * Linux/X11 backend-private boundary.
 *
 * Xlib and XKB headers may be included from this private header in later
 * patches. They must never be included by public headers under include/wpl/.
 * The concrete WplWindow definition also belongs behind this boundary.
 */

#include "wpl/wpl_window.h"

#endif /* WPL_LINUX_X11_INTERNAL_H */
