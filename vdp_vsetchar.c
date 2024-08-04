#include "vdp.h"

// put a text character on the screen at the passed vram address
// This is valid for all but 64-column and the color text modes
// This method is not exposed in vdp.h so people stop using it directly
void vsetchar_base(int pAddr, unsigned char ch) {
	VDP_SET_ADDRESS_WRITE(pAddr+gImage);
	VDPWD(ch);
}

void (*vsetchar)(int pAddr, unsigned char ch) = vsetchar_base;
