// Fast keyboard scan for the TI-99/4A by Tursi aka Mike Brent
// This code and library released into the Public Domain
// You can copy this file and use it at will ;)

// Mode 0 reads only the keyboard.
// Mode 1 and 2 read the joystick fire button, and if not pressed, then reads the keyboard
// if that ends up too slow I might make a separate function just for the fire buttons... 
// but that helps parity between the TI and Coleco versions.

#include "kscan.h"

#ifdef TI99
// TODO: add a 99/4 keyboard scan with some kind of autodetect (whether once or every time)

// By columns, then rows. 8 Rows per column. No shift states
const unsigned char keymap[] = {
		//61,32,13,255,1,2,3,255,   1,2,3 are fctn,shift,ctrl - we'll ignore them for now
		61,32,13,255,255,255,255,255,
		'.','L','O','9','2','S','W','X',
		',','K','I','8','3','D','E','C',
		'M','J','U','7','4','F','R','V',
		'N','H','Y','6','5','G','T','B',
		'/',';','P','0','1','A','Q','Z'
};

// TODO: I want to rethink compatibility between joystick buttons and Q/Y
// Does it belong in the low level function? Should we have a disable flag?
// Or an alt function that is more precise?
void kscanfast(unsigned char mode) {
	KSCAN_KEY = 0xff;
	
    if (mode > 0) {
		unsigned int key;

		int col = 0x0600;		// joystick 1 fire column

		if (mode == 2) {
			col = 0x0700;		// make that joystick 2
		}

		__asm__ volatile ("li r12,>0024\n\tldcr %1,3\n\tsrc r12,7\n\tli r12,>0006\n\tclr %0\n\tstcr %0,1" : "=r"(key) : "r"(col) : "r12","cc");	// set cru, column, delay, read (only need 1 bit)
		if (key == 0) {
			KSCAN_KEY = 18;
		} else {
            // if not the joystick, then check for Q/Y (everyone expects this)
            unsigned int mask = 0x4000;
            col = 0x0500;       // Q
            if (mode == 2) {
                col = 0x0400;   // Y
                mask = 0x0400;
            }
            
            __asm__ volatile ("li r12,>0024\n\tldcr %1,3\n\tsrc r12,7\n\tli r12,>0006\n\tclr %0\n\tstcr %0,7" : "=r"(key) : "r"(col) : "r12","cc");	// set cru, column, delay, read (need 7 bits)
            if ((key&mask) == 0) {
                KSCAN_KEY = 18;
            }
        }
		if (KSCAN_KEY == 18) {
	        return;
		}
		// else fall through and read the keyboard
	}

    // otherwise read the keyboard
	{
		for (unsigned int col=0; col < 0x0600; col += 0x0100) {
			unsigned int key;
			__asm__ ("li r12,>0024\n\tldcr %1,3\n\tsrc r12,7\n\tli r12,>0006\n\tclr %0\n\tstcr %0,8" : "=r"(key) : "r"(col) : "r12","cc");	// set cru, column, delay, read
			unsigned int shift=0x8000;

			for (int cnt=7; cnt>=0; cnt--) {
				// a pressed key returns a 0 bit
				if (key & shift) {
					shift>>=1;
					continue;
				}
				// found one
				KSCAN_KEY = keymap[(col>>5)+cnt];
				if (KSCAN_KEY == 255) {
                    shift>>=1;
                    continue;
                }
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

#ifdef GBA

#include <tursigb.h>

// NOTE: keys are A, B, L, R, Select and Start
// Returning both A and B are regular fire (18)
// Select as backspace (8) and start as enter (13)
// L and R can be N and Y, I guess!

// For GBA, there is only ever 1 controller. All modes except '2' read it, '2' reads nothing.
// Currently mode 0 will split A and B, and 1 will return both as 'FIRE'
void kscanfast(unsigned char mode) {
	unsigned short key;

    if (mode == KSCAN_MODE_RIGHT) {
        KSCAN_KEY = 0xff;
        return;
    }

    // active low
    // TODO: this only allows one button return at a time - clearly not adequate
    key = REG_KEYINPUT;

    // A or B = fire in mode 1, and A or B in mode 0
    if ((key&(BTN_A|BTN_B)) != (BTN_A|BTN_B)) {
        if (mode == 0) {
            if ((key&BTN_A) == 0) {
                KSCAN_KEY = 'A';
            } else if ((key&BTN_B) == 0) {
                KSCAN_KEY = 'B';
            }
        } else {
            KSCAN_KEY = JOY_FIRE;
        }
    } else if ((key&BTN_SELECT) == 0) {
        KSCAN_KEY = 8;  // backspace
    } else if ((key&BTN_START) == 0) {
        KSCAN_KEY = 13; // enter
    } else if ((key&BTN_L) == 0) {
        KSCAN_KEY = 'N';
    } else if ((key&BTN_R) == 0) {
        KSCAN_KEY = 'Y';
    } else {
        KSCAN_KEY = 0xff;
    }
}

#endif

#ifdef CLASSIC99
void kscanfast(unsigned char mode) {
    kscan(mode);
}
#endif
