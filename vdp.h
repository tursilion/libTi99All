// TODO: add the rle unpack function for Convert9918a images
// TODO: the crt0 needs to include 4 more bytes so that the return from main() works on any page (What did I mean here?)

// VDP header for the TI-99/4A and Coleco by Tursi aka Mike Brent
// 8/3/2024
// This code and library released into the Public Domain
// You can copy this file and use it at will ;)

#ifndef VDP_H
#define VDP_H

#ifndef COLECO
#ifndef TI99
#ifndef GBA
// GBA is an unofficial target with a big gotcha - ints are suddenly 32-bit. So all pointer math MAY be wrong!
// Be careful about types.
#error Make sure to define either COLECO (for Coleco or SMS or MSX) or TI99
#endif
#endif
#endif

#ifdef GBA
#include "tursigb.h"
#endif

// TODO: we can break out the various subsystems (hardware, text, bitmap, etc)

//*********************
// VDP access ports
//*********************
#ifdef TI99

// Read Data
#define VDPRD()	*((volatile unsigned char*)0x8800)
// Read Status
#define VDPST()	*((volatile unsigned char*)0x8802)
// Write Address/Register
#define VDPWA(x)	*((volatile unsigned char*)0x8C02)=(x)
// Write Data
#define VDPWD(x)	*((volatile unsigned char*)0x8C00)=(x)

#endif
#ifdef COLECO
// SMS uses the same ports
// Read Data
volatile __sfr __at 0xbe pVDPRD;
// Read Status
volatile __sfr __at 0xbf pVDPST;
// Write Address/Register
volatile __sfr __at 0xbf pVDPWA;
// Write Data
volatile __sfr __at 0xbe pVDPWD;
#define VDPRD() pVDPRD
#define VDPST() pVDPST
#define VDPWA(x) pVDPWA=(x)
#define VDPWD(x) pVDPWD=(x)
#endif

#ifdef GBA
// these have to call out to functions
extern unsigned char CODE_IN_IWRAM gbaVDPRD();
extern unsigned char CODE_IN_IWRAM gbaVDPST();
extern void CODE_IN_IWRAM gbaVDPWA(unsigned char x);
extern void CODE_IN_IWRAM gbaVDPWD(unsigned char x);

#define VDPRD() gbaVDPRD()
#define VDPST() gbaVDPST()
#define VDPWA(x) gbaVDPWA(x)
#define VDPWD(x) gbaVDPWD(x)

extern volatile unsigned char vdp_status;        // for vertical blank only, set by interrupt
#endif


//*********************
// Inline VDP helpers
//*********************
#ifdef TI99
// the TI only needs a delay between setting the read address and reading,
// and only with certain instruction combinations in fast memory. SO, this
// function does not delay here.
inline void VDP_SAFE_DELAY() {	}
#endif
#ifdef COLECO
// safe delay between VDP accesses - each NOP is 4 on the Coleco (5 on MSX apparently! Watch out!)
// The VDP needs 29 cycles between accesses, roughly. The code generated often is slow enough, if variables are used instead
// of constants. If you need the speed, you could hand-optimize the asm knowing this. ;)
inline void VDP_SAFE_DELAY(void) {	
// still tuning this... from online comments:
// Actually, Z80 frequency is 3.579545 MHz (MSX) so the math comes to 28.63, roughly 29 T-states.
// You must include the OUT instruction, so it means we need 18 additional T-states before the next 
// VRAM access. That's a lot of unused CPU time, but if it is needed, so be it.
// When I did the math, I got this using OUTI:
// OUTI is 16 cycles or 4.47uS
// NOP is 4 cycles or 1.12uS
// to get to 8uS, we'd need 4 NOPS (although 3 would work 90% of the time... 0.12uS difference! So close! Maybe another instruction?)
// However, OUTI/OUTD are the slower instructions, the fastest OUT is OUT (p),A, which is 11 cycles (3.073 uS)
// To get to 8uS there, we do need 5 NOPs (although the last cycle is only off by 0.44uS)
#ifndef SMS
__asm
	nop
	nop
	nop
	nop
	nop
__endasm;
#endif
}
#endif
#ifdef GBA
// no delays needed
inline void VDP_SAFE_DELAY() {	}
#endif

// TODO: need some hardware testing to understand the VDP limits
// Can you write the address register full speed? Is it /only/ VRAM access that needs the delay?
// If true, these sequences are safe. I am quite sure they are. 
//		ADR,ADR_WR,DATA,>DELAY<
//		ADR,ADR_RD,>DELAY<,DATA
// I've taken out the address write safety for that reason, we will see! Even the MSX guys aren't sure.

