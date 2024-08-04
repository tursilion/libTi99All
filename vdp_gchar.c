#include "vdp.h"

unsigned char gchar(unsigned char r, unsigned char c) {
	VDP_SET_ADDRESS(gImage+(r<<5)+c);
	VDP_SAFE_DELAY();
#ifdef TI99
	// the one case it might matter, cause VDP_SAFE_DELAY does nothing on the TI
	__asm("NOP");	// address write to read turnaround can be too short!
#endif
	return VDPRD();
}
