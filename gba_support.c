// GBA specific support functions
#include <tursigb.h>
#include <GBASNPlay.h>
#include "vdp.h"

// make sure to link with --wrap=main
extern void __real_main();
extern void vdpinit();
void __wrap_main() {
    vdpinit();
    __real_main();
}

// They aren't supposed to because of priority, but long DMAs are
// breaking the audio playback. So we limit each copy to bursts to
// allow the update interrupt to fire.
#define COPY_CYCLES 128
void fastcopy16(u32 pDest, u32 pSrc, u32 nCount) {
    u16 cnt;
    nCount >>= 1;

    while (nCount > 0) {
        cnt = (nCount > COPY_CYCLES ? COPY_CYCLES : nCount);
        REG_DMA3SAD=(pSrc);
        REG_DMA3DAD=(pDest);
        REG_DMA3CNT_L=cnt;
        REG_DMA3CNT_H=HALF_WORD_DMA|ENABLE_DMA|START_IMMEDIATELY;
        nCount -= cnt;
        pSrc += cnt<<1;
        pDest += cnt<<1;
    }
}
void fastset16(u32 pDest, u16 value, u32 nCount) {
    u16 cnt;
    volatile u16 dummy=(value);
    nCount >>= 1;

    while (nCount > 0) {
        cnt = (nCount > COPY_CYCLES ? COPY_CYCLES : nCount);
        REG_DMA3SAD=(u32)&dummy;
        REG_DMA3DAD=(pDest);
        REG_DMA3CNT_L=cnt;
        REG_DMA3CNT_H=HALF_WORD_DMA|ENABLE_DMA|START_IMMEDIATELY|SRC_REG_SAME;
        nCount -= cnt;
        pDest += cnt<<1;
    }
}

void fastcopy(u32 pDest, u32 pSrc, u32 nCount) {
    u16 cnt;
    nCount >>= 2;

    while (nCount > 0) {
        cnt = (nCount > COPY_CYCLES ? COPY_CYCLES : nCount);
        REG_DMA3SAD=(pSrc);
        REG_DMA3DAD=(pDest);
        REG_DMA3CNT_L=cnt;
        REG_DMA3CNT_H=WORD_DMA|ENABLE_DMA|START_IMMEDIATELY;
        nCount -= cnt;
        pSrc += cnt<<2;
        pDest += cnt<<2;
    }
}


void fastset(u32 pDest, u32 value, u32 nCount) {
    u16 cnt;
    volatile u32 dummy=(value);
    nCount >>= 2;
    while (nCount > 0) {
        cnt = (nCount > COPY_CYCLES ? COPY_CYCLES : nCount);
        REG_DMA3SAD=(u32)&dummy;
        REG_DMA3DAD=(pDest);
        REG_DMA3CNT_L=cnt;
        REG_DMA3CNT_H=WORD_DMA|ENABLE_DMA|START_IMMEDIATELY|SRC_REG_SAME;
        nCount -= cnt;
        pDest += cnt<<2;
    }
}

// TI video emulation, hacky and dubious ;)

// put the main VDP RAM in 256k external, otherwise it's half our available memory
// we're way faster than coleco or TI, so it won't hurt our performance.
unsigned char vdp_ram[16384] DATA_IN_EWRAM;	// no need for GPU RAM, the CPU can't touch it
unsigned char vdp_reg[64];		// to cover F18A
unsigned int  vdp_addr;			// vdp address
unsigned char vdp_prefetch;		// vdp prefetch byte
unsigned char vdp_flipflop;		// vdp address flipflop
volatile unsigned char vdp_status;        // for vertical blank only, set by interrupt
static short vdp_dirty = 0;
static short vdp_autorender = 0;
void gbaRender();
void gbaRenderScaled();

void setGBAAutoRender(unsigned short mode) {
    vdp_autorender = mode;
}

