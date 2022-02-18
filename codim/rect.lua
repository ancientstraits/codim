local cairo = require('codim.cairo')
local cairo_utils = require('codim.cairo_utils')

local M = {}

function M.new(x, y, w, h, color)
    return setmetatable({
        x = x,
        y = y,
        w = w,
        h = h,
        color = color,
    }, {__index = M})
end

function M:wait(seconds)
    local time = 0
    return function(cr, _, dt)
        -- if time == 0 then
        cairo_utils.rect(cr, self.x, self.y, self.w, self.h, self.color)
        -- end
        time = time + dt
        return time < seconds
    end
end

function M:move_relative(x, y, seconds)
    self:move(self.x + x, self.y + y, seconds)
end

function M:move(x, y, seconds)
    local xdiff = (x - self.x) / seconds
    local ydiff = (y - self.y) / seconds
    local time = 0
    return function(cr, _, dt)
        self.x = self.x + (xdiff * dt)
        self.y = self.y + (ydiff * dt)
        cairo_utils.rect(cr,
            self.x, self.y,
            self.w, self.h,
        self.color)
        time = time + dt
        return time < seconds
    end
end


return M