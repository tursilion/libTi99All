#include "conio.h"

void cvline(int len) {
    int orig_conio_y = conio_y;
    int end = conio_y+len;
    for (; conio_y<end; ++conio_y) {
        if (conio_y > 23) break;
        vsetchar(getscreenoffset(conio_x, conio_y), '|'|conio_reverseMask);
    }
    gotoy(orig_conio_y);
}
