// VDP code for the TI-99/4A by Tursi
// You can copy this file and use it at will if it's useful

#include "vdp.h"

#ifdef TI99
void charset() {
	gplvdp(0x004a, gPattern+0x300, 31);	// lower case letters
	gplvdp(0x0018, gPattern+0x100, 64);	// the rest of the character set (not shifted)
	vdpmemset(gPattern+(30*8), 0xfc, 8);	// cursor
}
#endif

#ifdef COLECO
#define COLECO_FONT (unsigned char*)0x15A3

void charset() {
	vdpmemcpy(gPattern+0x100, COLECO_FONT, 96*8);	// the character set (not shifted)
	vdpmemset(gPattern+(30*8), 0xfc, 8);	// cursor
}
#endif

#ifdef GBA
void charset() {
    // TODO: the GBA doesn't apparently have a ROM font, we need to provide it
    // for now I'll be very evil and copy the LC font in, but that leaves out all the numbers and punctuation
    charsetlc();
}
#endif
