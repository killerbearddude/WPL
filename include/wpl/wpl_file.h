#ifndef WPL_FILE_H
#define WPL_FILE_H

#include <stddef.h>

#include "wpl_result.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WplFileData {
  void* data;
  size_t size;
} WplFileData;

WplResult wpl_read_entire_file(const char* path, WplFileData* out_data);
WplResult wpl_write_entire_file(const char* path,
                                const void* data,
                                size_t size);
void wpl_free_file_data(WplFileData* data);

#ifdef __cplusplus
}
#endif

#endif /* WPL_FILE_H */
