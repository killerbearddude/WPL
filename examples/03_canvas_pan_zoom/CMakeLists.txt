add_executable(wpl_canvas_pan_zoom main.c)
target_link_libraries(wpl_canvas_pan_zoom PRIVATE wpl)
target_compile_options(wpl_canvas_pan_zoom PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
