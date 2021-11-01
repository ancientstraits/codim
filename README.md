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

The tutorial itself will be configured in Lua or Python:

Lua:
`tutorial.lua`
```lua
Tutorial = {
    files = {"tutorial.c"}
    -- The config table is used to configure the appearance of the video.
    config = {
        -- The video size is {width, height}
        font = "/usr/share/fonts/TTF/FiraCode.ttf",
        video_size = {1920, 1080},
        -- This table will change the colorscheme.
        colors = {
            bg = "#330033",
        },
    },
    -- anim is a function. It can be built with codim.template, or you can make your own.
    anim = codim.template.each_line({
        -- type_speed is in words per minute (wpm)
        type_speed = 100,
        -- wait x seconds before continuing to next line
        line_wait = 3,
    }),
}
```

Then, run it:
```sh
codim tutorial.lua -o out.mp4
```

This is out.mp4:
https://user-images.githubusercontent.com/ancientstraits/assets/master/out%20-%202021-11-01T132402.971.mp4

