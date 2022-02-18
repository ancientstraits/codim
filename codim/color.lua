local M = {}

function M.rgba(r, g, b, a)
    return {r / 255, g / 255, b / 255, a and a or 1.0}
end

function M.hex(hex, alpha)
    return {
        tonumber(hex:sub(1, 2), 16) / 255,
        tonumber(hex:sub(3, 4), 16) / 255,
        tonumber(hex:sub(5, 6), 16) / 255,
        alpha and alpha or 1.0,
    }
end

return M