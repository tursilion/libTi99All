#include "vdp.h"

void vdpchar_default(int pAddr, unsigned char ch) {
	VDP_SET_ADDRESS_WRITE(pAddr);
	VDPWD=ch;
}

void (*vdpchar)(int pAddr, unsigned char ch) = vdpchar_default;

