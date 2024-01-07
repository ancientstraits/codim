#ifndef BINDINGS_H
#define BINDINGS_H

// Lua API bindings.
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

// constructors
int lapi_renderer_new(lua_State* L);
int lapi_text_new(lua_State* L);

void lapi_add_bindings(lua_State* L);

#endif  // !BINDINGS_H
