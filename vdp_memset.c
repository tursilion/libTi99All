#include "vdp.h"

#ifndef CLASSIC99
void vdpmemset(int pAddr, unsigned char ch, int cnt) {
	VDP_SET_ADDRESS_WRITE(pAddr);
	while (cnt--) {
		VDPWD(ch);
	}
}
#endif

#ifdef CLASSIC99
void vdpmemset(int pAddr, unsigned char ch, int cnt) {
    SetVDPToClassic99(pAddr, ch, cnt);
}
#endif

