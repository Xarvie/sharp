/*
 * Bug: #ifndef include guard not auto-detected
 * Including the same header twice should NOT produce
 * "macro redefined" warnings when the header uses
 * traditional #ifndef guards.
 */
#include "bug_include_guard_hdr.h"
#include "bug_include_guard_hdr.h"
int main(void) { return GUARD_VAL - 42; }
