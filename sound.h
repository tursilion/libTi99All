// Helpers for direct and console interrupt sound processing

#ifndef SOUND_H
#define SOUND_H

//*********************
// direct sound chip access
//*********************

#ifdef TI99
#define SOUND		*((volatile unsigned char*)0x8400)
#endif
#ifdef COLECO
#ifdef SMS
volatile __sfr __at 0x06 SOUND;
#else
volatile __sfr __at 0xff SOUND;
#endif
#endif



// Command nibbles
#define TONE1_FREQ	0x80
#define TONE1_VOL	0x90
#define TONE2_FREQ	0xA0
#define TONE2_VOL	0xB0
#define TONE3_FREQ	0xC0
#define TONE3_VOL	0xD0
#define NOISE_MODE	0xE0
#define NOISE_VOL	0xF0

// mute all channels
inline void MUTE_SOUND()					{ SOUND=TONE1_VOL|0x0f; SOUND=TONE2_VOL|0x0f; SOUND=TONE3_VOL|0x0f; SOUND=NOISE_VOL|0x0f; }

#ifdef COLECO
#ifndef SMS
//*********************
// AY sound chip access (if SGM installed)
//*********************
// NOTE: The Phoenix implementation of the AY is somewhat bugged.
// To ensure your software works there, make sure of these two
// requirements:
// 1) At startup, set mixer and volume registers to 0 - even if you aren't using them.
//    It may be best to just zero all registers to ensure there's no garbage in them.
// 2) The mixer only runs if channel A/B/C tone frequency is in range. So if you have
//    a channel that is noise only, set a small but legal value on the tone generator
//    as well (ie: 16). You won't hear the tone but without it, you won't hear the noise
//    either.
// These are based on observation and experimentation and are not confirmed in the HDL.

volatile __sfr __at 0x50 AY_REGISTER;
volatile __sfr __at 0x51 AY_DATA_WRITE;
volatile __sfr __at 0x52 AY_DATA_READ;

#define AY_PERIODA_LOW  0
#define AY_PERIODA_HIGH 1
#define AY_PERIODB_LOW  2
#define AY_PERIODB_HIGH 3
#define AY_PERIODC_LOW  4
#define AY_PERIODC_HIGH 5
#define AY_NOISE        6
#define AY_MIXER        7
#define AY_VOLA         8
#define AY_VOLB         9
#define AY_VOLC         10
#define AY_ENV_LOW      11
#define AY_ENV_HIGH     12
#define AY_ENV_SHAPE    13
#define AY_PORTA        14
#define AY_PORTB        15

#endif
#endif

#endif /* SOUND_H */
