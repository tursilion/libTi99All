#include "vdp.h"
#include "conio.h"

int conio_x=0,conio_y=0;

// get a VRAM address based on the screen mode we're in
unsigned int conio_getvram() {
    return getscreenoffset(conio_x, conio_y)+gImage;
}
