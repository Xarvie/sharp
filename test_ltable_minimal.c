#define ltable_c
#define LUA_CORE

#include "lua.h"
#include "ldebug.h"
#include "ldo.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"

void test(lua_State *L, TValue *a, TValue *b) {
    setobj(L, a, b);
}
