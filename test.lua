local cm = require('codim')
cm.output('out.mp4', {
	sample_rate = 44100,
	width = 600,
	height = 400,
	fps = 24,
})

for i = 0,10,0.01 do
	cm.changebg(i, 0.0, i/10, 0.0)
end