#ifdef TI99
// the TI GCC currently doesn't optimize the MSB access very well, so this saves two 8 bit shifts
// and a handful of instructions. Even using pointers the C code is still 4 instructions long

// Set VDP address for read (no bit added)
inline void VDP_SET_ADDRESS(unsigned int x)     { __asm__  volatile ( "swpb %0\n\tmovb %0,@>8c02\n\tswpb %0\n\tmovb %0,@>8c02" : : "r"(x) : "cc"); }

// Set VDP address for write (adds 0x4000 bit)
inline void VDP_SET_ADDRESS_WRITE(unsigned int x) { __asm__  volatile ( "mov %0,r0\n\tswpb r0\n\tmovb r0,@>8c02\n\tswpb r0\n\tori r0,>4000\n\tmovb r0,@>8c02" : : "r"(x) : "cc"); }

#else
// Set VDP address for read (no bit added)
inline void VDP_SET_ADDRESS(unsigned int x)						{	VDPWA( ((x)&0xff) ); VDPWA( ((x)>>8) );			}

// Set VDP address for write (adds 0x4000 bit)
inline void VDP_SET_ADDRESS_WRITE(unsigned int x)					{	VDPWA( ((x)&0xff) ); VDPWA( (((x|0x4000)>>8)) );	}
#endif

// Set VDP write-only register 'r' to value 'v'
inline void VDP_SET_REGISTER(unsigned char r, unsigned char v)	{	VDPWA(v); VDPWA(0x80|(r));				}

// get a screen offset for 32x24 graphics mode
inline int VDP_SCREEN_POS(unsigned int r, unsigned int c)			{	return (((r)<<5)+(c));						}

// get a screen offset for 40x24 text mode
inline int VDP_SCREEN_TEXT(unsigned int r, unsigned int c)		{	return (((r)<<5)+((r)<<3)+(c));				}

// get a screen offset for 80x24 text mode
inline int VDP_SCREEN_TEXT80(unsigned int r, unsigned int c)	    {	return (((r)<<6)+((r)<<4)+(c));				}

// get a screen offset for 64x24 graphics mode
// NOTE: This is not a real VDP address, it's a virtual address that vdpchar64 understands
inline int VDP_SCREEN_TEXT64(unsigned int r, unsigned int c)		{	return (((r)<<6)+(c));						}

// Note getscreenoffset(x,y) can be used for a mode-independent lookup

//*********************
// VDP Console interrupt control
//*********************

#ifdef TI99
// Interrupt counter - incremented each interrupt
#define VDP_INT_COUNTER			*((volatile unsigned char*)0x8379)

// Copy of the VDP status byte. If VDP interrupts are enabled, you should read
// this value, instead of reading it directly from the VDP.
#define VDP_STATUS_MIRROR		*((volatile unsigned char*)0x837b)

// This flag byte allows you to turn parts of the console interrupt handler on and off
// See the VDP_INT_CTRL_* defines below
#define VDP_INT_CTRL			*((volatile unsigned char*)0x83c2)

// Address of a user-defined function to call during the vertical interrupt handler,
// set to 0x0000 if not using
#define VDP_INT_HOOK			*((volatile void**)0x83c4)

// If using KSCAN, you must put a copy of VDP register 1 (returned by the 'set' functions)
// at this address, otherwise the first time a key is pressed, the value will be overwritten.
// The console uses this to undo the screen timeout blanking.
#define VDP_REG1_KSCAN_MIRROR	*((volatile unsigned char*)0x83d4)

// The console counts up the screen blank timeout here. You can reset it by writing 0,
// or prevent it from ever triggering by writing an odd number. Each interrupt, it is
// incremented by 2, and when the value reaches 0x0000, the screen will blank by setting
// the blanking bit in VDP register 1. This value is reset on keypress in KSCAN.
#define VDP_SCREEN_TIMEOUT		*((volatile unsigned int*)0x83d6)
#endif

#ifdef COLECO
// Interrupt counter - incremented each interrupt
// WARNING: NEVER WRITE TO THIS VALUE. READ ONLY.
extern volatile unsigned char VDP_INT_COUNTER;

// Copy of the VDP status byte. If VDP interrupts are enabled, you should read
// this value, instead of reading it directly from the VDP.
extern volatile unsigned char VDP_STATUS_MIRROR;

// This flag byte allows you to turn parts of the console interrupt handler on and off
// Has no meaning on Coleco.
#define VDP_INT_CTRL			*((volatile unsigned char*)0)

#endif

