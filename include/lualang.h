#ifndef LUALANG_H
#define LUALANG_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

int eval_lua_script(const char *filename, const char *lib_name);

#endif