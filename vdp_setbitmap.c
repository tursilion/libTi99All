#include "vdp.h"

extern void vsetchar_base(int pAddr, unsigned char ch);

unsigned char set_bitmap_raw(unsigned char sprite_mode) {
	// note: no masking, full size bitmap mode
    scrn_scroll = scrn_scroll_default;

	unsigned char unblank = VDP_MODE1_16K | VDP_MODE1_UNBLANK | VDP_MODE1_INT | sprite_mode;
	VDP_SET_REGISTER(VDP_REG_MODE1, VDP_MODE1_16K);		// no need to OR in the sprite mode for now
	// because we blanked the display, we don't have to worry about VDP timing for the rest of this
	VDP_SET_REGISTER(VDP_REG_MODE0, VDP_MODE0_BITMAP);
	VDP_SET_REGISTER(VDP_REG_SIT, 0x06);	gImage = 0x1800;
	VDP_SET_REGISTER(VDP_REG_CT, 0xFF);		gColor = 0x2000;
	VDP_SET_REGISTER(VDP_REG_PDT, 0x03);	gPattern = 0x0000;
	VDP_SET_REGISTER(VDP_REG_SAL, 0x36);	gSprite = 0x1B00;	vdpchar(gSprite, 0xd0);
	VDP_SET_REGISTER(VDP_REG_SDT, 0x03);	gSpritePat = 0x1800;
	nTextRow = 736;
	nTextEnd = 767;
	nTextPos = nTextRow;
	nTextFlags = TEXT_FLAG_IS_BITMAPPED | TEXT_FLAG_HAS_ATTRIBUTES | TEXT_WIDTH_32;
	vsetchar = vsetchar_base;
	return unblank;
}

void set_bitmap(unsigned char sprite_mode) {
    unsigned char x = set_bitmap_raw(sprite_mode);
    VDP_SET_REGISTER(VDP_REG_MODE1, x);
    VDP_REG1_KSCAN_MIRROR = x;
}