#ifdef GBA
// Interrupt counter - incremented each interrupt
// WARNING: NEVER WRITE TO THIS VALUE. READ ONLY.
extern volatile unsigned char VDP_INT_COUNTER;

// Copy of the VDP status byte. No reset if you read this.
extern volatile unsigned char VDP_STATUS_MIRROR;

// This flag byte allows you to turn parts of the console interrupt handler on and off
// Has no meaning on GBA.
#define VDP_INT_CTRL			*((volatile unsigned char*)0)
#endif

// These values are flags for the interrupt control 
	// disable all processing (screen timeout and user interrupt are still processed)
	#define VDP_INT_CTRL_DISABLE_ALL		0x80
	// disable sprite motion
	#define VDP_INT_CTRL_DISABLE_SPRITES	0x40
	// disable sound list processing
	#define VDP_INT_CTRL_DISABLE_SOUND		0x20
	// disable QUIT key testing
	#define VDP_INT_CTRL_DISABLE_QUIT		0x10

// TODO: not just the wait for vblank, we need a safe way to CHECK interrupt. Coleco can use vdpLimi&0x80, TI needs to check CRU bit 2

#ifdef TI99
// wait for a vblank (interrupts disabled - will work unreliably if enabled)
// call vdpwaitvint() instead if you want to keep running the console interrupt
// TODO: VDPSTCRU to return true if bit is set
#define VDP_WAIT_VBLANK_CRU	  __asm__ volatile ( "clr r12\nvdp%=:\n\ttb 2\n\tjeq vdp%=" : : : "r12","cc" );
#define VDP_CLEAR_VBLANK { VDP_INT_DISABLE; VDP_STATUS_MIRROR = VDPST(); }

// we enable interrupts via the CPU instruction, not the VDP itself, because it's faster
// Note that on the TI interrupts DISABLED is the default state
#define VDP_INT_ENABLE			__asm__ volatile ("LIMI 2")
#define VDP_INT_DISABLE			__asm__ volatile ("LIMI 0")

// If using KSCAN, you must put a copy of VDP register 1 (returned by the 'set' functions)
// at this address, otherwise the first time a key is pressed, the value will be overwritten.
// The console uses this to undo the screen timeout blanking.
#define FIX_KSCAN(x) VDP_REG1_KSCAN_MIRROR=(x);

#endif
#ifdef COLECO
// wait for a vblank (interrupts disabled - will work unreliably if enabled)
// there's no CRU on the Coleco, of course... but for compatibility..
extern volatile unsigned char vdpLimi;
#define VDPSTCRU() (vdpLimi&0x80)
#define VDP_WAIT_VBLANK_CRU	  while (VDPSTCRU == 0) { }

#define VDP_CLEAR_VBLANK { vdpLimi = 0; VDP_STATUS_MIRROR = VDPST(); }    // has to force to 0 to clear any pending unprocessed int

// we enable interrupts via a mask byte, as Coleco ints are NMI
// Note that the enable therefore needs to check a flag!
// Note that on the TI interrupts DISABLED is the default state
// on enable, we make sure we are disabled before entering the NMI, just in case we race
// with another one. All the assumptions in the code assume that ints are disabled on entry
// to prevent double-call.
#define VDP_INT_ENABLE			{ if (vdpLimi&0x80) { vdpLimi=0; my_nmi(); } __asm__ ("\tpush hl\n\tld hl,#_vdpLimi\n\tset 0,(hl)\n\tpop hl"); }
#define VDP_INT_DISABLE			{ __asm__ ("\tpush hl\n\tld hl,#_vdpLimi\n\tres 0,(hl)\n\tpop hl"); }
	
// this might have no value... we'll see
// If using KSCAN, you must put a copy of VDP register 1 (returned by the 'set' functions)
// at this address, otherwise the first time a key is pressed, the value will be overwritten.
// The console uses this to undo the screen timeout blanking.
#define VDP_REG1_KSCAN_MIRROR	*((volatile unsigned char*)0)

// If using KSCAN, you must put a copy of VDP register 1 (returned by the 'set' functions)
// at this address, otherwise the first time a key is pressed, the value will be overwritten.
// The console uses this to undo the screen timeout blanking. (not needed on Coleco)
#define FIX_KSCAN(x)
	
#endif
#ifdef GBA
// wait for a vblank 
// there's no CRU on the GBA, of course... but for compatibility..
// I thought this spin is hard on the battery... but when I tried putting the CPU to sleep
// in Cool Herders it didn't seem to help battery life at all.
#define VDPSTCRU gbaVDPSTCRU
extern unsigned char gbaVDPSTCRU();
#define VDP_WAIT_VBLANK_CRU	  while ((VDPSTCRU() & VDP_ST_INT)==0) { } 

