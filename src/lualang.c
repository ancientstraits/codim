#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "video.h"
#include "drawing.h"

#include "lualang.h"

static int conv(const char* str) {
    if (str[0] != '#')
        return 0;
    switch (strlen(str)) {
    case 7:
        return strtol(&str[1], NULL, 16);
    case 4:
        return strtol(
            (char[]){str[1], str[1], str[2], str[2], str[3], str[3]},
            NULL, 16);
    default:
        return 0;
    }
}

static int lua_get_table_int(lua_State* L, const char* str) {
	lua_getfield(L, 1, str);
	if (lua_isinteger(L, -1))
		return lua_tointeger(L, -1);
	else
		return -1;
}
static int lua_get_table_int_else(lua_State* L, const char* str, int or) {
	lua_getfield(L, 1, str);
	if (lua_isinteger(L, -1))
		return lua_tointeger(L, -1);
	else
		return or;
}
static const char* lua_get_table_str(lua_State* L, const char* str) {
	lua_getfield(L, 1, str);
	if (lua_isstring(L, -1))
		return lua_tostring(L, -1);
	else
		return "";
}
static const char* lua_get_table_str_else(lua_State* L, const char* str, const char* or) {
	lua_getfield(L, 1, str);
	if (lua_isstring(L, -1))
		return lua_tostring(L, -1);
	else
		return or;
}

typedef struct {
	int width;
	int height;
} VideoOpts;


VideoContext* vc = NULL;
static int codim_set_video_opts(lua_State* L) {
	if (!lua_istable(L, -1))
		return 0;
	vc = video_context_create(
			lua_get_table_str_else(L, "output", "out.mp4"), 
			lua_get_table_int_else(L, "width",  1920),
			lua_get_table_int_else(L, "height", 1080),
			lua_get_table_int_else(L, "fps",    24),
			0);
	fill_frame(vc->frame, 0x000000);
	return 0;
}

static int codim_fill_frame(lua_State* L) {
	if (!vc)
		return 0;	
	int color = conv(luaL_checkstring(L, 1));
	fill_frame(vc->frame, color);
	return 0;
}

static int codim_draw_rect(lua_State* L) {
	if (!vc)
		return 0;
	draw_box(vc->frame,
		&(Rect){
			.x      = lua_get_table_int(L, "x"),
			.y      = lua_get_table_int(L, "y"),
			.width  = lua_get_table_int(L, "width"),
			.height = lua_get_table_int(L, "height"),
		},
		conv(lua_get_table_str_else(L, "color", "#fff")));
	return 0;
}

static int codim_wait(lua_State* L) {
	if (!vc)
		return 0;
	int lim = luaL_checkinteger(L, 1);
	for (int i = 0; i < lim; i++)
		video_context_write_frame(vc);
	return 0;
}

static const struct luaL_Reg codim_lib[] = {
	{"set_video_opts", codim_set_video_opts},
	{"fill_frame", codim_fill_frame},
	{"draw_rect", codim_draw_rect},
	{"wait", codim_wait},
	{NULL, NULL},
};

static int codim_lib_preload(lua_State* L) {
	lua_newtable(L);
	for (int i = 0; codim_lib[i].name && codim_lib[i].func; i++) {
		lua_pushstring(L, codim_lib[i].name);
		lua_pushcfunction(L, codim_lib[i].func);
		lua_settable(L, -3);
	}
	return 1;
}

int eval_lua_script(const char* filename, const char* lib_name) {
	lua_State* L = luaL_newstate();
	if (!L) {
		fprintf(stderr, "Could not create Lua State\n");
		return 1;
	}
	luaL_openlibs(L);

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	lua_pushcfunction(L, codim_lib_preload);
	lua_setfield(L, -2, lib_name);

	luaL_loadfile(L, filename);
	lua_pcall(L, 0, LUA_MULTRET, 0);
	if (vc)
		video_context_save_and_delete(vc);

	lua_close(L);
	return 0;
}



