#include "conio.h"

unsigned int bgcolor(unsigned int color) {
    unsigned int ret = conio_scrnCol&0x0f;
    conio_scrnCol=(conio_scrnCol&0xf0)|(color&0x0f);
    // preserve the old behaviour - mostly for text mode
    if (!(nTextFlags & TEXT_FLAG_HAS_ATTRIBUTES)) {
        VDP_SET_REGISTER(VDP_REG_COL, conio_scrnCol);
    }
    return ret;
}
