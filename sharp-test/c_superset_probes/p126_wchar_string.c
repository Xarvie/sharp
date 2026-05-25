/* p126 — wide character string literals: L"string"
 * KNOWN-ERROR: sharpc reports "unknown type 'wchar_t'" when the type
 * appears without <wchar.h> or <stddef.h>.
 * Note: in practice wchar_t comes via system headers and tokcmp passes
 * --isystem flags, so this variant tests the bare case.
 */
#include <stddef.h>
#include <wchar.h>
wchar_t wc = L'A';
const wchar_t *ws = L"hello world";
int main(void) { return (int)wc - 65; }
