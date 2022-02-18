local M = {}

local function speak_to(text, out)
    local espeak = io.popen(table.concat({
        'espeak',
        '-w',
        out,
    }, ' '), 'w')
    espeak:write(text)
    espeak:close()
end

local function to_timestamp(ts)
    local hours = math.floor(ts / 3600)
    local minutes = math.floor(ts / 60) - (hours * 3600)
    local seconds = math.floor(ts) - ((minutes * 60) + (hours * 3600))
    local decimal = ts - (seconds + (minutes * 60) + (hours * 3600))
    -- decimal = math.floor(decimal * 10000)
    -- return string.format('%02d:%02d:%02d.%04d', hours, minutes, seconds, decimal)
    return string.format('%02d:%02d:%02d', hours, minutes, seconds)
end

-- `writer` must be closed before this function is called.
function M.say(out, tbl)
    local cmd = {
        'ffmpeg',
        '-y',
    }
    local filter = ''
    local amix = ''
    for i=1,#tbl do
        local filename = string.format('.codim%d.wav', i)
        speak_to(tbl[i][1], filename)
        cmd[#cmd+1] = '-i'
        cmd[#cmd+1] = filename

        local t = math.floor(tbl[i][2] * 1000)
        filter = filter .. string.format('[%d:a]adelay=%d|%d[a%d]',
            i - 1, t, t, i - 1)
        filter = filter .. ';'

        amix = amix .. string.format('[a%d]', i - 1)
    end
    amix = amix .. 'amix=inputs=' .. #tbl .. ':duration=longest[a]'
    filter = filter .. amix
    cmd[#cmd+1] = '-filter_complex'
    cmd[#cmd+1] = '\'' .. filter .. '\''
    cmd[#cmd+1] = '-map'
    cmd[#cmd+1] = '\'[a]\''
    cmd[#cmd+1] = '.codim.wav'
    cmd[#cmd+1] = '2>/dev/null'
    print(table.concat(cmd, ' '))
    os.execute(table.concat(cmd, ' '))
    for i=1,#tbl do
        os.remove(string.format('.codim%d.wav', i))
    end
    os.execute(table.concat({
        'ffmpeg',
        '-y',
        '-i',
        out,
        '-i',
        '.codim.wav',
        '.codim.mp4',
        '2>/dev/null',
    }, ' '))
    os.remove('.codim.wav')
    os.remove(out)
    os.rename('.codim.mp4', out)
end

return M