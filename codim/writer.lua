local M = {}

local VideoFrame = require('codim.videoframe')

function M.new(w, h, fps)
    return setmetatable({
        frame = VideoFrame.new(w, h),
        fps = fps,
        time = 0,
        rel_time = 0,
        animations = {},
        out = '',
    }, {__index = M})
end

function M:open(out)
    self.out = out
    self.process = io.popen(table.concat({
        'ffmpeg',
        '-y', -- overwrite
        '-f', 'rawvideo', -- accept raw video
        '-pix_fmt', 'argb', -- the pixel format needs 4 components
        '-s', string.format('%dx%d', self.frame.w, self.frame.h), -- resolution
        '-r', tostring(self.fps), -- framerate
        '-i', '-', -- use pipe for input
        '-an', -- no audio
        out, -- the output file
        '2>/dev/null',
    }, ' '), 'w')
    assert(self.process)
end

function M:close()
    assert(self.process:close())
    self.frame:free()
end

function M:write()
    self.frame:write(self.process, 1)
    self.time = self.time + 1
    self.rel_time = self.rel_time + 1
end

function M:time_reset()
    self.rel_time = 0
end

function M:add(tbl, ts)
    if not ts then ts = 0 end
    local time = (self.time + ts) * self.fps
    if self.animations[time] then
        for _, v in ipairs(tbl) do
            table.insert(self.animations[time], v)
        end
    else
        self.animations[time] = tbl
    end
end

function M:play()
    local cur_anims = self.animations[0]
    local i = 0
    while #cur_anims > 0 do
        if self.animations[i] and i ~= 0 then
            for _, animation in ipairs(self.animations[i]) do
                cur_anims[#cur_anims+1] = animation
            end
        end
        for j = 1,#cur_anims do
            if not cur_anims[j] then goto continue end
            local x = cur_anims[j](self.frame.cairo, self.frame.csurf, 1 / self.fps)
            if not x then
                -- table.remove(cur_anims, j)
                cur_anims[j] = nil
            end
            ::continue::
        end
        self:write()
        i = i + 1
    end
end

return M