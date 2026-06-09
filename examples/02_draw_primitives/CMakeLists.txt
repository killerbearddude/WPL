add_executable(wpl_draw_primitives
  main.c
)

target_link_libraries(wpl_draw_primitives PRIVATE wpl)

target_compile_options(wpl_draw_primitives PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
