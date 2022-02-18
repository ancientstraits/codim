# Codim - the CODing anIMation software

Codim is a program to easily script programming tutorials.

# Installation

## Dependencies

- [eSpeak](http://espeak.sourceforge.net/)
- [FFmpeg](https://www.ffmpeg.org/)
- [luaJIT](https://luajit.org/)

If all of these dependencies are installed, just run the install script:
```sh
./install.sh
```


# Example:

The video will be configured in Lua. This example script can be found in `examples/codim`.:

`tutorial.lua`
```lua
local cm = require('codim')

local w = cm.writer.new(600, 400, 24)
w:open('out.mp4')

local bg = cm.rect.new(0, 0, 600, 400, cm.color.hex('00a0a0'))
w:add({bg:wait(12)}, 0)

-- text
w:add({cm.text.new([[
int isImproving = 1;
const AVRational difficulty = {1000, 1};
local ease = true
os.execute('ffmpeg')
local unstable = true
function improving() return true end
]], 20, 20, cm.color.hex('ffffff')):type_duration(25, 0)}, 0)

w:play()

w:close()

-- text to speech
cm.tts.say('out.mp4', {
    {'Much progress has been made on codim since the last update.', 0},
    {'When using C, I tried to add audio support, but I kept encountering problems with the FFmpeg C API.', 5},
    {'That is when I decided that I would move to Lua for the time being. It was far easier to use.', 13},
    {'I also am using the FFmpeg CLI in this version.', 19},
    {'This version is also finnicky, but at least it is progress.', 23},
})
```

Then, run it:
```sh
codim tutorial.lua
```

This is `out.mp4`:

https://user-images.githubusercontent.com/73802848/154703115-0bb46083-3032-43ae-ab0e-4f37fdb8fb74.mp4
