#include "vdp.h"

// put a text character on the screen at the passed vram address
// This is valid for all but 64-column and the color text modes
// This method is not exposed in vdp.h so people stop using it directly
// This is expected to be an offset into the current screen!
void vsetchar_base(int pOffset, unsigned char ch) {
	VDP_SET_ADDRESS_WRITE(pOffset+gImage);
	VDPWD(ch);
}

void (*vsetchar)(int pOffset, unsigned char ch) = vsetchar_base;
