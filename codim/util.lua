local M = {}

function M.read_file(filename)
    local f = io.open(filename, 'r')
    local ret = f:read('*a')
    f:close()
    return ret
end

return M