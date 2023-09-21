#include "vdp.h"

unsigned char vdpreadchar(int pAddr) {
	VDP_SET_ADDRESS(pAddr);
	VDP_SAFE_DELAY();
#ifdef TI99
	__asm("NOP");
#endif
	return VDPRD;
}
