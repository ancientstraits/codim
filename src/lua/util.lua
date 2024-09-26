local M = {}

M.new_index_fn = function(idx)
    return function(self, k)
        local obj = self[idx]
        if type(obj[k]) == 'function' then
            return function(self, ...)
                return obj[k](obj, ...)
            end
        else
            return obj[k]
        end
    end
end

return M
