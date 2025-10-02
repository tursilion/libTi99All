#include "vdp.h"

#ifndef CLASSIC99
void vdpmemcpy(int pAddr, const unsigned char *pSrc, int cnt) {
	VDP_SET_ADDRESS_WRITE(pAddr);
	while (cnt--) {
		VDPWD(*(pSrc++));
	}
}
#endif

#ifdef CLASSIC99
void vdpmemcpy(int pAddr, const unsigned char *pSrc, int cnt) {
    WriteVDPBlockToClassic99(pAddr, (unsigned char*)pSrc, cnt);
}
#endif


