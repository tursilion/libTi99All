// Fast reset check for various systems
// This code and library released into the Public Domain
// You can copy this file and use it at will ;)

#include "kscan.h"

#ifdef TI99
unsigned char check_reset() {
    unsigned char ret;
    
    // test for QUIT
    __asm__ volatile("clr %0\n\tli r12,>0024\n\tli r0,8\n\tldcr r0,3\n\tsrc 12,7\n\tli r12,6\n\tstcr r0,8\n\tli r12,>1100\n\tczc r12,r0\n\tjne 2\n\tseto %0" : "=r"(ret) : : "r12","cc");
    
    return ret;
}
#endif

#ifdef CLASSIC99
#include <Windows.h>
unsigned char check_reset() {
    if ((GetAsyncKeyState(VK_CONTROL)&0x8000) && (GetAsyncKeyState(VK_OEM_PLUS)&0x8000)) {
        return 1;
    }
    return 0;
}
#endif

#ifdef COLECO

// Coleco and SMS differ here

#ifdef SMS
static volatile __sfr __at 0xdd pad1;
unsigned char check_reset() {
    // test for reset button
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

#ifdef GBA

// GBA Version
// TODO: do I want a dedicated reset sequence? Probably?

unsigned char check_reset() {
    return 0;
}

#endif

