#include "vdp.h"

static const unsigned char masks[] = {
	0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01
};

// warning: like most of these, there's no range checking!
// don't draw off the screen!
// TODO: does not deal with aligned lines less than 8 pixels wide
void bm_sethlinefast(unsigned int x0, unsigned int y0, unsigned int x1) {
	// Calculate the starting address
//	unsigned int addr = (8 * (x0/8)) + (256 * (y0/8)) + (y0%8) + gPattern;
	unsigned int addr = ((x0>>3)<<3) + ((y0>>3)<<8) + (y0&0x07) + gPattern;

	// do first byte
	if (x0&0x07) {
		VDP_SET_ADDRESS(addr);
		unsigned char bits = VDPRD;
		bits |= masks[x0&0x07];
		VDP_SET_ADDRESS_WRITE(addr);
		VDPWD = bits;
		addr+=8;
		x0 = (x0&0xf8) + 8;
	}
	
	// do middle bytes - these are guaranteed solid
	int range = x1-x0+1;
	while (range >= 8) {
		VDP_SET_ADDRESS_WRITE(addr);
		VDPWD = 0xff;
		addr+=8;
		range -= 8;
	}
	
	// do last byte
	if (range) {
		VDP_SET_ADDRESS(addr);
		unsigned char bits = VDPRD;
		bits |= ~masks[range];
		VDP_SET_ADDRESS_WRITE(addr);
		VDPWD = bits;
	}	
}
