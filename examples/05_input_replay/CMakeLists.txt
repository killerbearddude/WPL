add_executable(wpl_input_replay main.c)
target_link_libraries(wpl_input_replay PRIVATE wpl)
target_compile_options(wpl_input_replay PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
