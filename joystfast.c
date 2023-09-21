// Fast keyboard scan for the TI-99/4A by Tursi aka Mike Brent
// This code and library released into the Public Domain
// You can copy this file and use it at will ;)

#include "kscan.h"

#ifdef TI99

void joystfast(int unit) {
	unsigned int result;

	// read the joystick lines (column 6 or 7, (5 added to unit))
	__asm__("li r12,>0024\n\tai %1,5\n\tswpb %1\n\tldcr %1,3\n\tsrc r12,7\n\tli r12,>0006\n\tclr %0\n\tstcr %0,8" : "=r"(result) : "r"(unit) : "r12");

	KSCAN_JOYY = 0;
	KSCAN_JOYX = 0;

	if ((result & 0x0200) == 0) KSCAN_JOYX = JOY_LEFT;
	if ((result & 0x0400) == 0) KSCAN_JOYX = JOY_RIGHT;
	if ((result & 0x0800) == 0) KSCAN_JOYY = JOY_DOWN;
	if ((result & 0x1000) == 0) KSCAN_JOYY = JOY_UP;
}

#endif

#ifdef COLECO
// Address to read back the detected key. 0xFF if no key was pressed.
volatile unsigned char KSCAN_KEY;
// Address to read back the joystick X axis (scan modes 1 and 2 only)
volatile unsigned char KSCAN_JOYY;
// Address to read back the joystick Y axis (scan modes 1 and 2 only)
volatile unsigned char KSCAN_JOYX;
// Address to check the status byte. KSCAN_MASK is set if a key was pressed
volatile unsigned char KSCAN_STATUS;

static volatile __sfr __at 0xfc port0;
static volatile __sfr __at 0xff port1;
static volatile __sfr __at 0x80 port2;
static volatile __sfr __at 0xc0 port3;

// double check rawhide docs, but I think this value is unimportant...
#define SELECT 0x2a

void joystfast(unsigned char unit) {
	unsigned char key;

	if ((unit == KSCAN_MODE_LEFT) || (unit == KSCAN_MODE_RIGHT)) {
		KSCAN_JOYX = 0;
		KSCAN_JOYY = 0;

		port3 = SELECT;		// select joystick
		if (unit == KSCAN_MODE_RIGHT) {
			key = port1;
		} else {
			key = port0;
		}
		// active low bits:
		// xxxxLDRU
		if ((key&0x08) == 0) {
			KSCAN_JOYX = JOY_LEFT;
		}
		if ((key&0x04) == 0) {
			KSCAN_JOYY = JOY_DOWN;
		}
		if ((key&0x02) == 0) {
			KSCAN_JOYX = JOY_RIGHT;
		}
		if ((key&0x01) == 0) {
			KSCAN_JOYY = JOY_UP;
		}
	}
}
#endif
