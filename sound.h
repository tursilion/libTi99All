// Helpers for direct and console interrupt sound processing

#ifndef SOUND_H
#define SOUND_H

//*********************
// direct sound chip access
//*********************

#ifdef TI99
#define SOUNDCHIP 0x8400
#define SOUND(x)		*((volatile unsigned char*)SOUNDCHIP)=(x)

// base addresses for each chip in the FORTI card
#define FORTI_CHIP1 0x841c
#define FORTI_CHIP2 0x841a
#define FORTI_CHIP3 0x8416
#define FORTI_CHIP4 0x840e
#define FORTI_CONSOLE 0x841e
#endif
#ifdef COLECO
#ifdef SMS
volatile __sfr __at 0x06 SOUNDCHIP;
#else
volatile __sfr __at 0xff SOUNDCHIP;
#endif
#define SOUND(x) SOUNDCHIP=(x)
#endif
#ifdef GBA
// again, need to change a memory write to a function call
// this /requires/ libgbavgm2 as that contains the sound chip emulation
#include <GBASNPlay.h>
#define SOUNDCHIP
#define SOUND(x) snsim(x)
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
inline void MUTE_SOUND() { SOUND(TONE1_VOL|0x0f); SOUND(TONE2_VOL|0x0f); SOUND(TONE3_VOL|0x0f); SOUND(NOISE_VOL|0x0f); }

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

#ifdef TI99
// SID Blaster Sound chip access
// Requires the SID chip be mapped in by setting the CRU
// All SID access is WRITE-ONLY, you can not read back from the SID Blaster.

// Map in the SID chip by setting a low CRU. NEVER call this if a DSR is active, both devices
// will be listening at the same time. (It may work, but it's a bad idea.). The bit set here
// is just part of the keyboard select.
#define MAP_SID_BLASTER __asm__ volatile( "clr  r12\n\tsbo 18" : : : "r12","cc" )

// SID channel control:
// FREQLO - Frequency low byte. On the SID Blaster the SID is clocked at 1MHz which is slower than
//          an NTSC C64 SID (but faster than PAL), so frequencies are correspondingly adjusted.
//          FREQ = ((256^3)/(1000000)) * Hz (or, 16.777216 * Hz)
// FREQHI - Frequency high byte. All 16 bits are available between LO and HI.
// PWLO   - Pulse width low byte. 12 bits are available with the count representing the low time of the
//          waveform. 0x800 is 50% duty. Pulse waveform must be set in CR.
// PWHI   - Pulse width high 4 bits.
// CR     - Control Register:
//          80 - Enable NOISE output (Warning: can lock up if other generators are also active. Reset with TEST)
//          40 - Enable PULSE output (PW registers active)
//          20 - Enable SAWTOOTH output
//          10 - Enable TRIANGLE output
//          08 - Test - locks output to a single level
//          04 - Ring Mod - 1: Replaces TRIANGLE with modulated combo of voice 1 and 3. Only 3's frequency affects it. 
//                          2: Replaces TRIANGLE with modulated combo of voice 2 and 1.
//                          3: Replaces TRIANGLE with modulated combo of voice 3 and 2.
//               (Not sure if set on other voices)
//          02 - Sync - 1: Sync osc 1 with osc 3
//                      2: Sync osc 2 with osc 1
//                      3: Sync osc 3 with osc 2 
//          01 - Gate - controls envelope generator. Setting it starts the Attack/Decay/Sustain cycle, clearing it starts Release
// AD     - Bits 0-3 = Decay rate to sustain level, bits 4-7 = Attack rate to maximum volume
// SR     - Bits 0-3 = Release rate to zero, bits 4-7 = Sustain level (0=mute, 15=maximum volume)
// FCLO   - Low 3 bits only are the low part of an 11 bit number that controls the cutoff frequency of the filter.
// FCHI   - High 8 bits
// RESFLT - High 4 bits control the resonance of the filter, from 0 (none) to 15 (max).
//          08 - Filt EX - filter external audio input (not connected)
//          04 - Filt 3 - Filter voice 3
//          02 - Filt 2 - Filter voice 2
//          01 - Filt 1 - Filter voice 1
// MODEVOL- 80 - 3 OFF - turn off voice 3 (but still allow it for modulation)
//          40 - HP - enable high pass filter
//          20 - BP - enable bandpass filter
//          10 - LP - enable lowpass filter
//          0F - Lower four bits are master volume from 0 (mute) to 15 (max)
//
// ADSR Rates (in ms at 1MHz) Attack and Decay/Release values do not have to be the same.
// Val  Att    Dec/Rel
// 0    2      6
// 1    8      24
// 2    16     48
// 3    24     72
// 4    38     114
// 5    56     168
// 6    68     204
// 7    80     240
// 8    100    300
// 9    240    750
// 10   500    1,500
// 11   800    2,400
// 12   1,000  3,000
// 13   3,000  9,000
// 14   5,000  15,000
// 15   8,000  24,000

