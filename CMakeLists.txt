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
# GCC/Clang flags
if(NOT MSVC)
    add_compile_options(
        -O2
        -Wall
        -Wextra
        -Wno-unused-parameter
        -Wno-unused-function
        -Wno-switch
    )
endif()
# MSVC-specific flags
if(MSVC)
    add_compile_options(/wd4028 /wd4244 /wd4267 /wd4018 /wd4142)
    # Disable Runtime Checks (/RTC1) which conflicts with /O2 in MSVC
    string(REPLACE "/RTC1" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
endif()

# -----------------------------------------------------------------------
# Preprocessor library (src/preproc/) — MUST be defined BEFORE sharpc
# -----------------------------------------------------------------------

set(CPP_SOURCES
    src/preproc/pptok.c
    src/preproc/macro.c
    src/preproc/expr.c
    src/preproc/directive.c
    src/preproc/cpp.c
)

add_library(sharp_preproc STATIC ${CPP_SOURCES})
target_include_directories(sharp_preproc PUBLIC ${CMAKE_SOURCE_DIR}/src/preproc)
target_compile_options(sharp_preproc PRIVATE $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wno-implicit-fallthrough>)

# Standalone sharp-cpp preprocessor driver
add_executable(sharp-cpp src/preproc/cpp_main.c)
target_link_libraries(sharp-cpp PRIVATE sharp_preproc)
target_include_directories(sharp-cpp PRIVATE ${CMAKE_SOURCE_DIR}/src/preproc)

# -----------------------------------------------------------------------
# libtcc integration — embed TinyCC for self-hosted C→binary compilation
# -----------------------------------------------------------------------

set(TCC_DIR ${CMAKE_SOURCE_DIR}/third_party/tcc)

# Dynamic loading of libtcc.dll at runtime — just need include path for libtcc.h
# -----------------------------------------------------------------------

# Source files — compiler
set(SOURCES
    src/util.c
    src/types.c
    src/lexer.c
    src/parser.c
    src/sema.c
    src/lower.c
    src/hir.c
    src/cgen.c
    src/main.c
)

# Header files (for IDEs)
set(HEADERS
    src/sharp.h
    src/types.h
    src/ast.h
    src/hir.h
)

# Main executable
add_executable(sharpc ${SOURCES})
target_link_libraries(sharpc PRIVATE sharp_preproc)
target_include_directories(sharpc PRIVATE
    ${CMAKE_SOURCE_DIR}/src/preproc
    ${CMAKE_SOURCE_DIR}/src
    ${TCC_DIR}/libtcc
)

install(TARGETS sharpc sharp-cpp
    RUNTIME DESTINATION bin
)

# ----------------------------------------------------------------------
# Testing
# ----------------------------------------------------------------------

enable_testing()

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
