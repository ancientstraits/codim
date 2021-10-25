#ifndef LUALANG_H
#define LUALANG_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

int exec_lua(const char* file_path);

#endif