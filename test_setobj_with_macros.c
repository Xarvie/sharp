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

/* ltable.c's own macro definitions from L42-L95 */
#define MAXABITS	cast_int(sizeof(int) * 8 - 3)
#define MAXASIZE	(1 << MAXABITS)
#define MAXHBITS	(MAXASIZE - 1)
#define hashpow2(t,n)	(gnode(t, (n) & MAXHBITS))
#define hashmod(t,n)	(gnode(t, (n) % ((sizenode(t)-1)|1)))
#define hashstr(t,str)		hashpow2(t, (str)->hash)
#define hashboolean(t,p)	hashpow2(t, p)
#define hashpointer(t,p)	hashpow2(t, cast(size_t, p))
#define dummynode		(&dummynode_)
static const Node dummynode_ = {
    {{NULL}, LUA_VEMPTY, LUA_VEMPTY},
    {{NULL}, LUA_VEMPTY, LUA_VEMPTY}
};
#define hashnil(t)		hashpointer(t, NULL)

void test1(lua_State *L, Table *t, TValue *key, TValue *value) {
    setobj(L, key, value);
}
