cmake_minimum_required(VERSION 3.14)

project(sharp
  VERSION 1.0.0
  DESCRIPTION "Sharp language compiler"
  LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall -Wextra -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -Wextra -DNDEBUG")

# -------------------------------------------------------------------
# Preprocessor / compiler sources
# -------------------------------------------------------------------

set(CPP_SOURCES
  sharp-cpp/cpp.c
  sharp-cpp/cpp_sys_paths.c
  sharp-cpp/cpp_targets.c
  sharp-cpp/directive.c
  sharp-cpp/expr.c
  sharp-cpp/macro.c
  sharp-cpp/pptok.c
)

add_executable(sharpc
  ${CPP_SOURCES}
  sharp-fe/ast.c
  sharp-fe/cg.c
  sharp-fe/lex.c
  sharp-fe/parse.c
  sharp-fe/scope.c
  sharp-fe/sema.c
  sharp-fe/sharp.c
  sharp-fe/sharpc.c
  sharp-fe/type.c
)

target_include_directories(sharpc PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/sharp-fe
  ${CMAKE_CURRENT_SOURCE_DIR}/sharp-cpp
)

# target_link_libraries(sharpc PRIVATE sharp_socket sharp_poller)

if(WIN32)
  target_link_libraries(sharpc PRIVATE shlwapi)
endif()

add_executable(probe_cpp
  ${CPP_SOURCES}
  sharp-cpp/probe.c
)
target_include_directories(probe_cpp PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/sharp-cpp
)
# target_link_libraries(probe_cpp PRIVATE sharp_socket sharp_poller)

add_executable(probe_sharp
  ${CPP_SOURCES}
  sharp-test/probe.c
)
target_include_directories(probe_sharp PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/sharp-cpp
)
# target_link_libraries(probe_sharp PRIVATE sharp_socket sharp_poller)

if(WIN32)
  target_link_libraries(probe_cpp PRIVATE shlwapi)
  target_link_libraries(probe_sharp PRIVATE shlwapi)
endif()
