#include "conio.h"

// sets the border color but does NOT change the attribute background color
unsigned int bordercolor(unsigned int x) {
    unsigned int ret = conio_scrnCol&0xf0;
    VDP_SET_REGISTER(VDP_REG_COL, ret | (x&0x0f));
    return 0;
}
