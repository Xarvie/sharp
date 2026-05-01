#include "test/lua-5.4.6/src/lobject.h"

void test_setobj(lua_State *L, TValue *obj1, const TValue *obj2) {
    setobj(L, obj1, obj2);
}
