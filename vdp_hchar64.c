#include "vdp.h"

void hchar64(unsigned char r, unsigned char c, unsigned char ch, int cnt) {
	int pAddr = getscreenoffset(c, r) + gImage;

	if (nTextFlags & TEXT_CUSTOM_VSETCHAR) {
		// have to do it slower due to custom character output on this screen
		while (cnt--) {
			vsetchar(pAddr++, ch);
		}
	} else {
		vdpmemset(pAddr, ch, cnt);
	}
}
