cmake_minimum_required(VERSION 3.16)

project(wpl
  VERSION 0.2.0
  DESCRIPTION "Whacky Platform Layer"
  LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(WPL_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer." OFF)

enable_testing()

include(CheckLanguage)
check_language(CXX)
if(CMAKE_CXX_COMPILER)
  enable_language(CXX)
endif()

if(WPL_ENABLE_SANITIZERS)
  if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_library(wpl_sanitizers INTERFACE)
    target_compile_options(wpl_sanitizers INTERFACE
      -fsanitize=address,undefined
      -fno-omit-frame-pointer
    )
    target_link_options(wpl_sanitizers INTERFACE
      -fsanitize=address,undefined
      -fno-omit-frame-pointer
    )
  else()
    message(FATAL_ERROR "WPL_ENABLE_SANITIZERS requires GCC or Clang")
  endif()
endif()

add_library(wpl_core STATIC
  src/wpl_base.c
  src/wpl_time.c
  src/wpl_draw.c
  src/wpl_canvas.c
  src/wpl_file.c
  src/wpl_replay.c
  src/wpl_debug.c
  src/wpl_log.c
)

target_include_directories(wpl_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(wpl_core
  PUBLIC
    m
)

if(WPL_ENABLE_SANITIZERS)
  target_link_libraries(wpl_core PUBLIC wpl_sanitizers)
endif()

find_package(X11 REQUIRED)

add_library(wpl_linux_x11 STATIC
  backends/linux_x11/wpl_linux_x11_backend.c
  backends/linux_x11/wpl_linux_x11_window.c
  backends/linux_x11/wpl_linux_x11_input.c
  backends/linux_x11/wpl_linux_x11_renderer.c
  backends/linux_x11/wpl_linux_x11_font.c
)

set_source_files_properties(
  backends/linux_x11/wpl_linux_x11_window.c
  PROPERTIES
    COMPILE_DEFINITIONS WPL_LINUX_X11_RENAME_WINDOW_PUBLIC_SYMBOLS
)

set_source_files_properties(
  backends/linux_x11/wpl_linux_x11_renderer.c
  PROPERTIES
    COMPILE_DEFINITIONS WPL_LINUX_X11_RENAME_RENDERER_PUBLIC_SYMBOLS
)

target_include_directories(wpl_linux_x11
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/linux_x11
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${X11_INCLUDE_DIR}
)

target_link_libraries(wpl_linux_x11
  PUBLIC
    wpl_core
  PRIVATE
    ${X11_LIBRARIES}
)

if(WPL_ENABLE_SANITIZERS)
  target_link_libraries(wpl_linux_x11 PUBLIC wpl_sanitizers)
endif()

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
add_subdirectory(examples/02_draw_primitives)
add_subdirectory(examples/03_canvas_pan_zoom)
add_subdirectory(examples/04_debug_overlay)
add_subdirectory(examples/05_input_replay)

add_executable(wpl_test_input_state
  tests/test_input_state.c
)
target_include_directories(wpl_test_input_state
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/linux_x11
    ${X11_INCLUDE_DIR}
)
target_link_libraries(wpl_test_input_state PRIVATE wpl)
target_compile_options(wpl_test_input_state PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
add_test(NAME wpl_test_input_state COMMAND wpl_test_input_state)

add_executable(wpl_test_input_window_isolation
  tests/test_input_window_isolation.c
)
target_include_directories(wpl_test_input_window_isolation
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/linux_x11
    ${X11_INCLUDE_DIR}
)
target_link_libraries(wpl_test_input_window_isolation PRIVATE wpl)
target_compile_options(wpl_test_input_window_isolation PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
add_test(NAME wpl_test_input_window_isolation COMMAND wpl_test_input_window_isolation)

add_executable(wpl_test_time
  tests/test_time.c
)
target_include_directories(wpl_test_time
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/linux_x11
    ${X11_INCLUDE_DIR}
)
target_link_libraries(wpl_test_time PRIVATE wpl)
target_compile_options(wpl_test_time PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
add_test(NAME wpl_test_time COMMAND wpl_test_time)

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

add_executable(wpl_test_draw_derived_geometry
  tests/test_draw_derived_geometry.c
)
target_link_libraries(wpl_test_draw_derived_geometry PRIVATE wpl_core)
target_compile_options(wpl_test_draw_derived_geometry PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
add_test(NAME wpl_test_draw_derived_geometry COMMAND wpl_test_draw_derived_geometry)

add_executable(wpl_test_text_metrics
  tests/test_text_metrics.c
)
target_link_libraries(wpl_test_text_metrics PRIVATE wpl_core)
target_compile_options(wpl_test_text_metrics PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
add_test(NAME wpl_test_text_metrics COMMAND wpl_test_text_metrics)

add_executable(wpl_test_canvas
  tests/test_canvas.c
)
target_link_libraries(wpl_test_canvas PRIVATE wpl_core)
target_compile_options(wpl_test_canvas PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
add_test(NAME wpl_test_canvas COMMAND wpl_test_canvas)

add_executable(wpl_test_debug_overlay
  tests/test_debug_overlay.c
)
target_link_libraries(wpl_test_debug_overlay PRIVATE wpl_core)
target_compile_options(wpl_test_debug_overlay PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
add_test(NAME wpl_test_debug_overlay COMMAND wpl_test_debug_overlay)


add_executable(wpl_test_file_io
  tests/test_file_io.c
)
target_link_libraries(wpl_test_file_io PRIVATE wpl_core)
target_compile_options(wpl_test_file_io PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
add_test(NAME wpl_test_file_io COMMAND wpl_test_file_io)

add_executable(wpl_test_replay_format
  tests/test_replay_format.c
)
target_include_directories(wpl_test_replay_format
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(wpl_test_replay_format PRIVATE wpl_core)
target_compile_options(wpl_test_replay_format PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
add_test(NAME wpl_test_replay_format COMMAND wpl_test_replay_format)

add_executable(wpl_test_replay
  tests/test_replay.c
)
target_include_directories(wpl_test_replay
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(wpl_test_replay PRIVATE wpl_core)
target_compile_options(wpl_test_replay PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
add_test(NAME wpl_test_replay COMMAND wpl_test_replay)

add_executable(wpl_test_log
  tests/test_log.c
)
target_link_libraries(wpl_test_log PRIVATE wpl_core)
target_compile_options(wpl_test_log PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
add_test(NAME wpl_test_log COMMAND wpl_test_log)

add_executable(wpl_test_public_headers
  tests/test_public_headers.c
)
target_link_libraries(wpl_test_public_headers PRIVATE wpl_core)
target_compile_options(wpl_test_public_headers PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
add_test(NAME wpl_test_public_headers COMMAND wpl_test_public_headers)

add_executable(wpl_test_backend_interface
  tests/test_backend_interface.c
)
target_include_directories(wpl_test_backend_interface
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/backends/linux_x11
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${X11_INCLUDE_DIR}
)
target_link_libraries(wpl_test_backend_interface PRIVATE wpl)
target_compile_options(wpl_test_backend_interface PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
add_test(NAME wpl_test_backend_interface COMMAND wpl_test_backend_interface)

add_executable(wpl_test_backend_dispatch
  tests/test_backend_dispatch.c
)
target_link_libraries(wpl_test_backend_dispatch PRIVATE wpl)
target_compile_options(wpl_test_backend_dispatch PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
add_test(NAME wpl_test_backend_dispatch COMMAND wpl_test_backend_dispatch)

add_executable(wpl_test_window_api
  tests/test_window_api.c
)
target_link_libraries(wpl_test_window_api PRIVATE wpl)
target_compile_options(wpl_test_window_api PRIVATE
  -Wall
  -Wextra
  -Wpedantic
)
add_test(NAME wpl_test_window_api COMMAND wpl_test_window_api)

if(CMAKE_CXX_COMPILER)
  add_executable(wpl_test_public_headers_cpp
    tests/test_public_headers_cpp.cpp
  )
  target_link_libraries(wpl_test_public_headers_cpp PRIVATE wpl_core)
  target_compile_options(wpl_test_public_headers_cpp PRIVATE
    -Wall
    -Wextra
    -Wpedantic
  )
  add_test(NAME wpl_test_public_headers_cpp COMMAND wpl_test_public_headers_cpp)
endif()
