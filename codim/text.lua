local cairo = require('codim.cairo')

local M = {}

local function split_str(str, sep)
    sep = sep or '%s'

    local ret = {}
    for s in string.gmatch(str, '([^' .. sep .. ']+)') do
        table.insert(ret, s)
    end

    return ret
end

-- fontopts = {
--     face = "",
--     slant = "[normal, italic, oblique]",
--     weight = "[normal, bold]"
--     size = 40.0
-- }

local slantmap = {
    ['normal']  = cairo.FONT_SLANT_NORMAL,
    ['italic']  = cairo.FONT_SLANT_ITALIC,
    ['oblique'] = cairo.FONT_SLANT_OBLIQUE,
}
local weightmap = {
    ['normal'] = cairo.FONT_WEIGHT_NORMAL,
    ['bold']   = cairo.FONT_WEIGHT_BOLD,
}

function M.new(text, x, y, color, fontopts)
    fontopts = fontopts or {}
    fontopts.face = fontopts.face or 'Mono'
    fontopts.slant = fontopts.slant or 'normal'
    fontopts.weight = fontopts.slant or 'normal'
    fontopts.size = fontopts.size or 20.0
    return setmetatable({
        text = split_str(text, '\n'),
        color = color,
        x = x,
        y = y,
        face = fontopts.face,
        size = fontopts.size,
        slant = slantmap[fontopts.slant],
        weight = weightmap[fontopts.weight],
    }, {__index = M})
end

function M:setup(cr)
    cairo.set_source_rgba(cr, unpack(self.color))
    cairo.select_font_face(cr, self.face, self.slant, self.weight)
    cairo.set_font_size(cr, self.size)
end

function M:draw_tbl(cr, tbl)
    local y = self.y
    for _, text in ipairs(tbl) do
        cairo.move_to(cr, self.x, y)
        cairo.show_text(cr, text)
        y = y + self.size
    end
end

function M:draw_tbl_until(cr, tbl, li, last)
    local y = self.y
    for i = 1,li - 1 do
        cairo.move_to(cr, self.x, y)
        cairo.show_text(cr, tbl[i])
        y = y + self.size
    end
    cairo.move_to(cr, self.x, y)
    cairo.show_text(cr, tbl[li]:sub(1, last))
end

function M:wait(seconds)
    local time = 0
    return function(cr, _, dt)
        cairo.move_to(cr, self.x, self.y)
        cairo.show_text(cr, self.text)
        time = time + dt
        return time < seconds
    end
end

function M:type_duration(time_typing, time_after)
    local len = 0
    for _, line in ipairs(self.text) do
        len = len + #line
    end
    -- TODO multiplying the len by a factor fixes a problem.
    -- without it, the typing would not be correct.
    -- This needs to be figured out in the long term.
    return self:type(time_typing / (len * 1.2), time_after)
end

function M:type(rate, time_after)
    local index = 1
    local char = 1
    local anim_done = false
    local incr = 0
    local time = 0
    return function(cr, _, dt)
        self:setup(cr)
        if anim_done then
            cairo.move_to(cr, self.x, self.y)
            self:draw_tbl(cr, self.text)
            time = time + dt
            return time < time_after
        else
            self:draw_tbl_until(cr, self.text, index, char)

            incr = incr + dt
            if incr < rate then return true end
            incr = 0
            if char == #self.text[index] then
                if index == #self.text then
                    anim_done = true
                else
                    index = index + 1
                    char = 0
                end
            else
                char = char + 1
            end
            return true
        end
    end
end

return M