// clear any pending interrupt
#define VDP_CLEAR_VBLANK        { VDP_STATUS_MIRROR = VDPST(); }

// interrupts on and off in hardware - it will handle pending interrupts just fine
// or does it? I'm not so sure about that...
#define VDP_INT_ENABLE			{ REG_IE |= INT_VBLANK; REG_DISPSTAT |= VBLANK_IRQ;}
#define VDP_INT_DISABLE			{ REG_IE &= ~INT_VBLANK; REG_DISPSTAT &= ~VBLANK_IRQ; }
	
// this might have no value... we'll see
// If using KSCAN, you must put a copy of VDP register 1 (returned by the 'set' functions)
// at this address, otherwise the first time a key is pressed, the value will be overwritten.
// The console uses this to undo the screen timeout blanking. Not needed on GBA.
#define VDP_REG1_KSCAN_MIRROR	*((volatile unsigned char*)0)

// If using KSCAN, you must put a copy of VDP register 1 (returned by the 'set' functions)
// at this address, otherwise the first time a key is pressed, the value will be overwritten.
// The console uses this to undo the screen timeout blanking. (not needed on GBA)
#define FIX_KSCAN(x)
	
// GBA auto-render calls gbaRender to draw the screen if VDP RAM is dirty when
// calling gbaVDPST()
extern void setGBAAutoRender(unsigned short mode);
#define GBA_AUTORENDER_NONE 0
#define GBA_AUTORENDER_FULL 1
#define GBA_AUTORENDER_SCALE 2

// force a manual redraw, not needed if you set autorender
extern void gbaRender();
	
#endif

#define VDP_INT_POLL {	\
	VDP_INT_ENABLE;		\
	VDP_INT_DISABLE; }

//*********************
// Register settings
//*********************

// Bitmasks for the status register
#define VDP_ST_INT				(unsigned char)0x80		// interrupt ready
#define VDP_ST_5SP				(unsigned char)0x40		// 5 sprites-on-a-line
#define VDP_ST_COINC			(unsigned char)0x20		// sprite coincidence
#define VDP_ST_MASK				(unsigned char)0x1f		// mask for the 5 bits that indicate the fifth sprite on a line

// these are the actual write-only register indexes
#define VDP_REG_MODE0			(unsigned char)0x00		// mode register 0
#define VDP_REG_MODE1			(unsigned char)0x01		// mode register 1
#define VDP_REG_SIT				(unsigned char)0x02		// screen image table address (this value times 0x0400)
#define VDP_REG_CT				(unsigned char)0x03		// color table address (this value times 0x0040)
#define VDP_REG_PDT				(unsigned char)0x04		// pattern descriptor table address (this value times 0x0800)
#define VDP_REG_SAL				(unsigned char)0x05		// sprite attribute list address (this value times 0x0080)
#define VDP_REG_SDT				(unsigned char)0x06		// sprite descriptor table address (this value times 0x0800)
#define VDP_REG_COL				(unsigned char)0x07		// screen color (most significant nibble - foreground in text, least significant nibble - background in all modes)

// settings for mode register 0
#define VDP_MODE0_BITMAP		(unsigned char)0x02		// set bitmap mode
#define VDP_MODE0_EXTVID		(unsigned char)0x01		// enable external video (not connected on TI-99/4A)
#define VDP_MODE0_80COL         (unsigned char)0x04     // enable 9938/F18A 80 column

// settings for mode register 1
#define VDP_MODE1_16K			(unsigned char)0x80		// set 16k mode (4k mode if cleared)
#define VDP_MODE1_UNBLANK		(unsigned char)0x40		// set to enable display, clear to blank it
#define VDP_MODE1_INT			(unsigned char)0x20		// enable VDP interrupts
#define VDP_MODE1_TEXT			(unsigned char)0x10		// set text mode
#define VDP_MODE1_MULTI			(unsigned char)0x08		// set multicolor mode
#define VDP_MODE1_SPRMODE16x16	(unsigned char)0x02		// set 16x16 sprites
#define VDP_MODE1_SPRMAG		(unsigned char)0x01		// set magnified sprites (2x2 pixels) 

// sprite modes for the mode set functions
#define VDP_SPR_8x8				(unsigned char)0x00
#define	VDP_SPR_8x8MAG			(VDP_MODE1_SPRMAG)
#define VDP_SPR_16x16			(VDP_MODE1_SPRMODE16x16)
#define VDP_SPR_16x16MAG		(VDP_MODE1_SPRMODE16x16 | VDP_MODE1_SPRMAG)

