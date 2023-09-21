#include "kscan.h"

#ifdef TI99
unsigned char kscan(unsigned char mode) {
	KSCAN_MODE = mode;
	__asm__("LWPI >83E0\n\tBL @>000E\n\tLWPI >8300");
	return KSCAN_KEY;
}

#endif

#ifdef COLECO
#define SELECT 0x2a

// fire buttons are all the same for the moment. Note that 2 and 3 read
// via the keypad, but 1 has a dedicated bit and could be overlapped

// note: keys index 8 and 4 are fire 2 and fire 3, respectively
// for now, I'm defining them the same as regular fire (18),
// but if I ever want to split up them, I can update this.
extern const unsigned char keys[16];
// FIRE 1 returns as bit 0x40 being low

static volatile __sfr __at 0xfc port0;
static volatile __sfr __at 0xff port1;
static volatile __sfr __at 0x80 port2;
static volatile __sfr __at 0xc0 port3;

// For Coleco, all modes except 2 read controller 1, and 2 reads controller 2
unsigned char kscan(unsigned char mode) {
	unsigned char key;

	port2 = SELECT;		// select keypad

	if (mode == KSCAN_MODE_RIGHT) {
		key = port1;
	} else {
		key = port0;
	}
	// bits: xFxxNNNN (F - active low fire 2, NNNN - index into above table)

	// if reading joystick, the fire2 button overrides
	// Note this limits us not to read keypad and fire2 at the same time,
	// which honestly I will probably want later. (Also see below for
	// where FIRE1 overrides this.)
	if ((key&0x40) == 0) {
		KSCAN_KEY = JOY_FIRE2;
	} else {
		KSCAN_KEY = keys[key & 0xf];
	}
  
	if ((mode == KSCAN_MODE_LEFT) || (mode == KSCAN_MODE_RIGHT)) {
		KSCAN_JOYX = 0;
		KSCAN_JOYY = 0;

		port3 = SELECT;		// select joystick
		if (mode == KSCAN_MODE_RIGHT) {
			key = port1;
		} else {
			key = port0;
		}
		// active low bits:
		// xFxxLDRU
		if ((key&0x40) == 0) {
			KSCAN_KEY = JOY_FIRE;
		}
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

	if (KSCAN_KEY != 0xff) {
		KSCAN_STATUS |= KSCAN_MASK;
	} else {
		KSCAN_STATUS = 0;
	}

	return KSCAN_KEY;
}
#endif
