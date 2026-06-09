/* wpl_linux_x11_input.c - Linux/X11 input snapshot API stubs. */

#include "wpl/wpl_input.h"

WplInputState
wpl_get_input(const WplWindow* window)
{
  WplInputState input = {0};

  (void)window;
  return input;
}
