#ifndef WPL_LOG_H
#define WPL_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum WplLogLevel {
  WPL_LOG_TRACE = 0,
  WPL_LOG_INFO,
  WPL_LOG_WARNING,
  WPL_LOG_ERROR,
  WPL_LOG_FATAL
} WplLogLevel;

typedef void (*WplLogCallback)(WplLogLevel level,
                               const char* message,
                               void* user_data);

/* Install a process-global log callback. Passing NULL disables logging. */
void wpl_set_log_callback(WplLogCallback callback, void* user_data);

/* Emit a plain log message. NULL messages are delivered as an empty string. */
void wpl_log(WplLogLevel level, const char* message);

#ifdef __cplusplus
}
#endif

#endif /* WPL_LOG_H */
