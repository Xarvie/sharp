/*
 * cpp_targets.c — Call zig cc -E to fetch predefined macros.
 *
 * Instead of maintaining hardcoded predefined macros for each target,
 * we invoke `zig cc -E -dM -target <triple>` to obtain the exact
 * macro set that Zig reports for that target.  This guarantees
 * correctness without manual maintenance.
 */
#include "cpp.h"
#include "macro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <pthread.h>
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
    char *full = malloc(len);
    if (!full) return NULL;
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
    fseek(f, 0, SEEK_SET);
    char *buf = malloc((size_t)flen + 1);
    if (buf) {
        size_t rd = fread(buf, 1, (size_t)flen, f);
        buf[rd] = '\0';
    }
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
                    char *name = malloc(sig_len + 1);
                    memcpy(name, fbody, sig_len);
                    name[sig_len] = '\0';

                    const char *val = rp + 1;
                    while (*val == ' ' || *val == '\t') val++;
                    size_t val_len = len - (size_t)(val - line);
                    if (val_len > 0 && val[val_len - 1] == '\r') val_len--;
                    char *value = malloc(val_len + 1);
                    memcpy(value, val, val_len);
                    value[val_len] = '\0';

                    cpp_define(ctx, name, value);

                    free(name);
                    free(value);
                }
            } else {
                /* Object-like macro: #define NAME value */
                char *name = malloc(name_len + 1);
                memcpy(name, body, name_len);
                name[name_len] = '\0';

                const char *val = name_end;
                while (*val == ' ' || *val == '\t') val++;
                size_t val_len = len - (size_t)(val - line);
                if (val_len > 0 && val[val_len - 1] == '\r') val_len--;
                char *value = malloc(val_len + 1);
                memcpy(value, val, val_len);
                value[val_len] = '\0';

                cpp_define(ctx, name, value);

                /* Detect __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ so we
                 * can define __ENABLE_LEGACY_MAC_AVAILABILITY downstream.      */
                if (strcmp(name, "__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__") == 0) {
                    has_env_mac_min_required = 1;
                    env_mac_min_value = malloc(strlen(value) + 1);
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
 * Find zig executable path
 * ----------------------------------------------------------------- */
#ifdef _WIN32
#include <windows.h>

static int g_zig_path_found = 0;
static char g_zig_path[MAX_PATH] = {0};

static const char *find_zig_exe(void)
{
    if (g_zig_path_found)
        return g_zig_path[0] ? g_zig_path : NULL;
    g_zig_path_found = 1;

    /* First try: PATH search */
    if (SearchPathA(NULL, "zig.exe", NULL, sizeof(g_zig_path), g_zig_path, NULL))
        return g_zig_path;

    /* Fallback: known locations */
    const char *fallbacks[] = {
        "C:\\env\\zig\\zig.exe",
        "C:\\Program Files\\zig\\zig.exe",
        "C:\\zig\\zig.exe",
    };
    for (size_t k = 0; k < sizeof(fallbacks)/sizeof(fallbacks[0]); k++) {
        DWORD a = GetFileAttributesA(fallbacks[k]);
        if (a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY)) {
            strncpy(g_zig_path, fallbacks[k], sizeof(g_zig_path) - 1);
            return g_zig_path;
        }
    }
    return NULL;
}

const char *cpp_find_zig_exe(void)
{
    return find_zig_exe();
}
#else
static int g_zig_path_found = 0;
static char g_zig_path[4096] = {0};

static const char *find_zig_exe(void)
{
    if (g_zig_path_found)
        return g_zig_path[0] ? g_zig_path : NULL;
    g_zig_path_found = 1;

    const char *path_env = getenv("PATH");
    if (path_env) {
        const char *start = path_env;
        while (*start) {
            const char *end = strchr(start, ':');
            size_t len = end ? (size_t)(end - start) : strlen(start);
            if (len > 0 && len < sizeof(g_zig_path) - 5) {
                memcpy(g_zig_path, start, len);
                memcpy(g_zig_path + len, "/zig", 5);
                if (access(g_zig_path, X_OK) == 0)
                    return g_zig_path;
            }
            if (!end) break;
            start = end + 1;
        }
    }

    const char *fallbacks[] = {
        "/usr/local/zig/zig",
        "/usr/bin/zig",
        "/usr/local/bin/zig",
        "/opt/zig/zig",
    };
    for (size_t k = 0; k < sizeof(fallbacks)/sizeof(fallbacks[0]); k++) {
        if (access(fallbacks[k], X_OK) == 0) {
            strncpy(g_zig_path, fallbacks[k], sizeof(g_zig_path) - 1);
            return g_zig_path;
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

    snprintf(cmd, sizeof(cmd), "%s cc -E -dM -std=c11%s -xc %s%s %s",
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
