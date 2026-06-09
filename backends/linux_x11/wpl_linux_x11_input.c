/* wpl_linux_x11_input.c - Linux/X11 input snapshot translation. */

#include "wpl_linux_x11_internal.h"

#include <X11/keysym.h>

#include <stddef.h>

static void
wpl_linux_x11_apply_modifier_state(WplInputState* input, unsigned int state)
{
  if (input == NULL)
    return;

  input->keyboard.shift_down = (state & ShiftMask) != 0u;
  input->keyboard.ctrl_down = (state & ControlMask) != 0u;
  input->keyboard.alt_down = (state & Mod1Mask) != 0u;
}

static bool
wpl_linux_x11_keysym_is_shift(KeySym keysym)
{
  return keysym == XK_Shift_L || keysym == XK_Shift_R;
}

static bool
wpl_linux_x11_keysym_is_ctrl(KeySym keysym)
{
  return keysym == XK_Control_L || keysym == XK_Control_R;
}

static bool
wpl_linux_x11_keysym_is_alt(KeySym keysym)
{
  return keysym == XK_Alt_L || keysym == XK_Alt_R;
}

static void
wpl_linux_x11_force_modifier_state(WplInputState* input,
                                   KeySym keysym,
                                   bool down)
{
  if (input == NULL)
    return;

  if (wpl_linux_x11_keysym_is_shift(keysym))
    input->keyboard.shift_down = down;
  else if (wpl_linux_x11_keysym_is_ctrl(keysym))
    input->keyboard.ctrl_down = down;
  else if (wpl_linux_x11_keysym_is_alt(keysym))
    input->keyboard.alt_down = down;
}

static WplMouseButton
wpl_linux_x11_mouse_button_from_xbutton(unsigned int button)
{
  switch (button)
    {
    case Button1:
      return WPL_MOUSE_BUTTON_LEFT;
    case Button2:
      return WPL_MOUSE_BUTTON_MIDDLE;
    case Button3:
      return WPL_MOUSE_BUTTON_RIGHT;
    default:
      return WPL_MOUSE_BUTTON_COUNT;
    }
}

static WplKey
wpl_linux_x11_key_from_keysym(KeySym keysym)
{
  switch (keysym)
    {
    case XK_Escape:
      return WPL_KEY_ESCAPE;
    case XK_Delete:
      return WPL_KEY_DELETE;
    case XK_BackSpace:
      return WPL_KEY_BACKSPACE;
    case XK_Return:
    case XK_KP_Enter:
      return WPL_KEY_ENTER;
    case XK_Tab:
      return WPL_KEY_TAB;
    case XK_space:
      return WPL_KEY_SPACE;
    case XK_Left:
      return WPL_KEY_LEFT;
    case XK_Right:
      return WPL_KEY_RIGHT;
    case XK_Up:
      return WPL_KEY_UP;
    case XK_Down:
      return WPL_KEY_DOWN;
    case XK_A:
    case XK_a:
      return WPL_KEY_A;
    case XK_B:
    case XK_b:
      return WPL_KEY_B;
    case XK_C:
    case XK_c:
      return WPL_KEY_C;
    case XK_D:
    case XK_d:
      return WPL_KEY_D;
    case XK_E:
    case XK_e:
      return WPL_KEY_E;
    case XK_F:
    case XK_f:
      return WPL_KEY_F;
    case XK_G:
    case XK_g:
      return WPL_KEY_G;
    case XK_H:
    case XK_h:
      return WPL_KEY_H;
    case XK_I:
    case XK_i:
      return WPL_KEY_I;
    case XK_J:
    case XK_j:
      return WPL_KEY_J;
    case XK_K:
    case XK_k:
      return WPL_KEY_K;
    case XK_L:
    case XK_l:
      return WPL_KEY_L;
    case XK_M:
    case XK_m:
      return WPL_KEY_M;
    case XK_N:
    case XK_n:
      return WPL_KEY_N;
    case XK_O:
    case XK_o:
      return WPL_KEY_O;
    case XK_P:
    case XK_p:
      return WPL_KEY_P;
    case XK_Q:
    case XK_q:
      return WPL_KEY_Q;
    case XK_R:
    case XK_r:
      return WPL_KEY_R;
    case XK_S:
    case XK_s:
      return WPL_KEY_S;
    case XK_T:
    case XK_t:
      return WPL_KEY_T;
    case XK_U:
    case XK_u:
      return WPL_KEY_U;
    case XK_V:
    case XK_v:
      return WPL_KEY_V;
    case XK_W:
    case XK_w:
      return WPL_KEY_W;
    case XK_X:
    case XK_x:
      return WPL_KEY_X;
    case XK_Y:
    case XK_y:
      return WPL_KEY_Y;
    case XK_Z:
    case XK_z:
      return WPL_KEY_Z;
    default:
      return WPL_KEY_UNKNOWN;
    }
}

