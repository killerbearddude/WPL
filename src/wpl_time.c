/* wpl_time.c - monotonic process time entry point. */

#define _POSIX_C_SOURCE 200809L

#include "wpl/wpl_time.h"

#include <time.h>

/* Return monotonic seconds. Failure is unexpected on supported Linux hosts. */
double
wpl_time_seconds(void)
{
  struct timespec now;

  if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
    return 0.0;

  return (double)now.tv_sec + ((double)now.tv_nsec / 1000000000.0);
}
