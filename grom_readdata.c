#include "grom.h"

// multiple port GROM access code - this is slow for bulk access
// but will do for this test program, I don't expect that multiple
// reads/writes will be a problem if the cart works at all. ;)
unsigned char GromReadData(unsigned int address, unsigned char port) {
    volatile unsigned char *finalAdr;
    
	// we only support 15 ports, this still fits in a char
	port <<= 2;

	// set address
	finalAdr = (volatile unsigned char*)GROMWA_0+port;
	
#ifdef TI99
    // the TI GCC currently doesn't optimize the MSB access very well, so this saves two 8 bit shifts
    // and a handful of instructions. Even using pointers the C code is still 4 instructions long
    __asm__( "movb %1,*%0\n\tswpb %1\n\tmovb %1,*%0\n\tswpb %1" : : "r"(finalAdr), "r"(address) : "cc");
#else	
	*finalAdr = address>>8;
	*finalAdr = address&0xff;
#endif

	// read data
	return *((volatile unsigned char*)(GROMRD_0+port));
}
