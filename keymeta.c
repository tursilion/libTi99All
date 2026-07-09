#include "kscan.h"

#ifdef TI99
unsigned char keymeta() {
    unsigned char ret = 0;
    unsigned int key;

    // check fctn: col 0, row address 14
    // check shift: col 0, row address 16
    // check ctrl: col 0, row address 18
    __asm__("clr r0\n\tli r12,>0024\n\tldcr r0,3\n\tsrc r12,7\n\tli r12,14\n\tclr %0\n\tstcr %0,3\n\tinv %0" : "=r"(key) : : "r12","cc");

    // key should now contain: 1111 1CSF 1111 1111 - active HIGH
    ret = (key>>8)&0xff;
    return ret;
}
#endif

#ifdef COLECO 

// not implemented
unsigned char keymeta() {
    return 0;
}

#endif

#ifdef SMS

// not implemented
unsigned char keymeta() {
    return 0;
}

#endif

#ifdef GBA
// not implemented
unsigned char keymeta() {
    return 0;
}
#endif

#ifdef CLASSIC99
#include <Windows.h>

unsigned char keymeta() {
    unsigned char ret = 0;

    if (GetAsyncKeyState(VK_SHIFT)&0x8000) {
        ret |= KEY_SHIFT;
    }
    if (GetAsyncKeyState(VK_MENU)&0x8000) {
        ret |= KEY_FCTN;
    }
    if (GetAsyncKeyState(VK_CONTROL)&0x8000) {
        ret |= KEY_CTRL;
    }
    return ret;
}

#endif
