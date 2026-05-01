#define ltable_c
#define LUA_CORE

#include "lprefix.h"
#include <math.h>
#include <limits.h>
#include "lua.h"
#include "ldebug.h"
#include "ldo.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "lvm.h"

void test_direct_setobj(lua_State *L, TValue *a, TValue *b) {
    setobj(L, a, b);
}
