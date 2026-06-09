#ifndef WPL_TIME_H
#define WPL_TIME_H

#include "wpl_window.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Returns monotonic seconds once the time module is implemented. */
double wpl_time_seconds(void);

/* Returns the most recently computed frame delta for WINDOW. */
float wpl_window_delta_time(const WplWindow* window);

#ifdef __cplusplus
}
#endif

#endif /* WPL_TIME_H */
