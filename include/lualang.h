#ifndef LUALANG_H
#define LUALANG_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

typedef struct {
    lua_State* l;
    struct {
        
    } config;
} LuaContext;

#endif