add_executable(wpl_empty_window
  main.c
)

target_link_libraries(wpl_empty_window
  PRIVATE
    wpl
)

target_compile_options(wpl_empty_window PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
