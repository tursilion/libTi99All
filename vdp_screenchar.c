#include "vdp.h"

void vdpscreenchar(int pAddr, unsigned char ch) {
	VDP_SET_ADDRESS_WRITE(pAddr+gImage);
	VDPWD=ch;
}
