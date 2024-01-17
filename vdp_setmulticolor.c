#include "vdp.h"

void vsetchar_base(int pAddr, unsigned char ch);

unsigned char set_multicolor_raw(unsigned char sprite_mode) {
	// this layout is untested
    scrn_scroll = scrn_scroll_default;

	unsigned char unblank = VDP_MODE1_16K | VDP_MODE1_UNBLANK | VDP_MODE1_MULTI | VDP_MODE1_INT | sprite_mode;
	VDP_SET_REGISTER(VDP_REG_MODE1, VDP_MODE1_16K | VDP_MODE1_MULTI);		// no need to OR in the sprite mode for now
	// because we blanked the display, we don't have to worry about VDP timing for the rest of this
	VDP_SET_REGISTER(VDP_REG_MODE0, 0);
	VDP_SET_REGISTER(VDP_REG_SIT, 0x00);	gImage = 0x000;
	VDP_SET_REGISTER(VDP_REG_PDT, 0x01);	gPattern = 0x800;
	VDP_SET_REGISTER(VDP_REG_SAL, 0x36);	gSprite = 0x1B00;	vdpchar(gSprite, 0xd0);
	VDP_SET_REGISTER(VDP_REG_SDT, 0x04);	gSpritePat = 0x2000;
	// no color table, it's all in the PDT
	nTextRow = 736;
	nTextEnd = 767;
	nTextPos = nTextRow;
	nTextFlags = TEXT_FLAG_IS_MULTICOLOR | TEXT_WIDTH_32;	// technically 32 characters, not sure here...
														    // is printing to multicolor even useful?
	vsetchar = vsetchar_base;
	return unblank;
}

void set_multicolor(unsigned char sprite_mode) {
    int x = set_multicolor_raw(sprite_mode);
    VDP_SET_REGISTER(VDP_REG_MODE1, x);
    VDP_REG1_KSCAN_MIRROR = x;
}