static bool
wpl_linux_x11_is_synthetic_repeat_release(WplWindow* window,
                                          const XKeyEvent* event)
{
  XEvent next_event;

  if (window == NULL || event == NULL || window->display == NULL)
    return false;

  if (window->xkb_detectable_auto_repeat_enabled)
    return false;

  if (XEventsQueued(window->display, QueuedAfterReading) <= 0)
    return false;

  XPeekEvent(window->display, &next_event);
  if (next_event.type != KeyPress)
    return false;

  if (next_event.xkey.keycode != event->keycode
      || next_event.xkey.time != event->time)
    return false;

  XNextEvent(window->display, &next_event);
  return true;
}


void
wpl_linux_x11_input_reset_transients(WplInputState* input)
{
  int button_index;
  int key_index;

  if (input == NULL)
    return;

  input->mouse.delta.x = 0.0f;
  input->mouse.delta.y = 0.0f;
  input->mouse.wheel_delta = 0.0f;

  for (button_index = 0; button_index < WPL_MOUSE_BUTTON_COUNT; ++button_index)
    {
      input->mouse.button_pressed[button_index] = false;
      input->mouse.button_released[button_index] = false;
    }

  for (key_index = 0; key_index < WPL_KEY_COUNT; ++key_index)
    {
      input->keyboard.key_pressed[key_index] = false;
      input->keyboard.key_released[key_index] = false;
    }
}

void
wpl_linux_x11_input_clear_down_state(WplInputState* input)
{
  int button_index;
  int key_index;

  if (input == NULL)
    return;

  for (button_index = 0; button_index < WPL_MOUSE_BUTTON_COUNT; ++button_index)
    input->mouse.button_down[button_index] = false;

  for (key_index = 0; key_index < WPL_KEY_COUNT; ++key_index)
    input->keyboard.key_down[key_index] = false;

  input->keyboard.shift_down = false;
  input->keyboard.ctrl_down = false;
  input->keyboard.alt_down = false;
}

void
wpl_linux_x11_input_press_mouse_button(WplInputState* input,
                                       WplMouseButton button)
{
  if (input == NULL || button < 0 || button >= WPL_MOUSE_BUTTON_COUNT)
    return;

  if (!input->mouse.button_down[button])
    input->mouse.button_pressed[button] = true;

  input->mouse.button_down[button] = true;
}

void
wpl_linux_x11_input_release_mouse_button(WplInputState* input,
                                         WplMouseButton button)
{
  if (input == NULL || button < 0 || button >= WPL_MOUSE_BUTTON_COUNT)
    return;

  if (input->mouse.button_down[button])
    input->mouse.button_released[button] = true;

  input->mouse.button_down[button] = false;
}

void
wpl_linux_x11_input_press_key(WplInputState* input, WplKey key)
{
  if (input == NULL || key <= WPL_KEY_UNKNOWN || key >= WPL_KEY_COUNT)
    return;

  if (!input->keyboard.key_down[key])
    input->keyboard.key_pressed[key] = true;

  input->keyboard.key_down[key] = true;
}

void
wpl_linux_x11_input_release_key(WplInputState* input, WplKey key)
{
  if (input == NULL || key <= WPL_KEY_UNKNOWN || key >= WPL_KEY_COUNT)
    return;

  if (input->keyboard.key_down[key])
    input->keyboard.key_released[key] = true;

  input->keyboard.key_down[key] = false;
}

