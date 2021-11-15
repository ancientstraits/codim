local cm = require("codim")

cm.set_video_opts {
    width  = 1920,
    height = 1080,
    output = "out.mp4",
    fps    = 24,
}

cm.fill_frame("#000")

cm.draw_rect {
    x      = 10,
    y      = 10,
    width  = 1920 - 20,
    height = 20,
    color  = "#0055ff",
}


local txt = [[
#include <stdio.h>
int main() {
    printf("Hello World!\n");
    return 0;
}
#include <stdio.h>
int main() {
    printf("Hello World!\n");
    return 0;
}
#include <stdio.h>
int main() {
    printf("Hello World!\n");
    return 0;
}
#include <stdio.h>
int main() {
    printf("Hello World!\n");
    return 0;
}
#include <stdio.h>
int main() {
    printf("Hello World!\n");
    return 0;
}
]]

cm.draw_text {
    font_file = cm.font_mono(),
    font_size = 30,
    text = txt,
    x = 10,
    y = 20,
    color = "#00FF00",

    animated = true,
    animation_speed = 1,
}

cm.wait(300)
