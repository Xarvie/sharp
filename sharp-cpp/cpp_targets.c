/*
 * cpp_targets.c — Call zig cc -E to fetch predefined macros.
 *
 * Instead of maintaining hardcoded predefined macros for each target,
 * we invoke `zig cc -E -dM -target <triple>` to obtain the exact
 * macro set that Zig reports for that target.  This guarantees
 * correctness without manual maintenance.
 */
#define _GNU_SOURCE
#include "cpp.h"
#include "cpp_internal.h"
#include "macro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#endif

/* -----------------------------------------------------------------
 * execZig: execute a command via system(), return malloc'd stdout.
 *
 * stderr goes to a unique temp file per-thread; on failure it is
 * printed to real stderr.  stdout is captured and returned.
 *
 * No pipes, no fork/exec, no CreateProcess.  Pure system() + file
 * redirect — zero risk of pipe deadlock or handle leakage.
 * ----------------------------------------------------------------- */

static char *
execZig(const char *cmd)
{
    char out_path[512], err_path[512];

#ifdef _WIN32
    const char *tmp = getenv("TEMP");
    if (!tmp) tmp = ".";
    snprintf(out_path, sizeof(out_path), "%s\\zig_out_%lu_%lu.tmp",
             tmp, (unsigned long)GetCurrentProcessId(),
             (unsigned long)GetCurrentThreadId());
    snprintf(err_path, sizeof(err_path), "%s\\zig_err_%lu_%lu.tmp",
             tmp, (unsigned long)GetCurrentProcessId(),
             (unsigned long)GetCurrentThreadId());
#else
    const char *tmp = getenv("TMPDIR");
    if (!tmp) tmp = "/tmp";
    snprintf(out_path, sizeof(out_path), "%s/zig_out_%d_%lu.tmp",
             tmp, (int)getpid(), (unsigned long)pthread_self());
    snprintf(err_path, sizeof(err_path), "%s/zig_err_%d_%lu.tmp",
             tmp, (int)getpid(), (unsigned long)pthread_self());
#endif

    size_t len = strlen(cmd) + 1024;
    char *full = cpp_xmalloc(len);
    snprintf(full, len, "%s > \"%s\" 2> \"%s\"", cmd, out_path, err_path);

    int exit_code = system(full);
    free(full);

    if (exit_code != 0) {
        FILE *f = fopen(err_path, "r");
        if (f) {
            char line[1024];
            while (fgets(line, sizeof(line), f))
                fprintf(stderr, "%s", line);
            fclose(f);
        }
        remove(out_path);
        remove(err_path);
        return NULL;
    }
    remove(err_path);

    FILE *f = fopen(out_path, "rb");
    if (!f) { remove(out_path); return NULL; }
    fseek(f, 0, SEEK_END);
    long flen = ftell(f);
    if (flen < 0) { fclose(f); remove(out_path); return NULL; }
    fseek(f, 0, SEEK_SET);
    char *buf = cpp_xmalloc((size_t)flen + 1);
    size_t rd = fread(buf, 1, (size_t)flen, f);
    buf[rd] = '\0';
    fclose(f);
    remove(out_path);
    return buf;
}

/* -----------------------------------------------------------------
 * Parse zig cc -E -dM output: "#define <name> <value>"
 * ----------------------------------------------------------------- */
