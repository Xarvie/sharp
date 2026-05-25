/*
 * cpp_sys_paths.h — System include path discovery for sharp-cpp.
 *
 * Mirrors clang's heuristic search for system headers on each platform:
 *   Windows:  Clang builtin dirs (relative to exe) + Windows SDK (registry
 *             → env → filesystem scan) + MSVC toolchain (env → registry).
 *   Linux:    /usr/include + GCC versioned dirs.
 *   macOS:    Xcode SDK via xcrun.
 */
#ifndef CPP_SYS_PATHS_H
#define CPP_SYS_PATHS_H

#include "cpp.h"

/*
 * cpp_detect_sys_include_paths — auto-detect system include directories
 * and append them to ctx->sys_include_paths.
 *
 * Call this BEFORE processing user -I/-isystem flags so that user-specified
 * paths always have higher priority (they are appended later and searched
 * first by find_include_ex).
 *
 * Safe to call even when detection fails — the function silently returns
 * without adding any paths.
 */
void cpp_detect_sys_include_paths(CppCtx *ctx);

/*
 * cpp_detect_target_sys_paths — auto-detect system include directories
 * for a CROSS-COMPILATION target triple (e.g. "x86_64-macos",
 * "aarch64-linux-gnu").
 *
 * Resolves the zig libc layout:
 *   <zig>/lib/libc/include/<platform-arch-any>/
 *   <zig>/lib/libc/mingw/include/        (for Windows target)
 *
 * This is the primary mechanism for cross-compilation on any host.
 * When target is "windows", also calls detect_msvc_paths /
 * detect_windows_sdk_paths for platform-specific SDK paths.
 */
void cpp_detect_target_sys_paths(CppCtx *ctx, const char *target);

/**
 * cpp_detect_zig_sys_paths — Query zig cc directly (zig cc -E -v) to
 * discover system include directories with correct priority.
 *
 * This is the preferred method: it works on any host for any target
 * and does not hardcode directory layouts.
 * Falls back to cpp_detect_sys_include_paths() if zig is unavailable.
 */
void cpp_detect_zig_sys_paths(CppCtx *ctx, const char *target);

/**
 * Return the number of system include directories currently registered
 * in ctx (populated by cpp_detect_sys_include_paths or
 * cpp_detect_target_sys_paths).  Returns 0 if ctx is NULL. */
size_t cpp_sys_include_count(const CppCtx *ctx);

/**
 * Return the system include directory at index idx (0-based).
 * Returns NULL if idx is out of range or ctx is NULL. */
const char *cpp_sys_include(const CppCtx *ctx, size_t idx);

#endif /* CPP_SYS_PATHS_H */
