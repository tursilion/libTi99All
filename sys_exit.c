// exit reboots the system
#ifdef CLASSIC99
#include <stdlib.h>
#include "vdp.h"
#endif
#include "system.h"

#ifdef TI99
void exit() {
  __asm__ volatile ("clr @>83c4\n\tBLWP @>0000");
}
#endif

#ifdef COLECO
void exit() {
	// reboots the cart but not the BIOS, address defined by the CRT0 in use
	static void (* const hwreboot)()=0x802c;
	hwreboot();
}
#endif

#ifdef GBA
void exit() {
    // full software reboot vector (jump to start of ROM)
    static void (* const hwreboot)()=(void (*const)())0x08000000;
    hwreboot();
}
#endif

#ifdef CLASSIC99
void exit() {
#undef exit
    // reset the emulator. it should be running a spinloop at >a000
    // first, write a BLWP @>0000
    WriteWordToClassic99(0xa002, 0x0420);
    WriteWordToClassic99(0xa002, 0x0000);
    // clear the interrupt hook
    WriteWordToClassic99(0x83c4, 0x0000);
    // and release the infinite loop
    WriteByteToClassic99(0xa001, 0x00);

    // exit the app
    exit(0);
}
#endif
