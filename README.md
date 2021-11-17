# Codim - the CODing anIMation software

Codim is a program to easily script programming tutorials.

# Example:

The following file is a file that will be used for the tutorial in the video:

`tutorial.c`
```c
#include <stdio.h>

int main() {
    printf("Hello World!\n");
    return 0;
}
```

The tutorial itself will be configured in lua:

`tutorial.lua`
```lua
local cm = require("codim")

cm.set_video_opts {
    width  = 1920,
    height = 1080,
    output = "out.mp4",
    fps    = 24,
}

cm.fill_frame("#000")

-- draw an example rectangle
cm.draw_rect {
    x      = 10,
    y      = 10,
    width  = 1920 - 20,
    height = 20,
    color  = "#0055ff",
}


local function read_file(file_path)
    local f = io.open(file_path, "r")
    local content = f:read("*all")
    f:close()
    return content
end

cm.draw_text {
    font_file = cm.font_mono(),
    font_size = 50,
    text = read_file("test.c"),
    x = 10,
    y = 20,
    color = "#00FF00",

    animated = true,
    animation_speed = 3,
}

cm.wait(50)
```

Then, run it:
```sh
codim tutorial.lua
```

This is out.mp4:

https://user-images.githubusercontent.com/73802848/141845203-05008667-2188-41f4-98d6-d83a44095723.mp4
