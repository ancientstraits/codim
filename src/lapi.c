#include "lapi.h"
#include "text.h"
#include "render.h"
#include "editor.h"
#include "scripting.h"

#define MAKE_METAMETHODS(lapi_name, api_name, type_name) \
static int lapi_##lapi_name##_gc(lua_State* L) { \
    puts("Destructing " #type_name); \
    type_name* ptr = lua_touserdata(L, 1); \
    api_name##_deinit(ptr); \
    return 0; \
} \
// static int lapi_##lapi_name##_index(lua_State* L) { \
//     luaL_getmetatable(L, "codim." #lapi_name); \
//     lua_pushvalue(L, 2); \
//     lua_gettable(L, -2); \
//     return 1; \
// }

#define DEFINE_METAMETHODS(lapi_name) \
    {"__gc",    lapi_##lapi_name##_gc}
    // {"__gc",    lapi_##lapi_name##_gc},
    // {"__index", lapi_##lapi_name##_index}


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
    RenderContext* rc = lua_touserdata(L, 1);
    // printf("%p\n", luaL_testudata(L, 1, "codim.renderer"));

    RenderDrawable* rd;
    if (luaL_testudata(L, 2, "codim.editor")) {
        printf("ok\n");
        EditorContext* ec = lua_touserdata(L, 2);

        rd = &ec->rd;
    } else {
        printf("smth else\n");
        rd = lua_touserdata(L, 2);
    }

    render_add(rc, rd);
    return 0;
}
static int lapi_renderer_del(lua_State* L) {
    RenderContext* rc = lua_touserdata(L, 1);
    RenderDrawable* rd = lua_touserdata(L, 2);
    render_del(rc, rd);
    return 0;
}
static int lapi_renderer_render(lua_State* L) {
    RenderContext* rc = lua_touserdata(L, 1);
    // int width  = luaL_checkint(L, 2);
    // int height = luaL_checkint(L, 3);

    render(rc, scripting_state.info.width, scripting_state.info.height);

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
    luaL_setmetatable(L, "codim.renderdrawable");
    *rd = text_render(tc, s, x, y);

    return 1;
}
// static int lapi_text_draw_px(lua_State* L) {
//     TextContext* tc = lua_touserdata(L, 1);
//     const char* s = lua_tostring(L, 2);
//     float x = lua_tonumber(L, 3);
//     float y = lua_tonumber(L, 4);



//     return 0;
// }
MAKE_METAMETHODS(text, text, TextContext);

int lapi_editor_new(lua_State* L) {
    TextContext* tc = luaL_checkudata(L, 1, "codim.text");
    int x = lua_tointeger(L, 2), y = lua_tointeger(L, 3);

    EditorContext* ec = lua_newuserdata(L, sizeof(EditorContext));
    luaL_setmetatable(L, "codim.editor");

    editor_init(ec, tc, x, y, 0, 0);

    // printf("ec->rd.model = %p\n", ec->rd.model);

    // mat4x4_translate(ec->rd.model, 30.0, 20.0, 0.0);
    // mat4x4_rotate_Z(ec->rd.model, ec->rd.model, 0.3);

    // for (int i = 0; i < 4; i++) {
    //     for (int j = 0; j < 4; j++) {
    //         printf("%lf ", ec->rd.model[i][j]);
    //     }
    //     putchar('\n');
    // }

    return 1;
}
static int lapi_editor_get_rd(lua_State* L) {
    EditorContext* ec = luaL_checkudata(L, 1, "codim.editor");
    lua_pushlightuserdata(L, &ec->rd);
    luaL_setmetatable(L, "codim.renderdrawable");

    return 1;
}
static int lapi_editor_insert(lua_State* L) {
    // printf("this literally does nothing.\n");
    EditorContext* ec = lua_touserdata(L, 1);
    editor_insert(ec, lua_tostring(L, 2));
    return 0;
}

static int lapi_renderdrawable_get_model(lua_State* L) {
    RenderDrawable* rd = lua_touserdata(L, 1);
    // printf("&rd->model[0] = %p %p\n", (&rd->model)[0], &(rd->model[0]));
    lua_pushlightuserdata(L, &rd->model[0]);
    luaL_setmetatable(L, "codim.mat4x4");

    return 1;
}

