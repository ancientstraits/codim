#include <stdio.h>
#include <epoxy/gl.h>

#include "obj/luagen.h"
#include "arecord.h"
#include "output.h"
#include "gfx.h"
#include "lapi.h"
#include "scripting.h"
#include "error.h"
#define SASSERT(cond, ...) ASSERT(cond, ERROR_SCRIPTING, __VA_ARGS__)

#define GET_ELEM(L, idx, type, name) (lua_getfield(L, idx, name), lua_to##type(L, -1))
#define GET_CALLBACK(L, idx, name)   (lua_getfield(L, idx, name), lua_gettop(L))

// typedef struct ApiVideoInfo {
//     int set; // Whether the info has been set yet
//     const char* output;
//     // int sample_rate;
//     int width, height, fps, sample_rate;
// } ApiVideoInfo;
// struct {
//     int stop_rendering;
// 	OutputContext* oc;
// 	GfxContext* gc;
//     ARecordContext* arc;
//     ApiVideoInfo info;
// 	// RenderContext* rc;
// 	// TextContext* tc;
// } state = {0};

ScriptingState scripting_state;

static int api_arecord(lua_State* L) {
    const char* prompt = lua_isnil(L, 1) ? NULL : lua_tostring(L, 1);
    arecord_prompt_and_record(scripting_state.arc, prompt);
    ARecordUserData udata = arecord_get_data_copy(scripting_state.arc);

    // TODO convert this to userdata to make it faster
    lua_createtable(L, 2*udata.idx, 0);
    for (int i = 0; i < 2*udata.idx; i++) {
        lua_pushinteger(L, udata.samples[i]);
        lua_rawseti(L, -2, i+1);
    }

    free(udata.samples);
    return 1;
}