void
wpl_linux_x11_reset_transient_input(WplWindow* window)
{
  if (window == NULL)
    return;

  wpl_linux_x11_input_reset_transients(&window->input);
}

void
wpl_linux_x11_clear_input_down_state(WplWindow* window)
{
  if (window == NULL)
    return;

  wpl_linux_x11_input_clear_down_state(&window->input);
}

void
wpl_linux_x11_init_detectable_auto_repeat(WplWindow* window)
{
  Bool supported = False;
  Bool result;

  if (window == NULL || window->display == NULL)
    return;

  result = XkbSetDetectableAutoRepeat(window->display, True, &supported);
  window->xkb_detectable_auto_repeat_enabled = (result == True
                                                && supported == True);
}

void
wpl_linux_x11_handle_motion(WplWindow* window, const XMotionEvent* event)
{
  WplVec2 position;

  if (window == NULL || event == NULL)
    return;

  position.x = (float)event->x;
  position.y = (float)event->y;

  window->input.mouse.delta.x += position.x - window->input.mouse.position.x;
  window->input.mouse.delta.y += position.y - window->input.mouse.position.y;
  window->input.mouse.position = position;

  wpl_linux_x11_apply_modifier_state(&window->input, event->state);
}

void
wpl_linux_x11_handle_button_press(WplWindow* window,
                                  const XButtonEvent* event)
{
  WplMouseButton button;

  if (window == NULL || event == NULL)
    return;

  wpl_linux_x11_apply_modifier_state(&window->input, event->state);

  if (event->button == Button4)
    {
      window->input.mouse.wheel_delta += 1.0f;
      return;
    }

  if (event->button == Button5)
    {
      window->input.mouse.wheel_delta -= 1.0f;
      return;
    }

  if (event->button == 6u || event->button == 7u)
    return;

  button = wpl_linux_x11_mouse_button_from_xbutton(event->button);
  if (button == WPL_MOUSE_BUTTON_COUNT)
    return;

  wpl_linux_x11_input_press_mouse_button(&window->input, button);
}

void
wpl_linux_x11_handle_button_release(WplWindow* window,
                                    const XButtonEvent* event)
{
  WplMouseButton button;

  if (window == NULL || event == NULL)
    return;

  wpl_linux_x11_apply_modifier_state(&window->input, event->state);

  if (event->button == Button4 || event->button == Button5
      || event->button == 6u || event->button == 7u)
    return;

  button = wpl_linux_x11_mouse_button_from_xbutton(event->button);
  if (button == WPL_MOUSE_BUTTON_COUNT)
    return;

  wpl_linux_x11_input_release_mouse_button(&window->input, button);
}

void
wpl_linux_x11_handle_key_press(WplWindow* window, XKeyEvent* event)
{
  KeySym keysym;
  WplKey key;

  if (window == NULL || event == NULL)
    return;

  keysym = XLookupKeysym(event, 0);
  wpl_linux_x11_apply_modifier_state(&window->input, event->state);
  wpl_linux_x11_force_modifier_state(&window->input, keysym, true);

  key = wpl_linux_x11_key_from_keysym(keysym);
  if (key == WPL_KEY_UNKNOWN)
    return;

  wpl_linux_x11_input_press_key(&window->input, key);
}

void
wpl_linux_x11_handle_key_release(WplWindow* window, XKeyEvent* event)
{
  KeySym keysym;
  WplKey key;

  if (window == NULL || event == NULL)
    return;

  if (wpl_linux_x11_is_synthetic_repeat_release(window, event))
    return;

  keysym = XLookupKeysym(event, 0);
  wpl_linux_x11_apply_modifier_state(&window->input, event->state);
  wpl_linux_x11_force_modifier_state(&window->input, keysym, false);

  key = wpl_linux_x11_key_from_keysym(keysym);
  if (key == WPL_KEY_UNKNOWN)
    return;

  wpl_linux_x11_input_release_key(&window->input, key);
}

WplInputState
wpl_get_input(const WplWindow* window)
{
  WplInputState input = {0};

  if (window == NULL)
    return input;

  return window->input;
}
