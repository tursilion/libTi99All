#include "vdp.h"

void vchar(unsigned char r, unsigned char c, unsigned char ch, int cnt) {
	int pAddr = getscreenoffset(c, r) + gImage;

    int diff = nTextEnd - nTextRow + 1;
    while (cnt--) {
        vdpchar(pAddr, ch);
        pAddr+=diff;
	}
}
