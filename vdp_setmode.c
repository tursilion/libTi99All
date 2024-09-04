// VDP code for the TI-99/4A by Tursi
// This is the generic code that all include.
// You can copy this file and use it at will if it's useful

// global pointers for all to enjoy - make sure the screen setup code updates them!
// assumptions here are for E/A environment, they may not be accurate and your
// program should NOT trust them
unsigned int gImage = 0x0000;		// SIT, Register 2 * 0x400
unsigned int gColor = 0x0380;		// CR,  Register 3 * 0x40
unsigned int gPattern = 0x0800;		// PDT, Register 4 * 0x800
unsigned int gSprite = 0x0300;		// SAL, Register 5 * 0x80
unsigned int gSpritePat = 0x0000;	// SDT, Register 6 * 0x800

#ifdef GBA
#include <vdp.h>
#include <tursigb.h>

// put the main VDP RAM in 256k external, otherwise it's half our available memory
// we're way faster than coleco or TI, so it won't hurt our performance.
unsigned char vdp_ram[16384] DATA_IN_EWRAM;	// no need for GPU RAM, the CPU can't touch it
unsigned char vdp_reg[64];		// to cover F18A
unsigned int  vdp_addr;			// vdp address
unsigned char vdp_prefetch;		// vdp prefetch byte
unsigned char vdp_flipflop;		// vdp address flipflop
volatile unsigned char vdp_status;        // for vertical blank only, set by interrupt
void gbaRender();

unsigned char gbaVDPRD() {
	unsigned char ret = vdp_prefetch;
	vdp_prefetch = vdp_ram[vdp_addr];
	++vdp_addr;
	vdp_addr &= 0x3fff;
	vdp_flipflop = 0;
	return ret;
}
unsigned char gbaVDPST() {
    unsigned char ret = vdp_status;
    if (REG_IF & INT_VBLANK) ret |= VDP_ST_INT;
    vdp_status = 0;
	vdp_flipflop = 0;
    REG_IF = INT_VBLANK;    // acknowledge interrupt to hardware
    *(volatile unsigned short *)0x3007FF8 |= INT_VBLANK;    // acknowledge to BIOS
    return ret;
}
// check interrupt without clearing it
unsigned char gbaVDPSTCRU() {
    unsigned char ret = vdp_status;
    if (REG_IF & INT_VBLANK) ret |= VDP_ST_INT;
    return ret;
}
void gbaVDPWA(unsigned char x) {
	if (vdp_flipflop) {
		vdp_addr = (vdp_addr & 0xff) | ((x & 0x3f) << 8);
		if (x & 0x80) {
			// register write
			int reg = x & 0x3f;
			vdp_reg[reg] = vdp_addr & 0xff;
			if (reg == 1) {
                // if enable bit is set, draw screen. This is a temp hack, it's too slow to be realtime
                gbaRender();
            }
		}
		if ((x & 0xc0) == 0) {
			// prefetch
			vdp_prefetch = vdp_ram[vdp_addr];
			++vdp_addr;
			vdp_addr &= 0x3fff;
		}
		vdp_flipflop = 0;
	} else {
		vdp_addr = (vdp_addr & 0xff00) | x;
		vdp_flipflop = 1;
	}
}
void gbaVDPWD(unsigned char x) {
    vdp_prefetch = x;
    vdp_ram[vdp_addr] = x;
	++vdp_addr;
    vdp_addr &= 0x3fff;
    vdp_flipflop = 0;
}

// render the TI screen on the GBA screen - this is deliberately very limited
// ultimately the intent is to do some scaling
extern unsigned char vdp_ram[16384] DATA_IN_EWRAM;
extern unsigned char vdp_reg[64];
void gbaRender() {
    unsigned short x,y;     // position on GBA screen
    unsigned short tx,ty;   // calculated position on TI screen (for scaling)
    unsigned short px,py;   // offset inside the cell
    
    for (x=0; x<240; x++) {
        tx=(x*273)>>8;
        for (y=0; y<160; y++) {
            ty=(y*307)>>8;
            
            unsigned short sit = (ty/8)*32+(tx/8)+gImage;
            unsigned short pat = vdp_ram[sit]*8+gPattern;
            
            px=7-(tx%8);
            py=ty%8;
            
            unsigned char p = vdp_ram[pat+py];
            unsigned char mask = 1<<px;
            
            if (p&mask) {
                unsigned short fg = (vdp_reg[7]&0xf0)>>4;
                if (fg == 0) fg = 0xf;
                fg=14;
                unsigned int adr = BG_RAM_BASE + x + y*240;
                if (adr&1) {
                    // 16 bit write required
                    --adr;
                    *((unsigned short*)adr) = ((*((unsigned short*)adr))&0x00ff) | (fg<<8);
                } else {
                    *((unsigned short*)adr) = ((*((unsigned short*)adr))&0xff00) | fg;
                }
            } else {
                unsigned short bg = vdp_reg[7]&0x0f;
                unsigned int adr = BG_RAM_BASE + x + y*240;
                bg =1;
                if (adr&1) {
                    // 16 bit write required
                    --adr;
                    *((unsigned short*)adr) = ((*((unsigned short*)adr))&0x00ff) | (bg<<8);
                } else {
                    *((unsigned short*)adr) = ((*((unsigned short*)adr))&0xff00) | bg;
                }
            }
        }
    }
}

#endif