static int lapi_mat4x4_translate(lua_State* L) {
    mat4x4_translate_in_place(lua_touserdata(L, 1), lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3));
    return 0;
}
static int lapi_mat4x4_identity(lua_State* L) {
    mat4x4_identity(lua_touserdata(L, 1));
    return 0;
}
static int lapi_mat4x4_new(lua_State* L) {
    vec4* mat = lua_newuserdata(L, sizeof(mat4x4));
    luaL_setmetatable(L, "codim.mat4x4");

    if (lua_gettop(L) == 16) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                mat[i][j] = lua_tonumber(L, (j*4)+i+1);
            }
        }
    } else {
        mat4x4_identity(mat);
    }

    return 1;
}
static int lapi_mat4x4_rotx(lua_State* L) {
    vec4* mat = lua_touserdata(L, 1);
    mat4x4_rotate_X(mat, mat, luaL_checknumber(L, 2));
    return 0;
}
static int lapi_mat4x4_roty(lua_State* L) {
    vec4* mat = lua_touserdata(L, 1);
    mat4x4_rotate_Y(mat, mat, luaL_checknumber(L, 2));
    return 0;
}
static int lapi_mat4x4_rotz(lua_State* L) {
    vec4* mat = lua_touserdata(L, 1);
    // printf("mat = %p\n", mat);
    mat4x4_rotate_Z(mat, mat, luaL_checknumber(L, 2));
    // mat4x4_rotate(mat, mat, 0, 0, -1, luaL_checknumber(L, 2));
    return 0;
}
static int lapi_mat4x4_rotate(lua_State* L) {
    // vec4* mat = lua_touserdata(L, 1);
    // mat4x4_rotate(mat, mat, 0, 0, 0, 0);
}
static int lapi_mat4x4_dup(lua_State* L) {
    vec4* mat = lua_touserdata(L, 1);
    vec4* ret = lua_newuserdata(L, sizeof(mat4x4));
    luaL_setmetatable(L, "codim.mat4x4");
    memcpy(ret, mat, sizeof(mat4x4));
    return 1;
}
MAKE_METAMETHODS(editor, editor, EditorContext);

static int lapi_abuffer_duration(lua_State* L) {
    lua_getfield(L, 1, "n_samples");
    double duration = lua_tonumber(L, -1) * scripting_state.info.sample_rate;
    lua_pushnumber(L, duration);

    return 1;
}
static int lapi_abuffer_next(lua_State* L) {
    lua_getfield(L, 1, "n_samples");
    size_t n_samples = lua_tointeger(L, -1);
    lua_getfield(L, 1, "idx");
    size_t idx = lua_tointeger(L, -1);

    lua_createtable(L, 2, 0);
    int ret_idx = lua_gettop(L);
    if (idx >= n_samples) {
        lua_pushnumber(L, 0);
        lua_pushnumber(L, 0);
    } else {
        lua_getfield(L, 1, "data");
        int16_t* samples = lua_touserdata(L, -1);
        lua_pop(L, 1);

        lua_pushinteger(L, idx+1);
        lua_setfield(L, 1, "idx");

        lua_pushnumber(L, samples[2*idx]);
        lua_pushnumber(L, samples[2*(idx+1)]);
    }
    lua_rawseti(L, ret_idx, 1);
    lua_rawseti(L, ret_idx, 2);

    return 1;
}

static int lapi_abuffer_over(lua_State* L) {
    lua_getfield(L, 1, "n_samples");
    size_t n_samples = lua_tointeger(L, -1);
    lua_getfield(L, 1, "idx");
    size_t idx = lua_tointeger(L, -1);

    lua_pushboolean(L, idx >= n_samples);
    return 1;
}

luaL_Reg renderer_api[] = {
    DEFINE_METAMETHODS(renderer),
    {"add",     lapi_renderer_add},
    {"del",     lapi_renderer_del},
    {"render",  lapi_renderer_render},
    {0}
};
luaL_Reg text_api[] = {
    DEFINE_METAMETHODS(text),
    {"render", lapi_text_render},
    // {"draw_px", lapi_text_draw_px},
    {0}
};
luaL_Reg editor_api[] = {
    DEFINE_METAMETHODS(editor),
    {"insert", lapi_editor_insert},
    {"get_rd", lapi_editor_get_rd},
    {0}
};
luaL_Reg mat4x4_api[] = {
    {"translate", lapi_mat4x4_translate},
    {"identity", lapi_mat4x4_identity},
    {"rotx", lapi_mat4x4_rotx},
    {"roty", lapi_mat4x4_roty},
    {"rotz", lapi_mat4x4_rotz},
    {"dup", lapi_mat4x4_dup},
    {0}
};
luaL_Reg renderdrawable_api[] = {
    {"get_model", lapi_renderdrawable_get_model},
    {0}
};
luaL_Reg abuffer_api[] = {
    {"duration", lapi_abuffer_duration},
    {"next", lapi_abuffer_next},
    {"over", lapi_abuffer_over},
    {0}
};

luaL_Reg lapi_api[] = {
    {"Renderer", lapi_renderer_new},
    {"Text",     lapi_text_new},
    {"Editor",   lapi_editor_new},
    {"Mat4x4",   lapi_mat4x4_new},
    {0}
};


#define ADD_AND_MAKE(lapi_name) (\
    luaL_newmetatable(L, "codim." #lapi_name), \
    lua_pushvalue(L, -1), \
    lua_setfield(L, -2, "__index"), \
    lapi_make_api_table(L, lapi_name##_api, lua_gettop(L)) \
    )

void lapi_add_bindings(lua_State* L) {
    ADD_AND_MAKE(renderer);
    ADD_AND_MAKE(renderdrawable);
    ADD_AND_MAKE(text);
    ADD_AND_MAKE(editor);
    ADD_AND_MAKE(mat4x4);
    ADD_AND_MAKE(abuffer);
}
