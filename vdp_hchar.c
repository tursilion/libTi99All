#include "vdp.h"

void hchar(unsigned char r, unsigned char c, unsigned char ch, int cnt) {
	int pAddr = getscreenoffset(c, r) + gImage;
	vdpmemset(pAddr, ch, cnt);
}
