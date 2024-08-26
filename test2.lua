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

local r = cm.Renderer()
local text = cm.Text(20, "mono.ttf")
local editor = cm.Editor(text, 0, 300)
editor:insert(s[[
#include <stdio.h>
int main() {
    printf("codim is real.\n");
    return 0;
}
]])
r:add(editor)

return {
    video = function(t)
        if t >= 10.0 then
            cm.stop()
        end
        cm.clear(0.3, 0.4, 0.5, 0.0)
        r:render(600, 400)
    end,
    audio = function(t)
        return 0, 0
    end
}