// VDP colors
#define COLOR_TRANS				(unsigned char)0x00
#define COLOR_BLACK				(unsigned char)0x01
#define COLOR_MEDGREEN			(unsigned char)0x02
#define COLOR_LTGREEN			(unsigned char)0x03
#define COLOR_DKBLUE			(unsigned char)0x04
#define COLOR_LTBLUE			(unsigned char)0x05
#define COLOR_DKRED				(unsigned char)0x06
#define COLOR_CYAN				(unsigned char)0x07
#define COLOR_MEDRED			(unsigned char)0x08
#define COLOR_LTRED				(unsigned char)0x09
#define COLOR_DKYELLOW			(unsigned char)0x0A
#define COLOR_LTYELLOW			(unsigned char)0x0B
#define COLOR_DKGREEN			(unsigned char)0x0C
#define COLOR_MAGENTA			(unsigned char)0x0D
#define COLOR_GRAY				(unsigned char)0x0E
#define COLOR_WHITE				(unsigned char)0x0F

//*********************
// VDP related functions
//*********************

// set_graphics - sets up graphics I mode - 32x24, 256 chars, color, sprites
// Inputs: pass in VDP_SPR_xxx for the sprite mode you want
// Return: returns a value to be written to VDP_REG_MODE1 (and VDP_REG1_KSCAN_MIRROR if you use kscan())
// The screen is blanked until you do this write, to allow you time to set it up
unsigned char set_graphics_raw(unsigned char sprite_mode);
// this version enables the screen and sets the KSCAN copy for you
void set_graphics(unsigned char sprite_mode);

// set_text - sets up text mode - 40x24, 256 chars, monochrome (color set by VDP_REG_COL), no sprites
// Inputs: none
// Return: returns a value to be written to VDP_REG_MODE1 (and VDP_REG1_KSCAN_MIRROR if you use kscan())
// The screen is blanked until you do this write, to allow you time to set it up
unsigned char set_text_raw(void);
// this version enables the screen and sets the KSCAN copy for you
void set_text(void);

// set_text80 - sets up 80 column text mode - 80x24. 
// Inputs: none
// Return: returns a value to be written to VDP_REG_MODE1 (and VDP_REG1_KSCAN_MIRROR if you use kscan())
// The screen is blanked until you do this write, to allow you time to set it up
unsigned char set_text80_raw();
// this version enables the screen and sets the KSCAN copy for you
void set_text80();

// set_text80_color - sets up 80 column text mode - 80x24 with Position Attributes (F18A only!)
// Inputs: none
// this version enables the screen and sets the KSCAN copy for you
// Use bgcolor and textcolor functions from conio to change colors.
// Return: returns a value to be written to VDP_REG_MODE1 (and VDP_REG1_KSCAN_MIRROR if you use kscan())
// The screen is blanked until you do this write, to allow you time to set it up
unsigned char set_text80_color_raw();
// this version enables the screen and sets the KSCAN copy for you
void set_text80_color();

// set_text80_color - sets up 80 column text mode - 80x30 with Position Attributes (F18A only!)
// Inputs: none
// this version enables the screen and sets the KSCAN copy for you
// Use bgcolor and textcolor functions from conio to change colors.
// Return: returns a value to be written to VDP_REG_MODE1 (and VDP_REG1_KSCAN_MIRROR if you use kscan())
// The screen is blanked until you do this write, to allow you time to set it up
unsigned char set_text80x30_color_raw();
// this version enables the screen and sets the KSCAN copy for you
void set_text80x30_color();

// set_text64_color - sets up simulated 64-column text mode in bitmap mode - 64x24
// Inputs: none
// this version enables the screen and sets the KSCAN copy for you
// Use bgcolor and textcolor functions from conio to change colors.
void set_text64_color();

// set_multicolor - sets up multicolor mode - 64x48, 256 chars, color, sprites
// Inputs: pass in VDP_SPR_xxx for the sprite mode you want
// Return: returns a value to be written to VDP_REG_MODE1 (and VDP_REG1_KSCAN_MIRROR if you use kscan())
// The screen is blanked until you do this write, to allow you time to set it up
unsigned char set_multicolor_raw(unsigned char sprite_mode);
// this version enables the screen and sets the KSCAN copy for you
void set_multicolor(unsigned char sprite_mode);

// set_bitmap - sets up graphics II (aka bitmap) mode - 32x24, 768 chars in three zones, color, sprites
// Inputs: pass in VDP_SPR_xxx for the sprite mode you want
// Return: returns a value to be written to VDP_REG_MODE1 (and VDP_REG1_KSCAN_MIRROR if you use kscan())
// The screen is blanked until you do this write, to allow you time to set it up
unsigned char set_bitmap_raw(unsigned char sprite_mode);
// this version enables the screen and sets the KSCAN copy for you
void set_bitmap(unsigned char sprite_mode);

