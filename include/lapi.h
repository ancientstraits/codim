#ifndef BINDINGS_H
#define BINDINGS_H

// Lua API bindings.
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

// constructors
extern luaL_Reg lapi_api[];

void lapi_add_bindings(lua_State* L);

#endif  // !BINDINGS_H
