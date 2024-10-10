#include "conio.h"

unsigned int textcolor(unsigned int color) {
    unsigned int ret = (conio_scrnCol&0xf0)>>4;
    conio_scrnCol=(conio_scrnCol&0x0F)|((color&0x0f)<<4);
    // preserve the old behaviour
    if (!(nTextFlags & TEXT_FLAG_HAS_ATTRIBUTES)) {
        VDP_SET_REGISTER(VDP_REG_COL, conio_scrnCol);
    }
    return ret;
}

