#include "vdp.h"

void hchar(unsigned char r, unsigned char c, unsigned char ch, int cnt) {
	int pAddr = getscreenoffset(c, r) + gImage;

	if (nTextFlags & TEXT_WIDTH_64) {
		// special case for 64 column - may not do what you expect
		while (cnt--) {
			vdpchar64(pAddr++, ch);
		}
	} else {
		vdpmemset(pAddr, ch, cnt);
	}
}
