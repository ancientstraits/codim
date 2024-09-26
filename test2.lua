local cm = require('codim')
-- local what = require('codim.what')
-- print(what)

local AnimQueue = require('animqueue')

cm.output{
    file = 'out2.mp4',
    sample_rate = 44100,
    width = 600,
    height = 400,
    fps = 24
}

function s(x)
    return x:sub(1, #x-1)
end

function insert(editor, text, start, letters_per_sec)
    local times = {}
    for i=0,#text-1 do
        times[#times+1] = i/letters_per_sec
        -- print(times[#times])
    end
    idx = 1
    return AnimQueue{
        sequential = true,
        times = times,
        onupdate = function(t)
            -- editor:insert(text:sub(idx, idx))
            -- local x = ('jeoijewoijfewoiewjfwe'):sub(idx, idx)
            -- print(idx)
            idx = idx + 1
        end
    }
end

local r = cm.Renderer()
local text = cm.Text(40, "mono.ttf")

local editor = cm.Editor(text, 0, 300)
-- local rd = text:render(s[[
-- #include <hello.h>
-- int main() {
--     printf("codim is real.\n");
--     return 0;
-- }
-- ]], 0, 300)
-- r:add(rd)
-- local model = editor:get_rd():get_model()
-- model:rotz(0.5)
r:add(editor)

-- local queue = editor:type(s[[
-- #include <hello.h>
-- int main() {
--     printf("codim is real.\n");
--     return 0;
-- }
-- ]], 1.0, 10)

-- local queue = insert(editor, s[[
-- #include <hello.h>
-- int main() {
--     printf("codim is real.\n");
--     return 0;
-- }
-- ]], 1.0, 10)

local buf = cm.arecord('amogus')
-- print(getmetatable(buf))

local i = 0

return {
    video = function()
        if i >= 24 then
            cm.stop()
        end
        i = i + 1
        -- if end_t then
        --     if t >= end_t then cm.stop() end
        -- elseif queue:over() then
        --     end_t = 1.0 + t
        -- end

        -- queue:update(t)
        local a = {1, 2, 3}
        local x = a[1]
        -- os.execute('sleep 0.1')

        -- cm.clear(0.3, 0.4, 0.5, 0.0)
        r:render()
        -- r:render()
    end,
    audio = function()
        -- i = i + 1
        -- local a = {1, 2, 3}
        -- local x = a[1]
        -- print(t)
        return 0, 0
        -- return unpack(buf:next())
    end
}
