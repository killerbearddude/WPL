# File I/O

WPL v0.1 provides backend-independent whole-file binary read/write helpers.

## Scope

- Reads are binary.
- Writes are binary.
- The caller owns read buffers and releases them with `wpl_free_file_data`.
- Files larger than `WPL_MAX_FILE_SIZE_V0_1` return `WPL_RESULT_UNSUPPORTED`.
- Writes create or truncate files using mode `0644`, subject to the process umask.
- Symlinks are followed normally.
- `open`, `read`, and `write` retry on `EINTR`.
- Short reads and short writes are handled with explicit loops.
- Atomic writes are not guaranteed.

## Non-goals

File I/O is not an asset pipeline, serialization layer, config system, virtual
file system, directory traversal API, file watcher, async I/O system, or replay
implementation.
