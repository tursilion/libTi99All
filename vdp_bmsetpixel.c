#include "vdp.h"

void bm_setpixel(unsigned int x, unsigned int y) {
//	unsigned int addr = (8 * (x/8)) + (256 * (y/8)) + (y%8) + gPattern;
  unsigned int addr = ((x>>3)<<3) + ((y>>3)<<8) + (y&0x07) + gPattern;
  VDP_SET_ADDRESS(addr);
  unsigned char bits = VDPRD;
  bits = bits | (0x80 >> (x%8));
  VDP_SET_ADDRESS_WRITE(addr);
  VDPWD = bits;
  addr = (addr + gColor - gPattern) & 0x3fff;    // the mask handles the negative case
  VDP_SET_ADDRESS_WRITE(addr);
  VDPWD = gBitmapColor;
}