static int api_output(lua_State* L) {
    scripting_state.info.set    = 1;
    scripting_state.info.output = GET_ELEM(L, 1, string,  "file");
    scripting_state.info.width  = GET_ELEM(L, 1, integer, "width");
    scripting_state.info.height = GET_ELEM(L, 1, integer, "height");
    scripting_state.info.fps    = GET_ELEM(L, 1, integer, "fps");
    scripting_state.info.sample_rate = GET_ELEM(L, 1, integer, "sample_rate");

    scripting_state.oc = output_create(scripting_state.info.output,
        &(OutputAudioOpts){
            // .sample_rate = scripting_state.info.sample_rate,
            .sample_rate = 44100,
        }, &(OutputVideoOpts){
            .width  = scripting_state.info.width,
            .height = scripting_state.info.height,
            .fps    = scripting_state.info.fps,
        }
    );
    scripting_state.gc = gfx_create(scripting_state.info.width, scripting_state.info.height);
    scripting_state.arc = arecord_create(scripting_state.info.sample_rate);

    return 0;
}
static int api_clear(lua_State* L) {
    GLfloat
        r = lua_tonumber(L, 1),
        g = lua_tonumber(L, 2),
        b = lua_tonumber(L, 3)
    ;
    // printf("clear %f, %f, %f\n", r, g, b);
    glClearColor(r, g, b, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    return 0;
}
static int api_stop(lua_State* L) {
    scripting_state.stop_rendering = 1;
    return 0;
}
static int api_getmetatable(lua_State* L) {
    const char* key = luaL_checkstring(L, 1);
    luaL_getmetatable(L, key);
    return 1;
}

luaL_Reg api[] = {
	{"clear",    api_clear},
	{"stop",     api_stop},
    {"output",   api_output},
    {"arecord",  api_arecord},
    {"getmetatable", api_getmetatable},
	{0},
};
static int preload_codim(lua_State* L) {
	lua_newtable(L);
	int tbl = lua_gettop(L);

	for (int i = 0; api[i].func; i++) {
		lua_pushcfunction(L, api[i].func);
		lua_setfield(L, tbl, api[i].name);
	}
	for (int i = 0; lapi_api[i].func; i++) {
		lua_pushcfunction(L, lapi_api[i].func);
		lua_setfield(L, tbl, lapi_api[i].name);
	}

	lua_pushvalue(L, tbl);
	return 1;
}

static void print_stack(lua_State* L) {
    int top = lua_gettop(L);
    printf("%d entries in stack\n", top);
    for (int i = top; i >= 1; i--) {
        printf("\t%d (%d) type: %s\n", i, -(top-i+1), luaL_typename(L, i));
        if (lua_isnumber(L, i))
            printf("\t\t%lf\n", lua_tonumber(L, i));
        else if (lua_isstring(L, i))
            printf("\t\t%s\n", lua_tostring(L, i));
    }
}

static void make_video(lua_State* L, int audio_cb, int video_cb) {
// static void make_video(lua_State* L, int video_cb) {
    if (!scripting_state.info.set)
        luaL_error(L, "You need to call `codim.output()`!");

    // RenderContext rc;
    // render_init(&rc);
    // RenderDrawable rd;
    // TextContext tc;
    // text_init(&tc, "sample.ttf", 100);
    // text_render(&tc, "Oliopolig", 60, 60, &rd);
    // render_add(&rc, rd);

    size_t vframeno = 0, aframeno = 0;
    printf("%d\n", scripting_state.oc->acc->ch_layout.nb_channels);
    output_open(scripting_state.oc);
    while (output_is_open(scripting_state.oc)) {
        // TODO I feel like it should break if rendering is over. That would feel more intuitive.
        if (scripting_state.stop_rendering)
            output_close(scripting_state.oc);

        if (output_get_encode_type(scripting_state.oc) == OUTPUT_TYPE_VIDEO) {
            lua_pushvalue(L, video_cb); // video callback
            lua_pushnumber(L, ((double)vframeno/(double)scripting_state.info.fps)); // argument `t`
            // print_stack(L);
            SASSERT(!lua_pcall(L, 1, 0, 0), "Error running video(): %s", lua_tostring(L, -1));

            // glClear(GL_COLOR_BUFFER_BIT);
            // render(scripting_state.rc, sr->width, sr->height);
            gfx_render(scripting_state.gc, scripting_state.oc->vf);
            output_encode_video(scripting_state.oc);
            vframeno++;
        } else if (output_get_encode_type(scripting_state.oc) == OUTPUT_TYPE_AUDIO) {
            int16_t* data = (int16_t*)scripting_state.oc->afd->data[0];
            for (int i = 0; i < scripting_state.oc->afd->nb_samples; i++) {
                lua_pushvalue(L, audio_cb);
                lua_pushnumber(L, ((double)aframeno/(double)scripting_state.info.sample_rate));
                SASSERT(!lua_pcall(L, 1, 2, 0), "Error running audio(): %s", lua_tostring(L, -1));

                int16_t left  = lua_tointeger(L, -1);
                int16_t right = lua_tointeger(L, -2);
                *data++ = left;
                *data++ = right;

                // lua_pop(L, 1);
                aframeno++;
            }
            lua_pop(L, 2*scripting_state.oc->afd->nb_samples);

            output_encode_audio(scripting_state.oc);
        }
    }

    lua_close(L);

    // output_close(scripting_state.oc);
    
    // render_destroy(scripting_state.rc);
    gfx_destroy(scripting_state.gc);
    output_destroy(scripting_state.oc);
    arecord_destroy(scripting_state.arc);
}

void add_luagen_bindings(lua_State* L, int preload) {
    for (int i = 0; luagen_entries[i].name; i++) {
        luaL_loadbuffer(L, luagen_entries[i].code, luagen_entries[i].size, luagen_entries[i].name);
        lua_setfield(L, preload, luagen_entries[i].name);
    }
}

void scripting_exec(const char* scriptname) {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    // for `require('codim')`
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	lua_pushcfunction(L, preload_codim);
	lua_setfield(L, -2, "codim.internal");
    add_luagen_bindings(L, lua_gettop(L));

    lapi_add_bindings(L);

    SASSERT(!luaL_loadfile(L, scriptname), "Could not load script %s due to %s", scriptname, lua_tostring(L, -1));
    SASSERT(!lua_pcall(L, 0, 1, 0), "Error running script: %s", lua_tostring(L, -1));

    lua_getfield(L, -1, "video");
    int video_cb = lua_gettop(L);

    lua_getfield(L, -2, "audio");
    int audio_cb = lua_gettop(L);

    make_video(L, audio_cb, video_cb);
    // make_video(L, video_cb);

    // lua_close(L);
}
