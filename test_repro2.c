#include <stdio.h>
#include <string.h>

#define LUA_FLOAT_FLOAT 3
#define LUA_FLOAT_TYPE 3

#if LUA_FLOAT_TYPE == LUA_FLOAT_FLOAT
#define LUA_NUMBER float
#define LUA_NUMBER_FRMLEN ""
#define LUA_NUMBER_FMT "%.7g"
#elif LUA_FLOAT_TYPE == LUA_FLOAT_DOUBLE
#define LUA_NUMBER double
#define LUA_NUMBER_FRMLEN ""
#define LUA_NUMBER_FMT "%.14g"
#else
#define LUA_NUMBER long double
#define LUA_NUMBER_FRMLEN "ll"
#define LUA_NUMBER_FMT "%.19Lg"
#endif

#define L_FMTFLAGSF  "-+#0 "

static void addlenmod(char *form, const char *lenmod) {
    size_t len = strlen(lenmod);
    memmove(form + len, form, strlen(form) + 1);
    memcpy(form, lenmod, len);
}

int main() {
    char form[50] = "g";
    addlenmod(form, LUA_NUMBER_FRMLEN);
    printf("form = %s\n", form);
    return 0;
}
