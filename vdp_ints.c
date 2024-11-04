// VDP code for the TI-99/4A by Tursi
// You can copy this file and use it at will if it's useful

#include "vdp.h"
#include "f18a.h"
#include "sound.h"

#ifdef COLECO
#include <stdint.h>

// Coleco specific init code so that the nmi counts like the TI interrupt
// it also provides the user interrupt hook
// no other TI functionality is simulated (automotion, automusic, quit)
// but if needed, this is where it goes

// storage for VDP status byte
volatile unsigned char VDP_STATUS_MIRROR = 0;

// lock variable to prevent NMI from doing anything
// Z80 int is edge triggered, so it won't break us
// 0x80 = interrupt pending, 0x01 - interrupts enabled
// (This must be defined in the crt0.s)
//volatile unsigned char vdpLimi = 0;		// NO ints by default!

// address of user interrupt function
static void (*userint)() = 0;

// interrupt counter
volatile unsigned char VDP_INT_COUNTER = 0;

// used by vdpwaitvint - make certain it's reset
extern unsigned char gSaveIntCnt;

// May be called from true NMI or from VDP_INTERRUPT_ENABLE, depending on
// the flag setting when the true NMI fires.
void my_nmi() {
	// I think we're okay from races. There are only three conditions this is called:
	//
	// VDP_INTERRUPT_ENABLE - detects that vdpLimi&0x80 was set by the interrupt code.
	//						  Calls this code. But the interrupt line is still active,
	//						  so we can't retrigger until it's cleared by the VDPST read
	//						  below. At that time the vdpLimi is zeroed, and so we can't loop.
	//
	// nmi -				  detects that vdpLimi&0x01 is valid, and calls directly.
	//						  Again, the interrupt line is still active.
	//
	// maskable int (spinner)-detects that vdpLimi&0x80 was set by the interrupt code.
	//                        this one might be a little bit racey... still need
	//                        to think it all the way through...                        
	//
	// I think the edge cases are covered. Except if the user is manually reading VDPST,
	// then the state of vdpLimi could be out of sync with the real interrupt line, and cause
	// a double call. User apps should only read the mirror variable. Should I enforce that?

    VDP_CLEAR_VBLANK;           // release the VDP - we could instantly trigger again, but the vdpLimi is zeroed, so no loop
	VDP_INT_COUNTER++;			// count up the frames

	// the TI is running with ints off, so it won't retrigger in the
	// user code, even if it's slow. Our current process won't either because
	// the vdpLimi is set to 0.
	if (0 != userint) userint();

	// the TI interrupt would normally exit with the ints disabled
	// if it fired, so we will do the same here and not reset it.
}

// called automatically by crt0.S
void vdpinit() {
	volatile unsigned int x;
	
	// shut off the sound generator - if the cart skips the BIOS screen, this is needed.
	SOUND(0x9f);
	SOUND(0xbf);
	SOUND(0xdf);
	SOUND(0xff);
	
	// also silence and reset the AY sound chip in case it's present (if not present these
	// port writes should go nowhere)
    // note: turns out this is also important to work around a Phoenix AY powerup bug
    AY_REGISTER = AY_VOLA;
    AY_DATA_WRITE = 0x0;
    AY_REGISTER = AY_VOLB;
    AY_DATA_WRITE = 0x0;
    AY_REGISTER = AY_VOLC;
    AY_DATA_WRITE = 0x0;
    // To work around another Phoenix AY bug, make sure the tone C generator is 
    // running at an audible rate
    AY_REGISTER = AY_PERIODC_LOW;
    AY_DATA_WRITE = 0x10;

    // zero variables
    VDP_STATUS_MIRROR = 0;
    userint = (void (*)())0;
    VDP_INT_COUNTER = 1;
    gSaveIntCnt = 0;

	// interrupts off
	vdpLimi = 0;

	// before touching VDP, a brief delay. This gives time for the F18A to finish
	// initializing before we touch the VDP itself. This is needed on the Coleco if
	// you don't use the BIOS startup delay. This is roughly 200ms.
	x=60000;
	while (++x != 0) { }		// counts till we loop at 65536

    // reset the system and accomodate known alternate VDPs
    // First, reset then lock the F18A if any - this also turns off the screen
    reset_f18a();
    lock_f18a();

    VDP_STATUS_MIRROR = VDPST();	// init and clear any pending interrupt
}

