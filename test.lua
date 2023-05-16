local cm = require('codim')
cm.output('out.mp4', {
	sample_rate = 44100,
	width = 600,
	height = 400,
	fps = 24,
})


function mysin(x)
	return (math.sin((2*math.pi)*x) / 2.0) + 0.5
end
function mycos(x)
	return mysin(x+(1/4))
end

cm.procedural(0, 10, function(t)
	if t == 0 then
		print('yes')
		cm.something()
	end
	cm.changebg(0.0, mycos(t), mysin(t))
end)

