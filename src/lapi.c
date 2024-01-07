#include "lapi.h"
#include "text.h"
#include "render.h"

#define MAKE_METAMETHODS(lapi_name, api_name, type_name) \
static int lapi_##lapi_name##_gc(lua_State* L) { \
    type_name* ptr = lua_touserdata(L, 1); \
    api_name##_deinit(ptr); \
    return 0; \
} \
static int lapi_##lapi_name##_index(lua_State* L) { \
    luaL_getmetatable(L, "codim." #lapi_name); \
    lua_pushvalue(L, 2); \
    lua_gettable(L, -2); \
    return 1; \
}

#define DEFINE_METAMETHODS(lapi_name) \
    {"__gc",    lapi_##lapi_name##_gc}, \
    {"__index", lapi_##lapi_name##_index}


static void lapi_make_api_table(lua_State* L, luaL_Reg* api, int idx) {
	for (int i = 0; api[i].func; i++) {
		lua_pushcfunction(L, api[i].func);
		lua_setfield(L, idx, api[i].name);
	}
}

int lapi_renderer_new(lua_State* L) {
    RenderContext* rc = lua_newuserdata(L, sizeof(RenderContext));
    luaL_getmetatable(L, "codim.renderer");
    lua_setmetatable(L, -2);

    render_init(rc);
    return 1;
}
static int lapi_renderer_add(lua_State* L) {
    RenderContext*  rc = lua_touserdata(L, 1);
    RenderDrawable* rd = lua_touserdata(L, 2);
    render_add(rc, *rd);
    return 0;
}
static int lapi_renderer_render(lua_State* L) {
    RenderContext* rc = luaL_checkudata(L, 1, "codim.renderer");
    int width  = luaL_checkint(L, 2);
    int height = luaL_checkint(L, 3);

    render(rc, width, height);

    return 0;
}
MAKE_METAMETHODS(renderer, render, RenderContext)

int lapi_text_new(lua_State* L) {
    int font_size = lua_tointeger(L, 1);
    const char* font_path = lua_tostring(L, 2);

    TextContext* tc = lua_newuserdata(L, sizeof(TextContext));
    luaL_getmetatable(L, "codim.text");
    lua_setmetatable(L, -2);

    text_init(tc, font_path, font_size);
    return 1;
}
static int lapi_text_render(lua_State* L) {
    TextContext* tc = lua_touserdata(L, 1);
    const char* s = lua_tostring(L, 2);
    float x = lua_tonumber(L, 3);
    float y = lua_tonumber(L, 4);

    RenderDrawable* rd = lua_newuserdata(L, sizeof(RenderDrawable));
    text_render(tc, s, x, y, rd);

    return 1;
}
MAKE_METAMETHODS(text, text, TextContext);

luaL_Reg renderer_api[] = {
    DEFINE_METAMETHODS(renderer),
    {"add",     lapi_renderer_add},
    {"render",  lapi_renderer_render},
    {0}
};
luaL_Reg text_api[] = {
    DEFINE_METAMETHODS(text),
    {"render", lapi_text_render},
    {0}
};

void lapi_add_bindings(lua_State* L) {
    luaL_newmetatable(L, "codim.renderer");
    lapi_make_api_table(L, renderer_api, lua_gettop(L));
    luaL_newmetatable(L, "codim.text");
    lapi_make_api_table(L, text_api, lua_gettop(L));
}
