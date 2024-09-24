local Queue = require('codim.queue')
local internal = require('codim.internal')

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
            -- print(text:sub(idx, idx))
            print(string.sub('pppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppp', idx, idx))
            -- self:insert('M')
            -- self:insert(text:sub(idx, idx))
            idx = 1
        end
    }
end

return internal