unsigned char CODE_IN_IWRAM gbaVDPRD() {
	unsigned char ret = vdp_prefetch;
	vdp_prefetch = vdp_ram[vdp_addr];
	++vdp_addr;
	vdp_addr &= 0x3fff;
	vdp_flipflop = 0;
	return ret;
}
unsigned char CODE_IN_IWRAM gbaVDPST() {
    unsigned char ret = vdp_status;
    if (REG_IF & INT_VBLANK) ret |= VDP_ST_INT;
    vdp_status = 0;
	vdp_flipflop = 0;
    REG_IF = INT_VBLANK;    // acknowledge interrupt to hardware
    *(volatile unsigned short *)0x3007FF8 |= INT_VBLANK;    // acknowledge to BIOS
    if ((vdp_dirty) && (vdp_autorender)) {
        if (vdp_reg[VDP_REG_MODE1] & VDP_MODE1_UNBLANK) {
            if (vdp_autorender == 1) {
                gbaRender();
            } else {
                gbaRenderScaled();
            }
        }
    }
    return ret;
}
// check interrupt without clearing it
unsigned char gbaVDPSTCRU() {
    unsigned char ret = vdp_status;
    if (REG_IF & INT_VBLANK) ret |= VDP_ST_INT;
    return ret;
}
void CODE_IN_IWRAM gbaVDPWA(unsigned char x) {
	if (vdp_flipflop) {
		vdp_addr = (vdp_addr & 0xff) | ((x & 0x3f) << 8);
		if (x & 0x80) {
			// register write
			int reg = x & 0x3f;
			vdp_reg[reg] = vdp_addr & 0xff;
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
void CODE_IN_IWRAM gbaVDPWD(unsigned char x) {
    vdp_prefetch = x;
    vdp_ram[vdp_addr] = x;
	++vdp_addr;
    vdp_addr &= 0x3fff;
    vdp_flipflop = 0;
    vdp_dirty = 1;
}

// render the TI screen on the GBA screen - this is deliberately very limited
// ultimately the intent is to do some scaling
void CODE_IN_IWRAM gbaRender() {
    unsigned short tx,ty;   // calculated position on TI screen
    unsigned short px,py;   // offset inside the cell
    
    if (vdp_reg[VDP_REG_MODE0] & VDP_MODE0_BITMAP) {
        for (ty=0; ty<160; ty++) {
            for (tx=0; tx<240; tx+=2) {
                px=7-(tx%8);
                py=ty%8;

                unsigned short sit = (ty/8)*32+(tx/8)+gImage;
                unsigned short patbase = (ty/64)*0x800+vdp_ram[sit]*8+py;
                unsigned char p = vdp_ram[patbase+gPattern];
                unsigned char mask = 1<<px;
                unsigned int adr = BG_RAM_BASE + tx + ty*240;
                unsigned short pixdat = 0;
                unsigned short screen = vdp_reg[7]&0xf;
                unsigned short color = vdp_ram[patbase+gColor];
                
                if (p&mask) {
                    unsigned short fg = color>>4;
                    if (fg == 0) fg = screen;
                    pixdat = fg;
                } else {
                    unsigned short bg = color&0xf;
                    if (bg == 0) bg = screen;
                    pixdat = bg;
                }

                // second pixel
                mask >>= 1;
                if (p&mask) {
                    unsigned short fg = color>>4;
                    if (fg == 0) fg = screen;
                    pixdat |= (fg<<8);
                } else {
                    unsigned short bg = color&0xf;
                    if (bg == 0) bg = screen;
                    pixdat |= (bg<<8);
                }
                
                *((unsigned short*)adr) = pixdat;
            }
        }
    } else {
        for (ty=0; ty<160; ty++) {
            for (tx=0; tx<240; tx+=2) {
                px=7-(tx%8);
                py=ty%8;

                unsigned short sit = (ty/8)*32+(tx/8)+gImage;
                unsigned short patbase = vdp_ram[sit]*8+py;
                unsigned char p = vdp_ram[patbase+gPattern];
                unsigned char mask = 1<<px;
                unsigned int adr = BG_RAM_BASE + tx + ty*240;
                unsigned short pixdat = 0;
                unsigned short screen = vdp_reg[7]&0xf;
                unsigned short color = vdp_ram[vdp_ram[sit]/8+gColor];
                
                if (p&mask) {
                    unsigned short fg = color>>4;
                    if (fg == 0) fg = screen;
                    pixdat = fg;
                } else {
                    unsigned short bg = color&0xf;
                    if (bg == 0) bg = screen;
                    pixdat = bg;
                }

                // second pixel
                mask >>= 1;
                if (p&mask) {
                    unsigned short fg = color>>4;
                    if (fg == 0) fg = screen;
                    pixdat |= (fg<<8);
                } else {
                    unsigned short bg = color&0xf;
                    if (bg == 0) bg = screen;
                    pixdat |= (bg<<8);
                }
                
                *((unsigned short*)adr) = pixdat;
            }
        }
     }
    
    vdp_dirty=0;
}
void CODE_IN_IWRAM gbaRenderScaled() {
    unsigned short x,y;     // position on GBA screen
    unsigned short tx,ty;   // calculated position on TI screen (for scaling)
    unsigned short px,py;   // offset inside the cell
    
    if (vdp_reg[VDP_REG_MODE0] & VDP_MODE0_BITMAP) {
        for (x=0; x<240; x+=2) {
            tx=(x*273)>>8;
            for (y=0; y<160; y++) {
                ty=(y*307)>>8;
                
                unsigned short sit = (ty/8)*32+(tx/8)+gImage;
                unsigned short pat = (ty/64)*0x800+vdp_ram[sit]*8+gPattern;
                
                px=7-(tx%8);
                py=ty%8;
                
                unsigned char p = vdp_ram[pat+py];
                unsigned char mask = 1<<px;
                unsigned int adr = BG_RAM_BASE + x + y*240;
                unsigned short pixdat = 0;
                unsigned short screen = vdp_reg[7]&0xf;
                unsigned short color = vdp_ram[(ty/64)*0x800+vdp_ram[sit]*8+gColor+py];
                
                if (p&mask) {
                    unsigned short fg = color>>4;
                    if (fg == 0) fg = screen;
                    pixdat = fg;
                } else {
                    unsigned short bg = color&0xf;
                    if (bg == 0) bg = screen;
                    pixdat = bg;
                }

                // second pixel
                mask >>= 1;
                if (p&mask) {
                    unsigned short fg = color>>4;
                    if (fg == 0) fg = screen;
                    pixdat |= (fg<<8);
                } else {
                    unsigned short bg = color&0xf;
                    if (bg == 0) bg = screen;
                    pixdat |= (bg<<8);
                }
                
                *((unsigned short*)adr) = pixdat;
            }
        }
    } else {
        for (x=0; x<240; x+=2) {
            tx=(x*273)>>8;
            for (y=0; y<160; y++) {
                ty=(y*307)>>8;
                
                unsigned short sit = (ty/8)*32+(tx/8)+gImage;
                unsigned short pat = vdp_ram[sit]*8+gPattern;
                
                px=7-(tx%8);
                py=ty%8;
                
                unsigned char p = vdp_ram[pat+py];
                unsigned char mask = 1<<px;
                unsigned int adr = BG_RAM_BASE + x + y*240;
                unsigned short pixdat = 0;
                unsigned short screen = vdp_reg[7]&0xf;
                unsigned short color = vdp_ram[vdp_ram[sit]/8+gColor];
                
                if (p&mask) {
                    unsigned short fg = color>>4;
                    if (fg == 0) fg = screen;
                    pixdat = fg;
                } else {
                    unsigned short bg = color&0xf;
                    if (bg == 0) bg = screen;
                    pixdat = bg;
                }

                // second pixel
                mask >>= 1;
                if (p&mask) {
                    unsigned short fg = color>>4;
                    if (fg == 0) fg = screen;
                    pixdat |= (fg<<8);
                } else {
                    unsigned short bg = color&0xf;
                    if (bg == 0) bg = screen;
                    pixdat |= (bg<<8);
                }
                
                *((unsigned short*)adr) = pixdat;
            }
        }
     }
    
    vdp_dirty=0;
}


