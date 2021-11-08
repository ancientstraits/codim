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

The tutorial itself will be configured in Lua:

`tutorial.lua`
```lua
local cm = require("codim")

cm.set_video_opts {
	width  = 1920,
	height = 1080,
	output = "out.mp4",
	fps    = 24,
}

cm.fill_frame("#fff")

cm.draw_rect {
	x      = 10,
	y      = 10,
	width  = 20,
	height = 20,
	color  = "#0055ff",
}

cm.wait(300)
```

Then, run it:
```sh
codim tutorial.lua
```

This is out.mp4:

https://user-images.githubusercontent.com/73802848/139718552-278343ad-6bb8-4c56-b6c6-d42a63470874.mp4