static void
install_macros_from_zig(CppCtx *ctx, const char *zig_output)
{
    int has_env_mac_min_required = 0;
    char *env_mac_min_value = NULL;

    const char *p = zig_output;
    while (*p) {
        const char *line = p;
        const char *eol = strchr(p, '\n');
        size_t len = eol ? (size_t)(eol - p) : strlen(p);

        if (len >= 8 && strncmp(line, "#define ", 8) == 0) {
            const char *body = line + 8;

            /* Skip whitespace */
            while (*body == ' ' || *body == '\t') body++;

            const char *name_end = body;
            while (*name_end && *name_end != ' ' && *name_end != '\t' &&
                   *name_end != '(' && *name_end != '\n')
                name_end++;

            size_t name_len = (size_t)(name_end - body);

            if (*name_end == '(') {
                /* Function-like macro: #define FOO(x) ... */
                const char *fbody = line + 8;
                while (*fbody == ' ' || *fbody == '\t') fbody++;

                const char *rp = strchr(fbody, ')');
                if (rp) {
                    size_t sig_len = (size_t)(rp + 1 - fbody);
                    char *name = cpp_xmalloc(sig_len + 1);
                    memcpy(name, fbody, sig_len);
                    name[sig_len] = '\0';

                    const char *val = rp + 1;
                    while (*val == ' ' || *val == '\t') val++;
                    size_t val_len = len - (size_t)(val - line);
                    if (val_len > 0 && val[val_len - 1] == '\r') val_len--;
                    char *value = cpp_xmalloc(val_len + 1);
                    memcpy(value, val, val_len);
                    value[val_len] = '\0';

                    cpp_define(ctx, name, value);

                    free(name);
                    free(value);
                }
            } else {
                /* Object-like macro: #define NAME value */
                char *name = cpp_xmalloc(name_len + 1);
                memcpy(name, body, name_len);
                name[name_len] = '\0';

                const char *val = name_end;
                while (*val == ' ' || *val == '\t') val++;
                size_t val_len = len - (size_t)(val - line);
                if (val_len > 0 && val[val_len - 1] == '\r') val_len--;
                char *value = cpp_xmalloc(val_len + 1);
                memcpy(value, val, val_len);
                value[val_len] = '\0';

                cpp_define(ctx, name, value);

                /* Detect __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ so we
                 * can define __ENABLE_LEGACY_MAC_AVAILABILITY downstream.      */
                if (strcmp(name, "__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__") == 0) {
                    has_env_mac_min_required = 1;
                    env_mac_min_value = cpp_xmalloc(strlen(value) + 1);
                    strcpy(env_mac_min_value, value);
                }

                free(name);
                free(value);
            }
        }

        if (eol)
            p = eol + 1;
        else
            break;
    }

    /* macOS legacy availability macros require __ENABLE_LEGACY_MAC_AVAILABILITY
     * to be defined, otherwise AvailabilityInternalLegacy.h skips ALL the
     * __AVAILABILITY_INTERNAL__MAC_* definitions.  Clang/zig cc define this
     * implicitly when __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ is present,
     * but we must do it explicitly.                                          */
    if (has_env_mac_min_required) {
        cpp_define(ctx, "__ENABLE_LEGACY_MAC_AVAILABILITY", "1");

        /* Availability.h checks for __MAC_OS_X_VERSION_MIN_REQUIRED (without
         * the __ENVIRONMENT prefix) to decide which availability macros to
         * define.  AvailabilityInternal.h normally defines this as an alias
         * for __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ via
         * __has_builtin(__is_target_os) && __is_target_os(macos), but since
         * sharpc cannot evaluate __is_target_os() in #if expressions, we
         * must define it explicitly from the zig cc -dM data.               */
        if (env_mac_min_value)
            cpp_define(ctx, "__MAC_OS_X_VERSION_MIN_REQUIRED", env_mac_min_value);

        free(env_mac_min_value);
    }
}

/* -----------------------------------------------------------------
 * Find zig executable path (Windows)
 * ----------------------------------------------------------------- */
#ifdef _WIN32
#include <windows.h>

static const char *find_zig_exe(void)
{
    if (g_sess.zig_found)
        return g_sess.zig_exe[0] ? g_sess.zig_exe : NULL;
    g_sess.zig_found = 1;

    const char *root = getenv("SHARP_ROOT");
    if (root && root[0]) {
        size_t rlen = strlen(root);
        size_t need = rlen + 13; /* "\\zig\\zig.exe" + NUL */
        if (need < sizeof(g_sess.zig_exe)) {
            memcpy(g_sess.zig_exe, root, rlen);
            memcpy(g_sess.zig_exe + rlen, "\\zig\\zig.exe", 13);
            DWORD a = GetFileAttributesA(g_sess.zig_exe);
            if (a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY))
                return g_sess.zig_exe;
        }
    }

    return NULL;
}

const char *cpp_find_zig_exe(void)
{
    return find_zig_exe();
}
#else

/* -----------------------------------------------------------------
 * Find zig executable path (POSIX)
 * ----------------------------------------------------------------- */