#define SID_BASE_ADDRESS 0x5800

#define SIDBLASTER_FREQLO1 *((volatile unsigned char*)(SID_BASE_ADDRESS))
#define SIDBLASTER_FREQHI1 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x02))
#define SIDBLASTER_PWLO1 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x04))
#define SIDBLASTER_PWHI1 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x06)) 
#define SIDBLASTER_CR1 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x08))
#define SIDBLASTER_AD1 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x0A))
#define SIDBLASTER_SR1 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x0C))
        
#define SIDBLASTER_FREQLO2 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x0e))
#define SIDBLASTER_FREQHI2 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x10))
#define SIDBLASTER_PWLO2 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x12))
#define SIDBLASTER_PWHI2 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x14))
#define SIDBLASTER_CR2 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x16))
#define SIDBLASTER_AD2 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x18))
#define SIDBLASTER_SR2 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x1A))
        
#define SIDBLASTER_FREQLO3 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x1C))
#define SIDBLASTER_FREQHI3 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x1E))
#define SIDBLASTER_PWLO3 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x20))
#define SIDBLASTER_PWHI3 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x22))
#define SIDBLASTER_CR3 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x24))
#define SIDBLASTER_AD3 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x26))
#define SIDBLASTER_SR3 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x28))
        
#define SIDBLASTER_FCLO *((volatile unsigned char*)(SID_BASE_ADDRESS+0x2A))
#define SIDBLASTER_FCHI *((volatile unsigned char*)(SID_BASE_ADDRESS+0x2C))
#define SIDBLASTER_RESFLT *((volatile unsigned char*)(SID_BASE_ADDRESS+0x2E)) 
#define SIDBLASTER_MODEVOL *((volatile unsigned char*)(SID_BASE_ADDRESS+0x30))

// not available as we can not read from the SID
//#define SIDBLASTER_POTX *((volatile unsigned char*)(SID_BASE_ADDRESS+0x30))
//#define SIDBLASTER_POTY *((volatile unsigned char*)(SID_BASE_ADDRESS+0x32))
//#define SIDBLASTER_OSC3 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x34))
//#define SIDBLASTER_ENV3 *((volatile unsigned char*)(SID_BASE_ADDRESS+0x36))

#define SIDBLASTER_CR_NOISE (unsigned char)0x80
#define SIDBLASTER_CR_PULSE (unsigned char)0x40
#define SIDBLASTER_CR_SAWTOOTH (unsigned char)0x20
#define SIDBLASTER_CR_TRIANGLE (unsigned char)0x10
#define SIDBLASTER_CR_TEST (unsigned char)0x8
#define SIDBLASTER_CR_RING (unsigned char)0x4
#define SIDBLASTER_CR_SYNC (unsigned char)0x2
#define SIDBLASTER_CR_GATE (unsigned char)0x1

#endif


#endif /* SOUND_H */
