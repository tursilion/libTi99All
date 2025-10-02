#include "vdp.h"

#ifndef CLASSIC99
void vdpmemread(int pAddr, unsigned char *pDest, int cnt) {
	VDP_SET_ADDRESS(pAddr);
	while (cnt--) {
		*(pDest++)=VDPRD();
	}
}
#endif

#ifdef CLASSIC99
void vdpmemread(int pAddr, unsigned char *pDest, int cnt) {
    ReadVDPBlockFromClassic99(pAddr, pDest, cnt);
}
#endif
