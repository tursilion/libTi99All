
#include "vdp.h"

// room for the patterns of the displayable characters
unsigned char bmFont[BM_FONT_SIZE];

void bm_consolefont() {
  // setup graphics mode and load the charset from grom
  // then copy those patterns out to RAM
#ifdef TI99
	// TODO: we can do better here and go from GROM straight to RAM
	charset();
	vdpmemread(gPattern + (8 * 32), bmFont, BM_FONT_SIZE);
	gBmFont = bmFont;
#endif
#ifdef COLECO
	// just point the pointer at ROM
	// TODO: not tested - code assumes first character is space
	#define COLECO_FONT (unsigned char*)0x15A3
	gBmFont = COLECO_FONT;
#endif
}



