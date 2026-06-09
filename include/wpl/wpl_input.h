#ifndef WPL_INPUT_H
#define WPL_INPUT_H

#include <stdbool.h>

#include "wpl_base.h"
#include "wpl_window.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum WplMouseButton {
  WPL_MOUSE_BUTTON_LEFT = 0,
  WPL_MOUSE_BUTTON_RIGHT = 1,
  WPL_MOUSE_BUTTON_MIDDLE = 2,
  WPL_MOUSE_BUTTON_COUNT = 3
} WplMouseButton;

typedef enum WplKey {
  WPL_KEY_UNKNOWN = 0,
  WPL_KEY_ESCAPE,
  WPL_KEY_DELETE,
  WPL_KEY_BACKSPACE,
  WPL_KEY_ENTER,
  WPL_KEY_TAB,
  WPL_KEY_SPACE,
  WPL_KEY_LEFT,
  WPL_KEY_RIGHT,
  WPL_KEY_UP,
  WPL_KEY_DOWN,
  WPL_KEY_A,
  WPL_KEY_B,
  WPL_KEY_C,
  WPL_KEY_D,
  WPL_KEY_E,
  WPL_KEY_F,
  WPL_KEY_G,
  WPL_KEY_H,
  WPL_KEY_I,
  WPL_KEY_J,
  WPL_KEY_K,
  WPL_KEY_L,
  WPL_KEY_M,
  WPL_KEY_N,
  WPL_KEY_O,
  WPL_KEY_P,
  WPL_KEY_Q,
  WPL_KEY_R,
  WPL_KEY_S,
  WPL_KEY_T,
  WPL_KEY_U,
  WPL_KEY_V,
  WPL_KEY_W,
  WPL_KEY_X,
  WPL_KEY_Y,
  WPL_KEY_Z,
  WPL_KEY_COUNT
} WplKey;

typedef struct WplMouseState {
  WplVec2 position;
  WplVec2 delta;
  bool button_down[WPL_MOUSE_BUTTON_COUNT];
  bool button_pressed[WPL_MOUSE_BUTTON_COUNT];
  bool button_released[WPL_MOUSE_BUTTON_COUNT];
  float wheel_delta;
} WplMouseState;

typedef struct WplKeyboardState {
  bool key_down[WPL_KEY_COUNT];
  bool key_pressed[WPL_KEY_COUNT];
  bool key_released[WPL_KEY_COUNT];
  bool shift_down;
  bool ctrl_down;
  bool alt_down;
} WplKeyboardState;

/* Frame-stable input snapshot returned by value. */
typedef struct WplInputState {
  WplMouseState mouse;
  WplKeyboardState keyboard;
} WplInputState;

WplInputState wpl_get_input(const WplWindow* window);

#ifdef __cplusplus
}
#endif

#endif /* WPL_INPUT_H */