// writestring - writes an arbitrary string of characters at any position on the screen
// This is a fast low level function with no formatting or attributes, except in 64 column mode.
// Inputs: row and column (zero-based), NUL-terminated string to write
// Note: supports text mode
void writestring(unsigned char row, unsigned char col, char *pStr);

// vdpmemset - sets a count of VDP memory bytes to a value
// Inputs: VDP address to start, the byte to set, and number of repeats
void vdpmemset(int pAddr, unsigned char ch, int cnt);

// vdpmemcpy - copies a block of data from CPU to VDP memory
// Inputs: VDP address to write to, CPU address to copy from, number of bytes to copy
void vdpmemcpy(int pAddr, const unsigned char *pSrc, int cnt);

// vdpmemread - copies a block of data from VDP to CPU memory
// Inputs: VDP address to read from, CPU address to write to, number of bytes to copy
void vdpmemread(int pAddr, unsigned char *pDest, int cnt);

// vdpwriteinc - writes an incrementing sequence of values to VDP
// Inputs: VDP address to start, first value to write, number of bytes to write
// This is intended to be useful for setting up bitmap and multicolor mode with
// incrementing tables
void vdpwriteinc(int pAddr, unsigned char nStart, int cnt);

// vdpchar - write a character to VDP memory (NOT to be confused with basic's CALL CHAR)
// Inputs: VDP address to write, character to be written
// This is NOT for writing text, use vsetchar
void vdpchar(int pAddr, unsigned char ch);

// vsetchar - write a text character to a text screen (mode independent)
// inputs: VDP address (not offset), character to be written
// Note: 64 column mode will work with an offset only
// WARNING: this is uninitialized until you call one of the setXXX graphics modes
extern void (*vsetchar)(int pAddr, unsigned char ch);

// vdpreadchar - read a character from VDP memory
// Inputs: VDP address to read
// Outputs: byte
unsigned char vdpreadchar(int pAddr);

// vdpwritescreeninc - like vdpwriteinc, but writes to the screen image table
// Inputs: offset from the screen image table to write, first value to write, number of bytes to write
void vdpwritescreeninc(int pAddr, unsigned char nStart, int cnt);

// vdpscreenchar - like vdpchar, but writes to the screen image table
// Inputs: offset from the screen image table to write to, value to be written
void vdpscreenchar(int pAddr, unsigned char ch);

// vdpwaitvint - enables console interrupts, then waits for one to happen
// Interrupts are disabled upon exit.
// returns non-zero if the interrupt fired before entry (ie: we are late)
unsigned char vdpwaitvint(void);

// putchar - writes a single character with limited formatting to the bottom of the screen
// Inputs: character to emit
// Returns: character input
// All characters are emitted except \r and \n which is handled for scrn_scroll and next line.
// It works in both 32x24 and 40x24 modes. Tracking of the cursor is thus 
// automatic in this function, and it pulls in scrn_scroll.
int putchar(int x);
#define vdpputchar putchar

// vdpprintf - writes a string with limited formatting. Only supports a very small subset
// of formatting at the moment. Supports width (for most fields), s, u, i, d, c and X
// (X is byte only). This function will call in putchar().
// Inputs: format string, and varable argument list
// Returns: always returns 0
int vdpprintf(char *str, ...);

// raw_vdpmemset - sets bytes at the current VDP address
void raw_vdpmemset(unsigned char ch, int cnt);

// raw_vdpmemcpy - copies bytes from CPU to current VDP address
void raw_vdpmemcpy(const unsigned char *p, int cnt);

// convert an x,y into an offset based on the screen mode
unsigned int getscreenoffset(int x, int y);

// putstring - writes a string with limited formatting to the bottom of the screen
// Inputs: NUL-terminated string to write
// This function only emits printable ASCII characters (32-127). It works in both
// 32x24 and 40x24 modes. It recognizes \r to go to the beginning of the line, and
// \n to go to a new line and scroll the screen. Tracking of the cursor is thus 
// automatic in this function, and it pulls in scrn_scroll.
void putstring(char *s);

// puts - calls putstring, but adds a carriage return. Needed for gcc compatibility
// always returns 1
int puts(char *s);

// printf - writes a string with limited formatting. Only supports a very small subset
// of formatting at the moment. Supports width (for most fields), s, u, i, d, c and X
// (X is byte only). This function will call in putchar().
// Inputs: format string, and varable argument list
// Returns: always returns 0
// TODO: pull in the printf from Cool Herders, ditch vdpprintf
int printf(char *str, ...);

