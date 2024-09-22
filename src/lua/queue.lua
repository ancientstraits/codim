local M = {}
M.__index = M

setmetatable(M, {
    __call = function(self, tbl)
        return setmetatable(tbl, M)
    end
})

function M:over()
    return #self.times == 0
end

function M:update(t)
    -- Will only check 1st element if `self.sequential` is on
    local max = self.sequential and 1 or #self.times

    if self:over() then return end

    if self.sequential and (t >= self.times[1]) then
        print(#self.times, self.times[1], t)
        table.remove(self.times, 1)
        self:onupdate(t)
    else
        for i=1,#self.times do
            if t >= self.times[i] then
                table.remove(self.times, i)
                self:onupdate(t)
            end
        end
    end
end

return M
