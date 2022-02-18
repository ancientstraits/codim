local cairo = require('codim.cairo')

local M = {}

function M.new(w, h)
    local csurf = cairo.image_surface_create(cairo.FORMAT_ARGB32, w, h)
    local cairo = cairo.create(csurf)
    return setmetatable({
        csurf = csurf,
        cairo = cairo,
        w = w,
        h = h,
        frameno = 0,
    }, {__index = M})
end

function M:free()
    cairo.surface_destroy(self.surface)
    cairo.destroy(self.cairo)
end

function M:write(f, times)
    cairo.surface_flush(self.csurf)
    local data = cairo.image_surface_get_data(self.csurf)
    local stride = cairo.image_surface_get_stride(self.csurf)
    for i = 1,times do
        for y = 0,(self.h - 1) do
            for x = 0,(self.w - 1) do
                for j = 3,0,-1 do
                    -- print(data[(y * stride) + (x * 4) + j])
                    f:write(string.char(data[(y * stride) + (x * 4) + j]))
                    -- f:write(string.char(255))
                end
                -- print('----')
            end
        end
        print(string.format('[%d/%d]', i + self.frameno, times + self.frameno))
    end
    self.frameno = self.frameno + times
end

return M