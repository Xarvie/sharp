# CMake build for Sharp Compiler (sharpc)
# Replaces the old Makefile

cmake_minimum_required(VERSION 3.10)

project(sharpc
    VERSION 0.4
    DESCRIPTION "C + Rust-style impl + C++-style templates + RAII"
    LANGUAGES C
)

# C11 standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Compiler flags (matches original Makefile)
add_compile_options(
    -O2
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wno-unused-function
    -Wno-switch
)

# Source files — compiler
set(SOURCES
    util.c
    types.c
    lexer.c
    parser.c
    sema.c
    lower.c
    hir.c
    cgen.c
    main.c
)

# Header files (for IDEs)
set(HEADERS
    sharp.h
    types.h
    ast.h
    hir.h
)

# Main executable
add_executable(sharpc ${SOURCES})
target_link_libraries(sharpc PRIVATE sharp_cpp)
target_include_directories(sharpc PRIVATE ${CMAKE_SOURCE_DIR}/cpp)

# -----------------------------------------------------------------------
# libtcc integration — embed TinyCC for self-hosted C→binary compilation
# -----------------------------------------------------------------------

set(TCC_DIR ${CMAKE_SOURCE_DIR}/third_party/tcc)

# Dynamic loading of libtcc.dll at runtime — just need include path for libtcc.h
target_include_directories(sharpc PRIVATE ${TCC_DIR}/libtcc)

# -----------------------------------------------------------------------
# Preprocessor library (cpp/)
# -----------------------------------------------------------------------

set(CPP_SOURCES
    cpp/pptok.c
    cpp/macro.c
    cpp/expr.c
    cpp/directive.c
    cpp/cpp.c
)

add_library(sharp_cpp STATIC ${CPP_SOURCES})
target_include_directories(sharp_cpp PUBLIC ${CMAKE_SOURCE_DIR}/cpp)
target_compile_options(sharp_cpp PRIVATE -Wno-implicit-fallthrough)

# Standalone sharp-cpp preprocessor driver
add_executable(sharp-cpp cpp/cpp_main.c)
target_link_libraries(sharp-cpp PRIVATE sharp_cpp)
target_include_directories(sharp-cpp PRIVATE ${CMAKE_SOURCE_DIR}/cpp)

install(TARGETS sharp-cpp RUNTIME DESTINATION bin)

# Install target
install(TARGETS sharpc sharp-cpp
    RUNTIME DESTINATION bin
)

# ----------------------------------------------------------------------
# Testing
# ----------------------------------------------------------------------

enable_testing()

# Unit tests for types module
add_executable(test_types
    tests/test_types.c
    types.c
    util.c
)
# Add include directory so tests can find headers
target_include_directories(test_types PRIVATE ${CMAKE_SOURCE_DIR})
add_test(NAME unit_types COMMAND test_types)

# Smoke test for the preprocessor (verifies it builds and handles trivial input)
add_executable(test_cpp_smoke
    tests/test_cpp_smoke.c
)
target_link_libraries(test_cpp_smoke PRIVATE sharp_cpp)
target_include_directories(test_cpp_smoke PRIVATE ${CMAKE_SOURCE_DIR}/cpp)
add_test(NAME unit_cpp_smoke COMMAND test_cpp_smoke)

# Regression tests for bugs identified during code review
add_executable(test_cpp_regression
    tests/test_cpp_regression.c
)
target_link_libraries(test_cpp_regression PRIVATE sharp_cpp)
target_include_directories(test_cpp_regression PRIVATE ${CMAKE_SOURCE_DIR}/cpp)
add_test(NAME unit_cpp_regression COMMAND test_cpp_regression)

# Second round regression tests
add_executable(test_cpp_regression2
    tests/test_cpp_regression2.c
)
target_link_libraries(test_cpp_regression2 PRIVATE sharp_cpp)
target_include_directories(test_cpp_regression2 PRIVATE ${CMAKE_SOURCE_DIR}/cpp)
add_test(NAME unit_cpp_regression2 COMMAND test_cpp_regression2)

