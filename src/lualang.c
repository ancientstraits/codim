#include <lauxlib.h>
#include <lua.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lauxlib.h"
#include "lua.h"
#include "lualang.h"
#include "lualib.h"

static int hexcode_to_int(char* code) {
    if (code[0] != '#' || strlen(code) != 7) {
        fprintf(stderr, "Not a valid hex code\n");
        return 0;
    }
    return strtol(&code[1], NULL, 16);
}

static const struct luaL_Reg codim[] = {
    {NULL, NULL}
};



void llang_load_and_exec(const char* script_path) {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    luaL_loadfile(L, script_path);
    lua_pcall(L, 0, LUA_MULTRET, 0);

    lua_close(L);
}
