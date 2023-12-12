#include "vdp.h"

void faster_hexprint(unsigned char x) {
	unsigned int dat = byte2hex[x];

#ifdef TI99
    // the TI GCC currently doesn't optimize the MSB access very well, so this saves two 8 bit shifts
    // and a handful of instructions.
    __asm__( "movb %0,@>8c00\n\tswpb %0\n\tmovb %0,@>8c00\n\tswpb %0" : : "r"(dat) : "cc");
#else
	VDPWD = dat>>8;
	VDP_SAFE_DELAY();
	VDPWD = dat&0xff;
#endif
}