# Third round regression tests
add_executable(test_cpp_regression3
    tests/test_cpp_regression3.c
)
target_link_libraries(test_cpp_regression3 PRIVATE sharp_cpp)
target_include_directories(test_cpp_regression3 PRIVATE ${CMAKE_SOURCE_DIR}/cpp)
add_test(NAME unit_cpp_regression3 COMMAND test_cpp_regression3)

# Fourth round regression tests
add_executable(test_cpp_regression4
    tests/test_cpp_regression4.c
)
target_link_libraries(test_cpp_regression4 PRIVATE sharp_cpp)
target_include_directories(test_cpp_regression4 PRIVATE ${CMAKE_SOURCE_DIR}/cpp)
add_test(NAME unit_cpp_regression4 COMMAND test_cpp_regression4)

# Fifth round regression tests
add_executable(test_cpp_regression5
    tests/test_cpp_regression5.c
)
target_link_libraries(test_cpp_regression5 PRIVATE sharp_cpp)
target_include_directories(test_cpp_regression5 PRIVATE ${CMAKE_SOURCE_DIR}/cpp)
add_test(NAME unit_cpp_regression5 COMMAND test_cpp_regression5)

# Sixth round regression tests
add_executable(test_cpp_regression6
    tests/test_cpp_regression6.c
)
target_link_libraries(test_cpp_regression6 PRIVATE sharp_cpp)
target_include_directories(test_cpp_regression6 PRIVATE ${CMAKE_SOURCE_DIR}/cpp)
add_test(NAME unit_cpp_regression6 COMMAND test_cpp_regression6)

# Seventh round regression tests
add_executable(test_cpp_regression7
    tests/test_cpp_regression7.c
)
target_link_libraries(test_cpp_regression7 PRIVATE sharp_cpp)
target_include_directories(test_cpp_regression7 PRIVATE ${CMAKE_SOURCE_DIR}/cpp)
add_test(NAME unit_cpp_regression7 COMMAND test_cpp_regression7)

# Eighth round regression tests
add_executable(test_cpp_regression8
    tests/test_cpp_regression8.c
)
target_link_libraries(test_cpp_regression8 PRIVATE sharp_cpp)
target_include_directories(test_cpp_regression8 PRIVATE ${CMAKE_SOURCE_DIR}/cpp)
add_test(NAME unit_cpp_regression8 COMMAND test_cpp_regression8)

# Ninth round regression tests - C header inclusion
add_executable(test_cpp_regression9
    tests/test_cpp_regression9.c
)
target_link_libraries(test_cpp_regression9 PRIVATE sharp_cpp)
target_include_directories(test_cpp_regression9 PRIVATE ${CMAKE_SOURCE_DIR}/cpp ${CMAKE_SOURCE_DIR})
add_test(NAME unit_cpp_regression9 COMMAND test_cpp_regression9)

# -----------------------------------------------------------------------
# Positive integration test (compile each example, check exit code)
# -----------------------------------------------------------------------
add_custom_target(test_positive
    COMMAND ${CMAKE_MAKE_PROGRAM} -C ${CMAKE_SOURCE_DIR} test-positive
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS sharpc
    COMMENT "Running positive integration tests"
)

# ----------------------------------------------------------------------
# Convenience targets (matching Makefile functionality)
# ----------------------------------------------------------------------

# Compile and run a Sharp file
# Usage: cmake --build . --target run -- FILE=examples/hello.sp
add_custom_target(run
    COMMAND echo "Usage: cmake --build . --target run -- FILE=examples/hello.sp"
    DEPENDS sharpc
)

# Clean (CMake handles this via --build . --target clean)
# But add a custom target for compatibility
add_custom_target(distclean
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove
        ${CMAKE_BINARY_DIR}/CMakeCache.txt
        ${CMAKE_BINARY_DIR}/cmake_install.cmake
        ${CMAKE_BINARY_DIR}/CTestTestfile.cmake
        ${CMAKE_BINARY_DIR}/Makefile
)

# Print configuration summary
message(STATUS "")
message(STATUS "Sharp Compiler (sharpc) v${PROJECT_VERSION}")
message(STATUS "  C Standard: ${CMAKE_C_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
