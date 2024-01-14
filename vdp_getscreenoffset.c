#include "vdp.h"

// get a VRAM offset based on the screen mode we're in
unsigned int getscreenoffset(int x, int y) {
    if (nTextFlags&TEXT_WIDTH_40) {
        return VDP_SCREEN_TEXT(y,x);
    } else if (nTextFlags&TEXT_WIDTH_80) {
        return VDP_SCREEN_TEXT80(y,x);
    } else if (nTextFlags&TEXT_WIDTH_64) {
        // 64 column does not use a real VDP address, so it's unclear how helpful this is
        return VDP_SCREEN_TEXT64(y,x);
    } else {
        return VDP_SCREEN_POS(y,x);
    }
}

