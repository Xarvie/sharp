/* compiler: any */
/* Feature 6: wchar_t / wint_t types */

extern i32 fwprintf(void* stream, const wchar_t* fmt, ...);
extern wint_t fgetwc(void* stream);

i32 main() {
    wchar_t* ws;
    wint_t wc;
    return 0;
}
