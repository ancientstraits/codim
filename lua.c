#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

struct {
	char out[128];
} state = {0};

static int output(lua_State* L);
luaL_Reg api[] = {
	{"output", output},
	{0},
};

static int output(lua_State* L) {
	strcpy(state.out, lua_tostring(L, -1));
	return 0;
}

static int preload_codim(lua_State* L) {
	lua_newtable(L);
	int tbl = lua_gettop(L);

	for (int i = 0; api[i].func; i++) {
		lua_pushcfunction(L, api[i].func);
		lua_setfield(L, tbl, api[i].name);
	}

	lua_pushvalue(L, tbl);
	return 1;
}

int main(int argc, char** argv) {
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	lua_pushcfunction(L, preload_codim);
	lua_setfield(L, -2, "codim");

	// Lua functions return 0 on success
	assert(!luaL_loadfile(L, "test.lua"));
	if (lua_pcall(L, 0, 0, 0)) {
		printf("%s\n", lua_tostring(L, -1));
		exit(1);
	}

	lua_close(L);

	printf("Output: %s\n", state.out);
}