// NOT atomic! Do NOT call with interrupts enabled!
void setUserIntHook(void (*hookfn)()) {
	userint = hookfn;
}

// NOT atomic! Do NOT call with interrupts enabled!
void clearUserIntHook() {
	userint = 0;
}
#endif

#ifdef TI99
// contains default init for 9938 regs 8-15 that make it more compatible
const unsigned char REG9938Init[8] = {
    0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

void vdpinit() {
	// shut off the sound generator
	SOUND(0x9f);
	SOUND(0xbf);
	SOUND(0xdf);
	SOUND(0xff);
	
	// also silence and reset the SID if present (if not present nothing will be mapped when we write)
	// we write the keyboard select to guarantee the SID blaster is mapped in
	MAP_SID_BLASTER;
	// mute all
	SIDBLASTER_MODEVOL = 0;
	// and TEST all the channels to reset them
	SIDBLASTER_CR1 = SIDBLASTER_CR_TEST;
	SIDBLASTER_CR2 = SIDBLASTER_CR_TEST;
	SIDBLASTER_CR3 = SIDBLASTER_CR_TEST;

    // reset interrupts on the CRU - note this also DISABLES peripherial interrupts (CRU bit 1)
    // only VDP interrupts are enabled. If you need peripherals, you need to set 1 to CRU bit 1
    __asm__ volatile("li r12,>2\n\tli r1,>0002\n\tldcr r1,15" : : : "r12","r1","cc");

    // zero variables
    VDP_INT_HOOK = (void (*)())0;
    VDP_INT_COUNTER = 1;

    // reset the system and accomodate known alternate VDPs
    // First, reset then lock the F18A if any - this also turns off the screen
    reset_f18a();
    lock_f18a();

    // now load some default values that make the 9938 happy
    for (int i=0; i<8; ++i) {
        VDP_SET_REGISTER(i+8, REG9938Init[i]);
    }

    // NOTE: a stock 9918 will be very confused, but your first call should
    // be one of the set_xxx calls to init a display mode

	VDP_STATUS_MIRROR = VDPST();	// init and clear any pending interrupt
}
	
// NOT atomic! Do NOT call with interrupts enabled!
void setUserIntHook(void (*hookfn)()) {
	VDP_INT_HOOK = hookfn;
}

// NOT atomic! Do NOT call with interrupts enabled!
void clearUserIntHook() {
	VDP_INT_HOOK = 0;
}
#endif

#ifdef GBA
#include <tursigb.h>
#include <GBASNPlay.h>
#include "string.h"
#include "f18a.h"

// GBA specific init code so that the vblank counts like the TI interrupt
// it also provides the user interrupt hook
// no other TI functionality is simulated (automotion, automusic, quit)
// but if needed, this is where it goes

// storage for VDP status byte
volatile unsigned char VDP_STATUS_MIRROR = 0;

// address of user interrupt function (vblank)
static void (*userint)() = 0;

// address of user interrupt function (anything else) - argument is REG_IF flags
static void (*otherint)(unsigned int) = 0;

// interrupt counter
volatile unsigned char VDP_INT_COUNTER = 0;

// used by vdpwaitvint - make certain it's reset
extern unsigned char gSaveIntCnt;

// May be called from true interrupt or from VDP_INTERRUPT_ENABLE, depending on
// the flag setting when the true interrupt fires.
// CRT0 expects it to be called InterruptProcess
void InterruptProcess() {
    unsigned int intType = REG_IF;

    // master disable ints
    REG_IME = 0;

    if (intType & INT_VBLANK) {
        VDP_CLEAR_VBLANK;           // release the VDP - we could instantly trigger again, but the interrupt is disabled
	    VDP_INT_COUNTER++;			// count up the frames

        // ints off, so it won't retrigger in the
	    // user code, even if it's slow.
    	if (0 != userint) userint();
    } 
    if (intType & INT_TIMER2) {
        // feed the audio fifos
        snupdateaudio();
    }
    if (intType & (~(INT_VBLANK|INT_TIMER2))) {
        // for all interrupts that are not vblank or timer2
        if (0 != otherint) otherint(intType);
    }

    // clear and acknowledge all interrupts
    *(volatile unsigned short *)0x3007FF8 |= intType;   // To BIOS
    REG_IF = intType;   // To Hardware

	// the TI interrupt would normally exit with the ints disabled
	// if it fired, so we will do the same here and not reset vblank
    // But everything else needs to be back on
    REG_IME = 1;
}

void gbavidinit() {
    // set up for mode 4, eventually we'll add scaling to fit the whole screen
    // but for now we'll deal with cropping once the draw is done
    // mode 4 is 240x160x8 bit, and there are two pages so we can page flip
    // No sprites, we're going to draw everything. Otherwise we can't really scale it.
    REG_DISPCNT = MODE4 | BG2_ENABLE;
    REG_BG2CNT = COLORS_256;
    memset((char*)BG_RAM_BASE, 0, 240*160);
    reset_f18a();   // mostly to get the palette loaded
}

extern void intrwrap();
void gbainit() {
    // master interrupts off
    REG_IME = 0;

    // setup vblank and SN emulator
    INT_VECTOR = intrwrap; // assembly wrapper for interrupt handler
    REG_DISPSTAT = VBLANK_IRQ;
    REG_IE = INT_TIMER2;     // TIMER2 triggers an audio reload
    
    // set up the GBA sound hardware - uses TIMER1 for frequency
    gbasninit();
    MUTE_SOUND();
    
    // set up the video to 8-bit color mode, we'll be emulating the F18A's 64 colors
    gbavidinit();

    // master interrupt enable
    REG_IME = 1;
}

// called automatically by crt0 code
// TODO: it's not automatic on gba cause I don't have crt0 source, so I fake it with wrap
void vdpinit() {
    // init the gba basic hardware
    gbainit();
	
	// shut off the emulated sound generator
	SOUND(0x9f);
	SOUND(0xbf);
	SOUND(0xdf);
	SOUND(0xff);

    // zero variables
    VDP_STATUS_MIRROR = 0;
    
    userint = (void (*)())0;
    otherint = (void (*)(unsigned int))0;
    VDP_INT_COUNTER = 1;
    gSaveIntCnt = 0;

	// interrupts off
    VDP_INT_DISABLE;

    // reset the system and accomodate known alternate VDPs
    // First, reset then lock the F18A if any - this also turns off the screen
    reset_f18a();
    lock_f18a();

    // init status and clear any pending interrupt
    VDP_STATUS_MIRROR = VDPST();
}

// make sure to link with --wrap=main
extern void __real_main();
void __wrap_main() {
    vdpinit();
    __real_main();
}

// NOT atomic! Do NOT call with interrupts enabled!
void setUserIntHook(void (*hookfn)()) {
	userint = hookfn;
}

// NOT atomic! Do NOT call with interrupts enabled!
void clearUserIntHook() {
	userint = 0;
}

// NOT atomic! Do NOT call with interrupts enabled!
void setOtherIntHook(void (*hookfn)(unsigned int)) {
	otherint = hookfn;
}

// NOT atomic! Do NOT call with interrupts enabled!
void clearOtherIntHook() {
	otherint = 0;
}

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

#endif
