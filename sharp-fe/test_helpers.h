/*
 * test_helpers.h — Shared test-framework macros for sharp-fe test suites.
 *
 * Every test_phaseN.c includes this header.  Macros mirror the style
 * established in sharp-cpp/test_phase1.c so that the two projects feel
 * identical to a reader jumping between them.
 *
 * Global counters g_pass / g_fail must be defined in the .c file:
 *
 *   static int g_pass = 0, g_fail = 0;
 *
 * Each test function is self-contained: it creates its own context,
 * exercises one feature, asserts with the macros below, and tears down.
 * No global state, no ordering dependency.
 */
#ifndef SHARP_FE_TEST_HELPERS_H
#define SHARP_FE_TEST_HELPERS_H

#include <stdio.h>
#include <string.h>

/* g_pass and g_fail must be declared by the including .c file. */



#define CHECK(cond, msg) do {                                               \
    if (cond) { printf("    PASS: %s\n", msg); g_pass++; }                 \
    else      { printf("    FAIL: %s  (%s:%d)\n", msg, __FILE__, __LINE__);\
                g_fail++; }                                                 \
} while (0)

#define CHECK_EQ_INT(a, b, msg) do {                                        \
    long long _a = (long long)(a), _b = (long long)(b);                     \
    if (_a == _b) { printf("    PASS: %s\n", msg); g_pass++; }              \
    else { printf("    FAIL: %s  (got %lld, want %lld)  %s:%d\n",           \
                  msg, _a, _b, __FILE__, __LINE__); g_fail++; }             \
} while (0)

#define CHECK_STR_EQ(a, b, msg) do {                                        \
    if (strcmp((a),(b)) == 0) { printf("    PASS: %s\n", msg); g_pass++; }  \
    else { printf("    FAIL: %s\n      got : >>%s<<\n      want: >>%s<<\n", \
                  msg, (a), (b)); g_fail++; }                               \
} while (0)

#define CHECK_SUBSTR(haystack, needle, msg) do {                            \
    if ((haystack) && strstr((haystack),(needle)))                          \
        { printf("    PASS: %s\n", msg); g_pass++; }                       \
    else { printf("    FAIL: %s\n      text: >>%s<<\n      need: >>%s<<\n", \
                  msg, (haystack)?(haystack):"(null)", (needle)); g_fail++;}\
} while (0)

#define CHECK_NO_SUBSTR(haystack, needle, msg) do {                         \
    if (!(haystack) || !strstr((haystack),(needle)))                        \
        { printf("    PASS: %s\n", msg); g_pass++; }                       \
    else { printf("    FAIL: %s  (found unwanted: >>%s<<)\n",               \
                  msg, (needle)); g_fail++; }                               \
} while (0)

#endif /* SHARP_FE_TEST_HELPERS_H */
