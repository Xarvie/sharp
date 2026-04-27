/* compiler: any */
/* Feature 6: wchar_t / wint_t types - use without system wchar.h */

typedef int wchar_t;
typedef long wint_t;

int main() {
    wchar_t c = 0;
    wchar_t* ws = 0;
    wint_t wc = 0;
    return 0;
}