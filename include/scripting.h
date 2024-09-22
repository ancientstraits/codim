#ifndef SCRIPTING_H
#define SCRIPTING_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "output.h"
#include "gfx.h"
#include "arecord.h"

typedef struct ScriptingVideoInfo {
    int set; // Whether the info has been set yet
    const char* output;
    // int sample_rate;
    int width, height, fps, sample_rate;
} ScriptingVideoInfo;

typedef struct ScriptingState {
    int stop_rendering;
	OutputContext* oc;
	GfxContext* gc;
    ARecordContext* arc;
    ScriptingVideoInfo info;
	// RenderContext* rc;
	// TextContext* tc;
} ScriptingState;
extern ScriptingState scripting_state;

void scripting_exec(const char* scriptname);

#endif // !SCRIPTING_H

