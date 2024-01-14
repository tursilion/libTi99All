#include "vdp.h"

void vchar(unsigned char r, unsigned char c, unsigned char ch, int cnt) {
	int pAddr = getscreenoffset(c, r) + gImage;

	if (nTextFlags & TEXT_WIDTH_64) {
		// special case for 64 column - may not do what you expect
		while (cnt--) {
			vdpchar64(pAddr, ch);
			pAddr+=64;
		}
	} else {
		int diff = nTextEnd - nTextRow + 1;
		while (cnt--) {
			vdpchar(pAddr, ch);
			pAddr+=diff;
		}
	}
}
