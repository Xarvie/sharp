#define LUA_INTEGER_FRMLEN "ll"
#define LUA_NUMBER_FRMLEN ""

void foo() {
  addlenmod(form, LUA_INTEGER_FRMLEN);
  addlenmod(form, LUA_NUMBER_FRMLEN);
}
