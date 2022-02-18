#!/usr/bin/env luajit

if #arg < 1 then
    print(string.format('usage: %s [LUA_SCRIPT]', arg[0]))
    os.exit(1)
end

local function load_codim(...)
    local ret = {}
    for _, v in ipairs({...}) do
        ret[v] = require('codim.' .. v)
    end
    package.preload['codim'] = function() return ret end
end

load_codim(
    'cairo_utils',
    'cairo',
    'color',
    'rect',
    'text',
    'tts',
    'util',
    'videoframe',
    'writer'
)

local filename = arg[1]
dofile(filename)