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
	OP_PROCEDURAL_START,
	OP_PROCEDURAL_STOP,
} Opcode;

typedef struct {
	double r, g, b;
} IChangeBg;

typedef struct {
	size_t id;
} IProcedural;

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
	IProcedural* callbacks;
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
		case OP_PROCEDURAL_START: printf("[%d]: Start Procedural\n", i);
		case OP_PROCEDURAL_STOP: printf("[%d]: Stop Procedural\n", i);
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
	case OP_PROCEDURAL_START: {
		arrpush(state.callbacks, *(IProcedural*)(state.set[0].data));
		break;
	}
	case OP_PROCEDURAL_STOP: {
		int id = ((IProcedural*)(state.set[0].data))->id;
		for (int i = arrlen(state.callbacks) - 1; i >= 0; i--) {
			if (state.callbacks[i].id == id)
				arrdel(state.callbacks, i);
		}
		break;
	}
	default: break;
	}
	if (state.set[0].op != OP_PROCEDURAL_START) {
		free(state.set[0].data);
	}
	arrdel(state.set, 0);
}

static void cb_call(lua_State* L, int frameno) {
	lua_getfield(L, LUA_REGISTRYINDEX, "codim_procedural_callbacks");
	int callbacks = lua_gettop(L);

	int i;
	for (i = 0; i < arrlen(state.callbacks); i++) {
		lua_pushinteger(L, state.callbacks[i].id);
		lua_gettable(L, callbacks); // push callbacks[i] to top of stack
	}
	lua_pushnumber(L, (double)frameno / state.vo.fps);
	for (; i > 0; i--) {
		lua_pcall(L, 1, 0, 0);
		lua_remove(L, -2);
	}
}

static void make(lua_State* L) {
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

			cb_call(L, frameno);

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
	if (top == 3) {
		float
			r = lua_tonumber(L, 1),
			g = lua_tonumber(L, 2),
			b = lua_tonumber(L, 3);
		glClearColor(r, g, b, 1.0);
	} else if (top == 4) {
		float
			r = lua_tonumber(L, 2),
			g = lua_tonumber(L, 3),
			b = lua_tonumber(L, 4);
		IChangeBg* bg = malloc(sizeof *bg);
		bg->r = r;
		bg->g = g;
		bg->b = b;

		int time = lua_tonumber(L, 1);
		instruction_insert((Instruction){
			.op = OP_CHANGE_BG,
			.frameno = (int)(time * state.vo.fps),
			.data = bg,
		});
	} else {
		luaL_error(L, "must have 3 or 4 arguments");
	}
	return 0;
}

static int api_procedural(lua_State* L) {
	double start = lua_tonumber(L, 1);
	double stop  = lua_tonumber(L, 2);
	// parameter 3 is callback
	lua_getfield(L, LUA_REGISTRYINDEX, "codim_procedural_callbacks");
	size_t id = lua_objlen(L, -1) + 1;
	lua_pushinteger(L, id);
	lua_pushvalue(L, 3); // push callback
	lua_settable(L, -3); // cbs[#cbs+1]=(3rd parameter) (cbs=registry.codim_procedural_callbacks)
	lua_pop(L, 1); // remove registry.codim_procedural_callbacks from stack

	IProcedural* proc = malloc(sizeof *proc);
	proc->id = id;
	instruction_insert((Instruction) {
		.op = OP_PROCEDURAL_START,
		.frameno = (int)(start * state.vo.fps),
		.data = proc,
	});
	instruction_insert((Instruction) {
		.op = OP_PROCEDURAL_STOP,
		.frameno = (int)(stop * state.vo.fps),
		.data = proc,
	});

	return 0;
}

luaL_Reg api[] = {
	{"output", api_output},
	{"procedural", api_procedural},
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

static void done() {
	arrfree(state.set);
}

void scripting_exec(const char* scriptname) {
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	// for `local cm = require('codim')
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");
	lua_pushcfunction(L, preload_codim);
	lua_setfield(L, -2, "codim");

	// table of callbacks in registry
	lua_pushstring(L, "codim_procedural_callbacks");
	lua_createtable(L, 0, 0);
	lua_settable(L, LUA_REGISTRYINDEX);

	// Lua functions return 0 on success
	SASSERT(!luaL_loadfile(L, scriptname), "Could not load file %s", scriptname);
	SASSERT(!lua_pcall(L, 0, 0, 0), "Error running script: %s", lua_tostring(L, -1));

	make(L);
	lua_close(L);
	done();
}




