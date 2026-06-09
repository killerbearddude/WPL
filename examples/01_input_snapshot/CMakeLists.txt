add_executable(wpl_input_snapshot
  main.c
)

target_link_libraries(wpl_input_snapshot PRIVATE wpl)

target_compile_options(wpl_input_snapshot PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
