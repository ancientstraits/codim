#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "scripting.h"
#include "output.h"
#include "glutil.h"
#include "gfx.h"
#include "text.h"
#include "render.h"
#include "error.h"
#define SASSERT(cond, ...) ASSERT(cond, ERROR_SCRIPTING, __VA_ARGS__)

#define GET_ELEM(L, idx, type, name) (lua_getfield(L, idx, name), lua_to##type(L, -1))

typedef enum {
	OP_CHANGE_BG,
} Opcode;

typedef struct {
	double r, g, b;
} IChangeBg;

typedef struct {
	Opcode op;
	size_t frameno;
	void* data;
} Instruction;

struct {
	char filename[128];
	OutputAudioOpts ao;
	OutputVideoOpts vo;
	OutputContext* oc;
	GfxContext* gc;
	RenderContext* rc;
	TextContext* tc;
	Instruction* set;
} state = {0};

static void instruction_insert(Instruction in) {
	int i;
	for (i = arrlen(state.set); i > 0; i--) {
		if (in.frameno >= state.set[i - 1].frameno) break;
	}
	arrins(state.set, i, in);
}

static void print_instructions() {
	printf("Length: %d\n", arrlen(state.set));
	for (int i = 0; i < arrlen(state.set); i++) {
		switch (state.set[i].op) {
		case OP_CHANGE_BG: {
			printf("[%d]: Change Background\n", i);
			IChangeBg* bg = state.set[i].data;
			printf("\t(%f, %f, %f)\n", bg->r, bg->g, bg->b);
			break;
		}
		default:
			printf("[%d]: Other Instruction (Op: %u)\n", i, state.set[i].op);
			break;
		}
	}
}

static void instruction_handle() {
	switch (state.set[0].op) {
	case OP_CHANGE_BG: {
		IChangeBg* bg = state.set[0].data;
		LOG("Color Changed: (%f, %f, %f)", bg->r, bg->g, bg->b);
		glClearColor(bg->r, bg->g, bg->b, 1.0);
		break;
	}
	default: break;
	}

	arrdel(state.set, 0);
}

static void make() {
	state.oc = output_create(state.filename, &state.ao, &state.vo);
	output_open(state.oc);
	state.gc = gfx_create(state.vo.width, state.vo.height);

	int frameno = 0;
	float at = 0;
	float atincr = 2 * M_PI * 110.0 / state.oc->acc->sample_rate;
	float atincr2 = atincr / state.oc->acc->sample_rate;

	print_instructions();

	while (output_is_open(state.oc)) {
		if (output_get_seconds(state.oc) >= 10.0)
			output_close(state.oc);

		if (output_get_encode_type(state.oc) == OUTPUT_TYPE_VIDEO) {
			if (arrlen(state.set) > 0 && frameno >= state.set[0].frameno) {
				instruction_handle();
			}

			glClear(GL_COLOR_BUFFER_BIT);
			
			gfx_render(state.gc, state.oc->vf);
			output_encode_video(state.oc);

			frameno++;
		} else {
			int16_t *data = (int16_t *)state.oc->afd->data[0];
			for (int i = 0; i < state.oc->afd->nb_samples; i++) {
				int v = 10000 * sin(at);
				for (int j = 0; j < state.oc->acc->ch_layout.nb_channels; j++)
					*data++ = v;
				at += atincr;
				atincr += atincr2;
			}
			output_encode_audio(state.oc);
		}

	}

	gfx_destroy(state.gc);
	output_destroy(state.oc);
}

static int api_output(lua_State* L) {
	int top = lua_gettop(L);
	int filename = top - 1, opts = top;
	strncpy(state.filename, lua_tostring(L, filename), 128);
	state.ao.sample_rate = GET_ELEM(L, opts, integer, "sample_rate");
	state.vo.width = GET_ELEM(L, opts, integer, "width");
	state.vo.height = GET_ELEM(L, opts, integer, "height");
	state.vo.fps = GET_ELEM(L, opts, integer, "fps");
	return 0;
}

static int api_changebg(lua_State* L) {
	int top = lua_gettop(L);
	int time = top - 3, r = top - 2, g = top - 1, b = top;
	IChangeBg* bg = malloc(sizeof *bg); // TODO write code to free this
	bg->r = lua_tonumber(L, r);
	bg->g = lua_tonumber(L, g);
	bg->b = lua_tonumber(L, b);
	instruction_insert((Instruction){
		.op = OP_CHANGE_BG,
		.frameno = (int)(lua_tonumber(L, time) * state.vo.fps),
		.data = bg,
	});
	return 0;
}

luaL_Reg api[] = {
	{"output", api_output},
	{"changebg", api_changebg},
	{0},
};

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

void scripting_exec(const char* scriptname) {
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	lua_pushcfunction(L, preload_codim);
	lua_setfield(L, -2, "codim");

	// Lua functions return 0 on success
	SASSERT(!luaL_loadfile(L, scriptname), "Could not load file %s", scriptname);
	SASSERT(!lua_pcall(L, 0, 0, 0), "Error running script: %s", lua_tostring(L, -1));

	lua_close(L);

	make();
}




