/* compiler: any */
/* Feature 2: _CRTIMP macro expansion chain */

#define _CRTIMP __declspec(dllimport)

_CRTIMP i32 printf(const char* fmt, ...);

i32 main() {
    return 0;
}
