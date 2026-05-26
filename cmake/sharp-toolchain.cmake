# Sharp toolchain file for CMake
#
# Usage:
#   cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/sharp-toolchain.cmake
#   cmake --build build
#
# This toolchain tricks CMake into treating sharpc as a GNU-like C compiler
# and registers .sp / .sph as C source / header file extensions.

# ── Compiler identity ────────────────────────────────────────────────
set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR "x86_64")

set(CMAKE_C_COMPILER    "sharpc")
set(CMAKE_C_COMPILER_ID "GNU")
set(CMAKE_C_COMPILER_VERSION "1.0")

# ── File extensions ──────────────────────────────────────────────────
# Register .sp as a C source file extension so CMake treats *.sp like *.c
# Must be a regular variable (not CACHE) to take effect before project().
set(CMAKE_C_SOURCE_FILE_EXTENSIONS "c;C;sp")

# ── Compiler flags ───────────────────────────────────────────────────
# Compile a single source to an object (no linking)
set(CMAKE_C_COMPILE_OBJECT
    "<CMAKE_C_COMPILER> -c <DEFINES> <INCLUDES> <FLAGS> <SOURCE> -o <OBJECT>")

# Link object files into executable
set(CMAKE_C_LINK_EXECUTABLE
    "<CMAKE_C_COMPILER> <OBJECTS> -o <TARGET> <LINK_FLAGS> <LINK_LIBRARIES>")

# Create a static library (archive .o files)
set(CMAKE_C_CREATE_STATIC_LIBRARY
    "<CMAKE_AR> cr <TARGET> <OBJECTS>")
set(CMAKE_C_ARCHIVE_CREATE "${CMAKE_C_CREATE_STATIC_LIBRARY}")
set(CMAKE_C_ARCHIVE_FINISH "")

# ── Compiler feature detection overrides ─────────────────────────────
# Skip CMake's compiler feature detection (__STDC_VERSION__ etc.)
# sharpc passes -c tests but feature detection macros may differ from GNU.
set(CMAKE_C_COMPILE_FEATURES "c_std_11")

# ── Try-compile settings ─────────────────────────────────────────────
# Don't let CMake second-guess these
set(CMAKE_C_FLAGS_INIT "")
set(CMAKE_C_FLAGS_DEBUG_INIT "-g")
set(CMAKE_C_FLAGS_RELEASE_INIT "-O2")
set(CMAKE_C_FLAGS_MINSIZEREL_INIT "-Os")
set(CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "-O2 -g")