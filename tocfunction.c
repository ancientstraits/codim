#include <stdio.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int what(lua_State* L) {
}

int main() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    luaL_dofile(L, "thing.lua");
    lua_getfield(L, -1, "myfn");
    int id = lua_gettop(L);
    printf("%d\n", id);
    lua_pcall(L, 0, 0, 0);

    lua_close(L);
}
