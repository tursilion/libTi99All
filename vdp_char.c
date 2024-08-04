#include "vdp.h"

void vdpchar(int pAddr, unsigned char ch) {
	VDP_SET_ADDRESS_WRITE(pAddr);
	VDPWD(ch);
}
