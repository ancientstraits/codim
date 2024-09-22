local cm = require('codim')

cm.output{
    file = 'out.mp4',
    sample_rate = 44100,
    width = 600,
    height = 400,
    fps = 24
}

function s(x)
    return x:sub(1, #x-1)
end

-- function insert(editor, text, start, letters_per_sec)
--     local times = {}
--     for i=0,#text-1 do
--         times[#times+1] = i/letters_per_sec
--         -- print(times[#times])
--     end
--     idx = 1
--     return AnimQueue{
--         sequential = true,
--         times = times,
--         onupdate = function(t)
--             editor:insert(text:sub(idx, idx))
--             idx = idx + 1
--         end
--     }
-- end

local r = cm.Renderer()
local text = cm.Text(50, "mono.ttf")

print'a'
local text_rd = text:render(s[[
#include <stdio.h>
int main() {
    printf("codim is real.\n");
    return 0;
}
]], 0, 300)
r:add(text_rd)
print'b'

return {
    video = function(t)
		if t >= 5.0 then cm.stop() end

        cm.clear(0.3, 0.4, 0.5, 0.0)
        -- r:render(600, 400)
		r:render()
    end,
    audio = function(t)
        return 0, 0
    end
}

