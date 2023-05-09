#ifndef SCRIPTING_H
#define SCRIPTING_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

void scripting_exec(const char* scriptname);

#endif // !SCRIPTING_H

