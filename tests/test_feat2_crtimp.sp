/* compiler: any */
/* Feature 2: _CRTIMP macro expansion chain */

#define _CRTIMP __declspec(dllimport)

_CRTIMP int printf(const char* fmt, ...);

int main() {
    return 0;
}