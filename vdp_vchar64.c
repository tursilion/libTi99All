#include "vdp.h"

void vchar64(unsigned char r, unsigned char c, unsigned char ch, int cnt) {
	int pAddr = getscreenoffset(c, r) + gImage;

	// it doesn't really matter much if we do it direct or not...
	int diff = nTextEnd - nTextRow + 1;
	while (cnt--) {
		vsetchar(pAddr, ch);
		pAddr+=diff;
	}
}
