
#include "vdp.h"

#ifdef TI99
// room for the patterns of the displayable characters - not needed on Coleco
// TODO: maybe we can rework the TI code to pull from GROM instead of buffering in RAM?
unsigned char bmFont[BM_FONT_SIZE];
#endif

void bm_consolefont() {
  // load the charset from grom
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
#ifdef GBA
    // TODO this is to get the character set in RAM for bitmap mode
    // But we aren't supporting bitmap mode anyway
    gBmFont = (unsigned char*)0;
#endif
}



