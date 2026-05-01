#include "lobject.h"

void test(lua_State *L, TValue *a, TValue *b) {
    setobj(L, a, b);
}
