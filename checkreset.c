// Fast reset check for various systems
// This code and library released into the Public Domain
// You can copy this file and use it at will ;)

#include "kscan.h"

#ifdef TI99
// TODO: this function requires the TI ROM (0012 = 0008, 004c = 1100)
unsigned char check_reset() {
    unsigned char ret;
    
    __asm__ volatile("clr %0\n\tli r12,>0024\n\tli r0,8\n\tldcr r0,3\n\tsrc 12,7\n\tli r12,6\n\tstcr r0,8\n\tli r12,>1100\n\tczc r12,r5\n\tjne 2\n\tseto %0" : "=r"(ret) : : "r12","cc");
    
    return ret;
}
#endif

#ifdef COLECO

// Coleco and SMS differ here

#ifdef SMS
static volatile __sfr __at 0xdd pad1;
unsigned char check_reset() {
	key=pad1;
	if (key&0x10) {
        return 0xff;
    } else {
        return 0;
    }
}

#else

// Coleco Version
// TODO: do I want a dedicated reset sequence? Probably?

unsigned char check_reset() {
    return 0;
}

#endif
#endif