// hexprint - generates a 2 character hex string from an int and calls putstring to print it
void hexprint(unsigned char x);

// fast_hexprint - generates a 2 character hex string from an int and calls putstring to print it
// uses a 512 byte lookup table - so it is fast but costs more to use
void fast_hexprint(unsigned char x);

// faster_hexprint - works like fast_hexprint but displays directly to VDPWD, no formatting or
// scroll and you must set the VDP address before calling
void faster_hexprint(unsigned char x);

// scrn_scroll - scrolls the screen upwards one line - works in 32x24, 40x24 and 80x24 modes
// the pointer let you replace it, particularly with fast_scrn_scroll
void scrn_scroll_default();
extern void (*scrn_scroll)();

// fast_scrn_scroll- does the same, but uses 256 fixed bytes to do it faster
void fast_scrn_scroll();

// hchar - repeat a character horizontally on the screen, similar to CALL HCHAR
// Inputs: row and column (0-based, not 1-based) to start, character to repeat, number of repetitions (not optional)
// Note: for a single character, vdpscreenchar() is more efficient
void hchar(unsigned char r, unsigned char c, unsigned char ch, int cnt);

// hchar64 - slower version that supports all modes including 64 character
// Inputs: row and column (0-based, not 1-based) to start, character to repeat, number of repetitions (not optional)
void hchar64(unsigned char r, unsigned char c, unsigned char ch, int cnt);

// vchar - repeat a character vertically on the screen, similar to CALL VCHAR
// Inputs: row and column (0-based, not 1-based), character to repeat, number of repetitions (not optional)
// Note: for a single character, vdpscreenchar() is more efficient
void vchar(unsigned char r, unsigned char c, unsigned char ch, int cnt);

// vchar - (slightly) slower version that supports all modes including 64 character
// Inputs: row and column (0-based, not 1-based), character to repeat, number of repetitions (not optional)
void vchar64(unsigned char r, unsigned char c, unsigned char ch, int cnt);

// gchar - return a character from the screen, similar to CALL GCHAR
// Inputs: row and column (0-based, not 1-based) to read from
// Return: character at the specified position on the screen
unsigned char gchar(unsigned char r, unsigned char c);

// sprite - set up an entry in the sprite attribute list, similar to CALL SPRITE
// Inputs: sprite number (0-31), character (0-255), color (COLOR_xx), row and column (0-based)
// Note that motion set up is not handled by this function
// Note that row 255 is the first line on the screen
// And finally, note that a row of 208 will disable display of all subsequent sprite numbers
void sprite(unsigned char n, unsigned char ch, unsigned char col, unsigned char r, unsigned char c);

// delsprite - remove a sprite by placing it offscreen
// Inputs: sprite number (0-31) to hide
void delsprite(unsigned char n);

// charset - load the default character set from GROM. This will load both upper and lowercase (small capital) characters.
// Not compatible with the 99/4, if it matters.
void charset(void);

// charsetlc - load the default character set including true lowercase. This code includes a lower-case character
// set and shifts the GROM character set to align better with it. Because it pulls in data, it does take a little more
// memory (208 bytes). Not compatible with the 99/4, if it matters.
void charsetlc(void);

// gplvdp - copy data from a GPL function to VDP memory. 
// Inputs: address of a GPL vector, VDP address to copy to, number of characters to copy
// This is a very specialized function used by the charset() functions. It assumes a GPL 'B'
// instruction at the vector, and that the first instruction thereafter is a 'DEST'. It uses
// this to find the actual character set data regardless of the GROM version. This function
// is not compatible with the 99/4 (because it copies 7 bytes per character, and the 99/4
// only provided 6 bytes). (only supported on TI)
void gplvdp(int vect, int adr, int cnt);

// user interrupt access helpers (for more portable code)
//void vdpinit();	called automatically, don't use
void setUserIntHook(void (*hookfn)(void));
void clearUserIntHook(void);

#ifdef GBA
// Hook for non-vblank interrupts - argument is a copy of REG_IF
void setOtherIntHook(void (*hookfn)(unsigned int));
void clearOtherIntHook();
void InterruptProcess();
#else
void my_nmi(void);
#endif

// bm_setforeground - specify foreground color to use when drawing
void bm_setforeground(int c);

// bm_setbackground - specify background color to use when drawing
void bm_setbackground(int c);

// bm_clearscreen - clear the screen and sets all regions to the
//                  current colors
void bm_clearscreen();

