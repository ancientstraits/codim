#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "text.h"
#include "video.h"
#include "drawing.h"
#include "fcutil.h"
#include "lualang.h"
#include "c_lex.h"

#define FONT_STR_MAX 128

int current_color = 0;

static int hexstrtoi(const char *str) {
	if (str[0] != '#')
		return 0;
	switch (strlen(str)) {
	case 7:
		return strtol(&str[1], NULL, 16);
	case 4:
		return strtol((char[]){str[1], str[1], str[2], str[2], str[3], str[3]},
					  NULL, 16);
	default:
		return 0;
	}
}

#define lua_get_table_bool(L, str) lua_get_table_bool_else(L, str, 0)
static int lua_get_table_bool_else(lua_State *L, const char *str, int or) {
	lua_getfield(L, 1, str);
	if (lua_isboolean(L, -1)) {
		int ret = lua_toboolean(L, -1);
		// lua_pop(L, -1);
		return ret;
	} else
		return or ;
}

#define lua_get_table_int(L, str) lua_get_table_int_else(L, str, 0)
static int lua_get_table_int_else(lua_State *L, const char *str, int or) {
	lua_getfield(L, 1, str);
	if (lua_isinteger(L, -1)) {
		int ret = lua_tointeger(L, -1);
		// lua_pop(L, -1);
		return ret;
	} else
		return or ;
}

#define lua_get_table_str(L, str) lua_get_table_str_else(L, str, "")
static char *lua_get_table_str_else(lua_State *L, const char *str,
									const char * or) {
	lua_getfield(L, 1, str);
	if (lua_isstring(L, -1)) {
		char *ret = strdup(lua_tostring(L, -1));
		// lua_pop(L, -1);
		return ret;
	} else
		return strdup(or);
}

VideoContext *vc = NULL;
static int codim_set_video_opts(lua_State *L) {
	if (!lua_istable(L, -1))
		return 0;
	vc = video_context_create(lua_get_table_str_else(L, "output", "out.mp4"),
							  lua_get_table_int_else(L, "width", 1920),
							  lua_get_table_int_else(L, "height", 1080),
							  lua_get_table_int_else(L, "fps", 24), 0);
	fill_frame(vc->frame, 0x000000);
	return 0;
}

static int codim_fill_frame(lua_State *L) {
	if (!vc)
		return 0;
	int color = hexstrtoi(luaL_checkstring(L, 1));
	fill_frame(vc->frame, color);
	current_color = color;
	return 0;
}

static int codim_draw_rect(lua_State *L) {
	if (!vc)
		return 0;

	char *color_s = lua_get_table_str_else(L, "color", "#fff");
	int color = hexstrtoi(color_s);
	free(color_s);

	draw_box(vc->frame,
			 &(Rect){
				 .x = lua_get_table_int(L, "x"),
				 .y = lua_get_table_int(L, "y"),
				 .width = lua_get_table_int(L, "width"),
				 .height = lua_get_table_int(L, "height"),
			 },
			 color);
	return 0;
}

static int codim_wait(lua_State *L) {
	if (!vc)
		return 0;
	int lim = luaL_checkinteger(L, 1);
	for (int i = 0; i < lim; i++)
		video_context_write_frame(vc);
	return 0;
}

int codim_font_mono(lua_State *L) {
	char font_name[FONT_STR_MAX];
	findMonospaceFont(font_name);
	lua_pushstring(L, font_name);
	return 1;
}

int c_lex_to_color[] = {
	[C_LEX_NONE] = 0xffffff, [C_LEX_DELIM] = 0xff0000,
	[C_LEX_STR] = 0x00ff00,	 [C_LEX_OPERATOR] = 0x0000ff,
	[C_LEX_VAR] = 0xffff00,	 [C_LEX_TYPE] = 0xff00ff,
	[C_LEX_NUM] = 0x00ffff,
};

int codim_draw_text(lua_State *L) {
	if (!vc)
		return 0;
	// TODO does this actually work?
	char *font_file = lua_get_table_str(L, "font_file");
	if (!font_file) {
		char buf[FONT_STR_MAX];
		findMonospaceFont(buf);
		font_file = buf;
	}

	TextContext *tc = text_context_init(
		font_file, lua_get_table_int_else(L, "font_size", 20));

	char *color_s = lua_get_table_str(L, "color");
	int color = hexstrtoi(color_s);
	free(color_s);

	if (lua_get_table_bool(L, "animated")) {
		char *str = lua_get_table_str(L, "text");

		const int x = lua_get_table_int(L, "x");
		const int y =
			lua_get_table_int(L, "y") + tc->newline;

		const int speed = lua_get_table_int_else(L, "animation_speed", 2);

		tc->loc.x = x;
		tc->loc.y = y;
		for (int i = 0; str[i]; i++) {
			draw_single_char(tc, vc->frame, str[i], x, y,
							 c_lex_to_color[c_lex(str[i])], current_color);
			for (int j = 0; j < speed; j++)
				video_context_write_frame(vc);
		}
		free(str);
	} else {
		draw_text(tc, vc->frame, lua_get_table_str(L, "text"),
				  lua_get_table_int(L, "x"), lua_get_table_int(L, "y"), color,
				  current_color);
	}
	free(font_file);
	text_context_delete(tc);
	return 0;
}

static const struct luaL_Reg codim_lib[] = {
	{"set_video_opts", codim_set_video_opts},
	{"fill_frame", codim_fill_frame},
	{"draw_rect", codim_draw_rect},
	{"font_mono", codim_font_mono},
	{"draw_text", codim_draw_text},
	{"wait", codim_wait},
	{NULL, NULL},
};

static int codim_lib_preload(lua_State *L) {
	lua_newtable(L);
	for (int i = 0; codim_lib[i].name && codim_lib[i].func; i++) {
		lua_pushstring(L, codim_lib[i].name);
		lua_pushcfunction(L, codim_lib[i].func);
		lua_settable(L, -3);
	}
	return 1;
}

int eval_lua_script(const char *filename, const char *lib_name) {
	lua_State *L = luaL_newstate();
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
	int x = lua_pcall(L, 0, LUA_MULTRET, 0);
	switch (x) {
	case 0: break;
	default: {
		lua_Debug ar;
		lua_getstack(L, 1, &ar);
		lua_getinfo(L, "nSl", &ar);
		int line = ar.currentline;
		fprintf(stderr, "Failed to execute lua: \n%s:%d: %s\n",
			filename, line, lua_tostring(L, -1));
		exit(1);
		}
	}
	if (vc)
		video_context_save_and_delete(vc);

	lua_close(L);
	return 1;
}
