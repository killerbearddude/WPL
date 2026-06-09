#ifndef WPL_TIME_H
#define WPL_TIME_H

#include "wpl_window.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Return monotonic process time in seconds, or 0.0 on unexpected failure. */
double wpl_time_seconds(void);

/* Return the most recently computed frame delta for WINDOW. */
float wpl_window_delta_time(const WplWindow* window);

#ifdef __cplusplus
}
#endif

#endif /* WPL_TIME_H */
