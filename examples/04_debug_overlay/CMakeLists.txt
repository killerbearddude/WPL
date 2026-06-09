add_executable(wpl_debug_overlay
  main.c
)

target_link_libraries(wpl_debug_overlay PRIVATE wpl)

target_compile_options(wpl_debug_overlay PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
