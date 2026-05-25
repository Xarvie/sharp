/* p129 — wide string and character literals: L"..." and L'x'
 * KNOWN-DIFFER: sharpc splits L"hello world" into separate tokens
 * Expected: L"hello world"
 * Actual:   "" hello world " (split/malformed)
 * Also: wchar_t type requires <stddef.h> or <wchar.h>
 */
#include <wchar.h>
wchar_t wc = L'A';
const wchar_t *ws = L"hello";
const wchar_t *ws2 = L"world";
int main(void) { return (int)wc - 65; }