// bm_setpixel - turn a pixel on
// Inputs: x - 0-255 - horizontal location
//         y - 0-192 - vertial location
void bm_setpixel(unsigned int x, unsigned int y);

// bm_clearpixel - turn a pixel off
// Inputs: x - 0-255 - horizontal location
//         y - 0-192 - vertial location
void bm_clearpixel(unsigned int x, unsigned int y);

// bm_drawline - plot a line between two points, mode 0 = clear, mode 1 = set
void bm_drawline(int x0, int y0, int x1, int y1, int mode);

// TODO: the fast methods currently don't have Coleco equivalents

// bm_drawlinefast - plot a line between two points
// this version can DRAW (0), ERASE (1) or XOR (2)
// Erase and XOR modes skip the color update
// this function is faster but corrupts scratchpad from >8320 to >833F
// extra setup time may make it slower for short lines
// finally, color table must be at >2000 and pattern table at >0000 (setbitmap does this)
void bm_drawlinefast(int x0, int y0, int x1, int y1, int mode);

// bm_sethlinefast - draws a very fast horizontal line (there's not much speedup possible for vertical)
// does NOT change the color
void bm_sethlinefast(unsigned int x0, unsigned int y0, unsigned int x1);

// bm_clearhlinefast - clears a very fast horizontal line (there's not much speedup possible for vertical)
void bm_clearhlinefast(unsigned int x0, unsigned int y0, unsigned int x1);

// bm_consolefont - loads console font to vdp, then copies it up into ram for
// later use in bitmap mode. Use this before switching to bitmap mode if
// you want a TI font.
void bm_consolefont();

// bm_putc - draw a character at a tile location.
// Inputs : c - character column  0:31
//          r - character row  0:23
void bm_putc(int c, int r, unsigned char alphanum);

// bm_puts - draw a 0 terminated string at a tile location.
//    this provides no scrolling, or bounds limiting.
// Inputs : c - character column  0:31
//          r - character row  0:23
void bm_puts(int c, int r, char* str);

// bm_placetile - draw a 8x8 pattern at the given tile.
// Inputs : c - character column  0:31
//          r - character row  0:23
void bm_placetile(int c, int r, const unsigned char* pattern);

#define BM_FONT_SIZE (8*(127-32))

// globals holding pen color and background for bitmap drawing
// 0xF0 nibble is foreground, 0x0F nibble is background.
// or use bm_setforeground and bm_setbackground
extern unsigned char gBitmapColor;

// address of bitmap mode font pattern table in cpu memory. 
// This can be intialized by calling bm_consolefont(), or
// by setting it to your own characterset patterns spanning
// characters ' ' to '~'
// TODO: ditch this concept - at least for systems that have the charset in ROM
extern unsigned char* gBmFont;

// global pointers for all to enjoy - make sure the screen setup code updates them!
// assumptions here are for E/A environment, they may not be accurate and your
// program should NOT trust them until after one of the mode set functions is called.
extern unsigned int gImage;				// SIT, Register 2 * 0x400
extern unsigned int gColor;				// CR,  Register 3 * 0x40
extern unsigned int gPattern;			// PDT, Register 4 * 0x800
extern unsigned int gSprite;			// SAL, Register 5 * 0x80
extern unsigned int gSpritePat;			// SDT, Register 6 * 0x800

// text position information used by putstring and scrn_scroll
extern int nTextRow,nTextEnd;
extern int nTextPos,nTextFlags;

// bitflags for nTextFlags - no guessing! ;)
// used for things that the generic code makes decisions on
#define TEXT_FLAG_HAS_ATTRIBUTES 0x8000		// attribute table at gColor
#define TEXT_FLAG_IS_BITMAPPED   0x4000		// graphics in a bitmapped mode
#define TEXT_FLAG_IS_MULTICOLOR	 0x2000		// graphics in multicolor mode
#define TEXT_FLAG_IS_F18A        0x1000		// mode is F18A specific
#define TEXT_WIDTH_32            0x0800     // I wonder if I'll regret bitflags for width...
#define TEXT_WIDTH_40            0x0400		// Yes. TODO: move width and height to macros? Can calculate.
#define TEXT_WIDTH_64            0x0200
#define TEXT_WIDTH_80            0x0100
#define TEXT_HEIGHT_24           0x0080
#define TEXT_HEIGHT_30           0x0040
#define TEXT_CUSTOM_VSETCHAR	 0x0020

extern unsigned char gSaveIntCnt;	// console interrupt count byte

// 512 byte lookup table for converting a byte to two ASCII hex characters
extern const unsigned int byte2hex[256];

#endif /* VDP_H */
