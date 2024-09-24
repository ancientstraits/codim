# Buggy Branch
This branch has a bug in it, where an OpenGL error is given if list operations are done in a function.
Here's what we know:
- It only happens if a `userdata` that is very big, like 225700 bytes, is allocated.
- It doesn't have anything to do with running out of RAM, since using `malloc` instead works well.
- Somehow, this affects the OpenGL library.
- It occurs in the `render` function in `glBindTexture`, which gives an `GL_INVALID_OPERATION`.
- When `render` is called multiple times per frame, it does not make the error come faster, meaning that the error is not fully caused by `render`.

This error is very big and hard to fix, but if you in the future can fix it, that would be great!

# Codim: A Programming Animation Generator
Codim is software that can be used to script videos easily with Lua.
For example, take this Lua script:
```lua
local cm = require('codim')

cm.output('out.mp4', {
	-- Settings to make a typical 600x400 mp4
	sample_rate = 44100,
	width = 600,
	height = 400,
	fps = 24,
})

-- sin(x) but with range [0,1] and period of 1s
function mysin(x)
	return (math.sin((2*math.pi)*x) / 2.0) + 0.5
end
function mycos(x)
	return mysin(x+(1/4))
end

-- This function changes each frame based on `t`,
-- the number of seconds of the video.
cm.procedural(0, 10, function(t)
	-- cm.changebg(r, g, b) changes the background
	cm.changebg(0.0, mycos(t), mysin(t))
end)

```

This is the output:
https://github.com/ancientstraits/codim/assets/73802848/6dbc4b20-cf88-4abc-bf2d-84587fc9dd10

## Progress
Codim is an extremely new (albeit two years old) program, so it
is still in development. Soon, it will get a controllable text editor.

