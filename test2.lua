local cm = require('codim')

function mysin(x)
	return (math.sin((2*math.pi)*x) / 2.0) + 0.5
end
function mycos(x)
	return mysin(x+(1/4))
end

cm.output{
    file = 'out.mp4',
    sample_rate = 44100,
    width = 600,
    height = 400,
    fps = 24
}

local r = cm.Renderer()
local text = cm.Text(100, "sample.ttf")
local text_rd = text:render("Hello, World!", 60, 60)
r:add(text_rd)

return {
    audio = nil,
    video = function(t)
        if t >= 10.0 then
            cm.stop()
        end
        cm.clear(0.0, 1.0, 0.0, 0.0)
        r:render(600, 400)
    end
}
