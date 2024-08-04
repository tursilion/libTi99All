// F18A - by Tursi - public domain

#include <vdp.h>
#include <f18a.h>

// load an F18A palette from ptr (16-bit words, little endian)
// data format is 12-bit 0RGB color.
#ifndef GBA
void loadpal_f18a(const unsigned int *ptr, unsigned char first, unsigned char cnt) {
	VDP_SET_REGISTER(F18A_REG_DPM, F18A_DPM_ENABLE|F18A_DPM_INC|(first&0x3f));
	// Reg 47, value: 1100 0000, DPM = 1, AUTO INC = 1, palreg 0.       

	while (cnt-- > 0) {
#ifdef TI99
        // the TI GCC currently doesn't optimize the MSB access very well, so this saves two 8 bit shifts
        // and a handful of instructions. Even using pointers the C code is still 4 instructions long
        __asm__ volatile("movb *%0+,@>8C00\n\tmovb *%0+,@>8C00" : : "r"(ptr) : "cc");
#else
		VDPWD = ptr[0]>>8;
		VDPWD = ptr[0]&0xff;
		ptr++;
#endif
	}

	VDP_SET_REGISTER(F18A_REG_DPM, 0x00);	// Turn off the DPM mode
}
#endif

#ifdef GBA
// fake it more directly. note that on the GBA ints are 32-bit, not 16-bit like
// the library expects, so things may be a bit weird.
// F18A paletes are 12-bit 0000 RRRR GGGG BBBB, GBA palettes are 15-bit 0BBB BBGG GGGR RRRR
void loadpal_f18a(const unsigned int *ptr, unsigned char first, unsigned char cnt) {
    for (int idx=0; idx<cnt; ++idx) {
        // we'll convert the palette to GBA colors - and may later work out where to store them
        // F18A supports numerous palette mode, but I need to support 8 palettes in 8 color mode
        // GBA supports 16 palettes in 16 color mode, but since we are rendering ourselves we
        // can probably just make our own palette rules
        unsigned int val = *(ptr++);
        // extract RGB and extend to 5 bits, setting the LSB cause the GBA screen is dark anyway
        unsigned short r = ((val&0x0f00)>>8)|0x0001;
        unsigned short g = ((val&0x00f0)<<2)|0x0020;
        unsigned short b = ((val&0x000f)<<11)|0x0400;
        unsigned short final = (r|g|b)&0xffff;
        // write to both background and sprite palettes, though probably don't care about sprites...
        *((unsigned short*)(BG_PALETTE+(first+idx)*2)) = final;
        *((unsigned short*)(SPR_PALETTE+(first+idx)*2)) = final;
    }
}
#endif