static const char *find_zig_exe(void)
{
    if (g_sess.zig_found)
        return g_sess.zig_exe[0] ? g_sess.zig_exe : NULL;
    g_sess.zig_found = 1;

    const char *root = getenv("SHARP_ROOT");
    if (root && root[0]) {
        size_t rlen = strlen(root);
        size_t need = rlen + 9; /* "/zig/zig" + NUL */
        if (need < sizeof(g_sess.zig_exe)) {
            memcpy(g_sess.zig_exe, root, rlen);
            memcpy(g_sess.zig_exe + rlen, "/zig/zig", 9);
            struct stat st;
            if (stat(g_sess.zig_exe, &st) == 0 && !S_ISDIR(st.st_mode) &&
                access(g_sess.zig_exe, X_OK) == 0)
                return g_sess.zig_exe;
        }
    }

    return NULL;
}

const char *cpp_find_zig_exe(void)
{
    return find_zig_exe();
}
#endif

/* -----------------------------------------------------------------
 * Find zig installation directory (from SHARP_ROOT)
 * ----------------------------------------------------------------- */
const char *cpp_find_zig_install_dir(void)
{
    static char zig_install_dir[MAX_PATH];
    static int cached = 0;
    if (cached)
        return zig_install_dir[0] ? zig_install_dir : NULL;
    cached = 1;

    const char *root = getenv("SHARP_ROOT");
    if (root && root[0]) {
#ifdef _WIN32
        snprintf(zig_install_dir, sizeof(zig_install_dir), "%s\\zig", root);
        DWORD attrs = GetFileAttributesA(zig_install_dir);
        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY))
            return zig_install_dir;
#else
        snprintf(zig_install_dir, sizeof(zig_install_dir), "%s/zig", root);
        struct stat st;
        if (stat(zig_install_dir, &st) == 0 && S_ISDIR(st.st_mode))
            return zig_install_dir;
#endif
    }

    return NULL;
}

/* -----------------------------------------------------------------
 * Known target triples
 * ----------------------------------------------------------------- */
static const char *g_known_triples[] = {
    "x86_64-windows-gnu",
    "x86_64-linux-gnu",
    "aarch64-linux-gnu",
    "aarch64-macos",
    "x86_64-macos",
    "aarch64-ios",
    "x86_64-ios",
    "x86_64-linux-musl",
    "aarch64-linux-musl",
    NULL
};

const char *const *cpp_target_known_triples(void) {
    return g_known_triples;
}

int
cpp_probe_zig_macros(CppCtx *ctx, const char *target)
{
    if (!target || !target[0]) return -1;

    const char *zig_exe = find_zig_exe();
    if (!zig_exe) return -1;

    /* Build command string */
    char cmd[2048];
    int is_apple = 0;
#ifdef __APPLE__
    is_apple = !target;
#endif
    if (target) {
        is_apple = is_apple || strstr(target, "macos") || strstr(target, "ios")
                            || strstr(target, "tvos") || strstr(target, "watchos");
    }
    /* empty input file — zig cc needs a real file on Windows (NUL fails) */
    const char *empty_file;
#ifdef _WIN32
    char empty_path[MAX_PATH];
    DWORD tmp_len = GetTempPathA(sizeof(empty_path), empty_path);
    if (tmp_len == 0 || tmp_len >= sizeof(empty_path)) return -1;
    snprintf(empty_path + tmp_len, sizeof(empty_path) - tmp_len,
             "sp_%lu_%lu.c", (unsigned long)GetCurrentProcessId(),
             (unsigned long)GetCurrentThreadId());
    FILE *efp = fopen(empty_path, "w");
    if (!efp) return -1;
    fclose(efp);
    empty_file = empty_path;
#else
    empty_file = "/dev/null";
#endif

    snprintf(cmd, sizeof(cmd), "%s cc -E -dM -std=c23%s -xc %s%s %s",
             zig_exe,
             is_apple ? " -fno-blocks" : "",
             target ? "-target " : "",
             target ? target : "",
             empty_file);

    char *cap = execZig(cmd);
#ifdef _WIN32
    DeleteFileA(empty_path);
#endif
    if (!cap) return -1;
    if (cap[0] == '\0') { free(cap); return -1; }

    install_macros_from_zig(ctx, cap);
    free(cap);
    return 0;
}

int
cpp_probe_host_macros(CppCtx *ctx)
{
    return cpp_probe_zig_macros(ctx, NULL);
}
