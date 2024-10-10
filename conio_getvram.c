#include "vdp.h"
#include "conio.h"

// cursor position
int conio_x=0,conio_y=0;
// cache of screen color - foreground is only applicable to text mode
unsigned int conio_scrnCol = (COLOR_WHITE<<4) | COLOR_DKBLUE;

// get a VRAM address based on the screen mode we're in
unsigned int conio_getvram() {
    return getscreenoffset(conio_x, conio_y)+gImage;
}
