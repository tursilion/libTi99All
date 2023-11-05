// Fast keyboard scan for the TI-99/4A by Tursi aka Mike Brent
// This code and library released into the Public Domain
// You can copy this file and use it at will ;)

// Mode 0 reads only the keyboard.
// Mode 1 and 2 read the joystick fire button, and if not pressed, then reads the keyboard
// if that ends up too slow I might make a separate function just for the fire buttons... 
// but that helps parity between the TI and Coleco versions.

#include "kscan.h"

#ifdef TI99
// By columns, then rows. 8 Rows per column. No shift states
const unsigned char keymap[] = {
		61,32,13,255,1,2,3,255,
		'.','L','O','9','2','S','W','X',
		',','K','I','8','3','D','E','C',
		'M','J','U','7','4','F','R','V',
		'N','H','Y','6','5','G','T','B',
		'/',';','P','0','1','A','Q','Z'
};

void kscanfast(unsigned char mode) {
	KSCAN_KEY = 0xff;
	
    if (mode > 0) {
		unsigned int key;

		int col = 0x0600;		// joystick 1 fire column

		if (mode == 2) {
			col = 0x0700;		// make that joystick 2
		}

		__asm__("li r12,>0024\n\tldcr %1,3\n\tsrc r12,7\n\tli r12,>0006\n\tclr %0\n\tstcr %0,1" : "=r"(key) : "r"(col) : "r12");	// set cru, column, delay, read (only need 1 bit)
		if (key == 0) {
			KSCAN_KEY = 18;
			return;
		}
	}

    // otherwise read the keyboard
	{
		for (unsigned int col=0; col < 0x0600; col += 0x0100) {
			unsigned int key;
			__asm__("li r12,>0024\n\tldcr %1,3\n\tsrc r12,7\n\tli r12,>0006\n\tclr %0\n\tstcr %0,8" : "=r"(key) : "r"(col) : "r12");	// set cru, column, delay, read
			unsigned int shift=0x8000;

			for (int cnt=7; cnt>=0; cnt--) {
				// a pressed key returns a 0 bit
				if (key & shift) {
					shift>>=1;
					continue;
				}
				// found one
				KSCAN_KEY = keymap[(col>>5)+cnt];
				return;
			}
		}
	}
}

#endif

#ifdef COLECO

// coleco and SMS are very different

#ifdef SMS

static volatile __sfr __at 0xdc pad0;
static volatile __sfr __at 0xdd pad1;
extern unsigned char pause;

// TODO: reset reads as bit 0x10 on pad1 - do we need to manually handle reset in software?

// For SMS, all modes except 2 read controller 1, and 2 reads controller 2
void kscanfast(unsigned char mode) {
	unsigned char key;
	
	KSCAN_KEY = 0xff;

	if (mode == KSCAN_MODE_RIGHT) {
		key = pad1;
		if ((key&0x08)==0) KSCAN_KEY=JOY_FIRE2;
		if ((key&0x04)==0) KSCAN_KEY=JOY_FIRE;   // todo: make it possible to read both at once?
	} else {
		key = pad0;
		if ((key&0x20)==0) KSCAN_KEY=JOY_FIRE2;
		if ((key&0x10)==0) KSCAN_KEY=JOY_FIRE;   // todo: make it possible to read both at once?
	}
	
	key=pad1;
	if (key&0x10) KSCAN_KEY='#';      // reset
	if (pause) { pause=0; KSCAN_KEY='*'; }
}

#else
#define SELECT 0x2a

// note: keys index 8 and 4 are fire 2 and fire 3, respectively
// for now, I'm defining them the same as regular fire (18),
// but if I ever want to split up them, I can update this.
const unsigned char keys[16] = {
	0xff, '8', '4', '5', 
	0xff, '7', '#', '2',
	0xff, '*', '0', '9',
	'3',  '1', '6', 0xff
};
// FIRE 1 returns as bit 0x40 being low

static volatile __sfr __at 0xfc port0;
static volatile __sfr __at 0xff port1;
static volatile __sfr __at 0x80 port2;
static volatile __sfr __at 0xc0 port3;

// For Coleco, all modes except 2 read controller 1, and 2 reads controller 2
void kscanfast(unsigned char mode) {
	unsigned char key;

	port2 = SELECT;		// select keypad

	if (mode == KSCAN_MODE_RIGHT) {
		key = port1;
	} else {
		key = port0;
	}
	// bits: xFxxNNNN (F - active low fire, NNNN - index into above table)

	// if reading joystick, the fire button overrides
	// Note this limits us not to read keypad and fire at the same time,
	// which honestly I will probably want later.
	if ((key&0x40) == 0) {
		KSCAN_KEY = JOY_FIRE2;
	} else {
		KSCAN_KEY = keys[key & 0xf];
	}

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

}

#endif
#endif