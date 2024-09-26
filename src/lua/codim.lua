local Queue = require('codim.queue')
local internal = require('codim.internal')
local util = require('codim.util')

local old_Renderer = internal.Renderer
internal.Renderer = function()
    return setmetatable({
        renderer = old_Renderer(),
        dependencies = {},
        add = function(self, dep)
            if (type(dep) == 'table' and dep.editor ~= nil) then
                self.renderer:add(dep.editor)
            elseif type(dep) == 'userdata' then
                self.renderer:add(dep)
            end
            table.insert(self.dependencies, dep)
            print('added', #self.dependencies, type(dep))
        end,
        del = function(self, dep)
            self.renderer:del(dep)
            for i, d in pairs(self.dependencies) do
                if d == dep then
                    table.remove(self.dependencies, i)
                end
            end
        end,
    }, {
        __index = util.new_index_fn('renderer')
    })
end

local old_Editor = internal.Editor
internal.Editor = function(text, ...)
    return setmetatable({
        text = text,
        editor = old_Editor(text, ...)
    }, {
        __index = util.new_index_fn('text')
    })
end

local editor_table = internal.getmetatable('codim.editor')
function editor_table:type(text, start, letters_per_sec)
    local times = {}
    for i=0,#text-1 do
        times[#times+1] = i/letters_per_sec
    end

    local idx = 1
    return Queue{
        sequential = true,
        times = times,
        onupdate = function(t)
            print(text:sub(idx, idx))
            self:insert('M')
            self:insert(text:sub(idx, idx))
            idx = 1
        end
    }
end

return internal
