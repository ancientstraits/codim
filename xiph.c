#include <stdio.h>
#include <stdlib.h>

#include <ogg/ogg.h>
#include <theora/codec.h>
#include <theora/theora.h>
#include <theora/theoraenc.h>

#define WIDTH  1920
#define HEIGHT 1080

int main() {
    th_enc_ctx* tec = th_encode_alloc(&(th_info){
        .frame_width    = WIDTH,
        .frame_height   = HEIGHT,
        .pic_width      = WIDTH,
        .pic_height     = HEIGHT,
        .pic_x          = 0,
        .pic_y          = 0,
        .colorspace     = TH_CS_ITU_REC_470M,
        .pixel_fmt      = TH_PF_420,
        .target_bitrate = 0, // variable bitrate
        .quality        = 50, // 0-63
        .keyframe_granule_shift = 6,
    });
    if (!tec) {
        fprintf(stderr, "Could not initialize theora\n");
    }
}
