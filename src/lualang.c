#include <stdio.h>

#include "lauxlib.h"
#include "lua.h"
#include "lualang.h"
#include "lualib.h"

void lua_load_file(const char* file) {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    luaL_loadfile(L, file);
    lua_pcall(L, 0, LUA_MULTRET, 0);
    lua_getglobal(L, "Config");
    size_t size;
    const char* str = lua_tolstring(L, -1, &size);
    printf("%s\n", str);
    lua_close(L);
}

