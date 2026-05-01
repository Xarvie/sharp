#define LUA_FLOAT_TYPE 3
#define LUA_FLOAT_FLOAT 3

#if LUA_FLOAT_TYPE == LUA_FLOAT_FLOAT
#define LUA_NUMBER float
#define LUA_NUMBER_FRMLEN ""
#else
#define LUA_NUMBER_FRMLEN "ll"
#endif

void addlenmod(char *form, const char *lenmod);
void test(char *form) {
    addlenmod(form, LUA_NUMBER_FRMLEN);
}
