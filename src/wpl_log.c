/* wpl_log.c - Minimal process-global logging callback. */

#include "wpl/wpl_log.h"

static WplLogCallback wpl_log_callback;
static void* wpl_log_user_data;

void
wpl_set_log_callback(WplLogCallback callback, void* user_data)
{
  wpl_log_callback = callback;
  wpl_log_user_data = user_data;
}

void
wpl_log(WplLogLevel level, const char* message)
{
  if (wpl_log_callback == 0)
    return;

  wpl_log_callback(level, message != 0 ? message : "", wpl_log_user_data);
}
