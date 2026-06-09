#ifndef WPL_RESULT_H
#define WPL_RESULT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Result codes returned by fallible public WPL operations. */
typedef enum WplResult {
  WPL_RESULT_OK = 0,
  WPL_RESULT_ERROR = 1,
  WPL_RESULT_INVALID_ARGUMENT = 2,
  WPL_RESULT_OUT_OF_MEMORY = 3,
  WPL_RESULT_PLATFORM_ERROR = 4,
  WPL_RESULT_UNSUPPORTED = 5,
  WPL_RESULT_CAPACITY_EXCEEDED = 6,
  WPL_RESULT_IO_ERROR = 7,
  WPL_RESULT_PARSE_ERROR = 8,
  WPL_RESULT_TRUNCATED = 9
} WplResult;

#ifdef __cplusplus
}
#endif

#endif /* WPL_RESULT_H */
