cmake_minimum_required(VERSION 3.16)

project(wpl
  VERSION 0.1.0
  DESCRIPTION "Whacky Platform Layer"
  LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(X11 REQUIRED)

enable_testing()

add_library(wpl_core STATIC
  src/wpl_base.c
  src/wpl_time.c
  src/wpl_draw.c
  src/wpl_canvas.c
  src/wpl_file.c
  src/wpl_replay.c
  src/wpl_debug.c
)

target_include_directories(wpl_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(wpl_linux_x11 STATIC
  backends/linux_x11/wpl_linux_x11_window.c
  backends/linux_x11/wpl_linux_x11_input.c
  backends/linux_x11/wpl_linux_x11_renderer.c
)

target_include_directories(wpl_linux_x11
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/linux_x11
    ${X11_INCLUDE_DIR}
)

target_link_libraries(wpl_linux_x11
  PRIVATE
    wpl_core
    ${X11_LIBRARIES}
)

add_library(wpl INTERFACE)
target_link_libraries(wpl INTERFACE wpl_linux_x11 wpl_core)

foreach(wpl_target IN ITEMS wpl_core wpl_linux_x11)
  target_compile_options(${wpl_target} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
  )
endforeach()

add_subdirectory(examples/00_empty_window)
add_subdirectory(examples/01_input_snapshot)

add_executable(wpl_test_input_state
  tests/test_input_state.c
)
target_link_libraries(wpl_test_input_state PRIVATE wpl)
target_compile_options(wpl_test_input_state PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
add_test(NAME wpl_test_input_state COMMAND wpl_test_input_state)

add_executable(wpl_test_draw_list
  tests/test_draw_list.c
)
target_link_libraries(wpl_test_draw_list PRIVATE wpl_core)
target_compile_options(wpl_test_draw_list PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
add_test(NAME wpl_test_draw_list COMMAND wpl_test_draw_list)
