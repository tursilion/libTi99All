// RAYLIB (Linux/PC) audio backend for libTi99All - by Tursi aka Mike Brent
// This code and library released into the Public Domain
//
// This is the PC equivalent of Players/libgbavgm2/GBAPlayerSN.c. The SN76489
// chip-emulation math below (setfreq/setvol/snsim/the per-sample synth loop)
// is lifted verbatim from that file - it was already pure C with no GBA
// hardware dependency. Only the "how do generated samples actually reach
// speakers" half differs: GBA feeds DMA1+Timer1+FIFO_A from an interrupt:
// here, a raylib AudioStream callback pulls samples on raylib's own audio
// thread, so there's no hardware to set up and no per-frame refill call
// needed - snupdateaudio() below is a no-op for that reason.
//
// Also implements the GBA's second audio channel (Direct Sound B), which
// the game uses to play raw 8-bit PCM sound effects directly rather than
// through the chip emulator - see music.c's startsfx()/stopsfx(). That maps
// directly onto raylib's Sound/Wave API.
#ifdef RAYLIB

#include <raylib.h>
#include <stdbool.h>
#include "tursigb.h"

//*********************
// SN76489 chip emulation (music, "channel A")
//*********************

void setvol(int chan, int vol);

// shared between the main thread (snsim(), called from CPlayer's SongLoop())
// and raylib's audio-mixing thread (musicAudioCallback() below) with no
// locking - same trade-off the rest of this port makes elsewhere. It's a
// handful of plain ints; worst case is an occasional single-sample glitch,
// not a crash.
static int nCounter[4] = {0,0,0,1};
static int nNoisePos = 1;
static unsigned short LFSR = 0x4000;
static int nRegister[4] = {1,1,1,1};
static int nVolume[4] = {15,15,15,15};
static int nOutput[4] = {1,1,1,1};
static const int nVolumeTable[16] = {
   32767, 26028, 20675, 16422, 13045, 10362,  8231,  6538,
    5193,  4125,  3277,  2603,  2067,  1642,  1304,     0
};

// change the frequency counter on a channel
// chan - channel 0-3 (3 is noise)
// freq - frequency counter (0-1023) or noise code (0-7)
void setfreq(int chan, int freq) {
    if ((chan < 0)||(chan > 3)) return;

    if (chan==3) {
        freq &= 0x07;
        nRegister[3] = freq;
        nCounter[3] = 0;
        LFSR = 0x4000;  // (15 bit)
    } else {
        freq &= 0x3ff;
        if (freq == 0) freq = 0x400;
        nRegister[chan] = freq;
        // don't update the counters, let them run out on their own
    }
}

// change the volume on a channel
// chan - channel 0-3
// vol - 0 (loudest) to 15 (silent)
void setvol(int chan, int vol) {
    if ((chan < 0)||(chan > 3)) return;
    nVolume[chan] = vol & 0xf;
}

// sn chip simulation - write data (standard SN76489 register-write protocol)
void snsim(unsigned char c) {
    static unsigned short latch_byte = 0;
    static unsigned short oldFreq[3] = {0,0,0};

    if (c&0x80) {
        latch_byte = c;
    }

    switch (c&0xf0) {
    case 0x90: case 0xb0: case 0xd0: case 0xf0:    // volume
        setvol((c&0x60)>>5, c&0x0f);
        break;

    case 0xe0:                                     // noise type
        setfreq(3, c&0x07);
        break;

    case 0x80: case 0xa0: case 0xc0:                // tone frequency (low bits)
        {
            int nChan = (latch_byte&0x60)>>5;
            oldFreq[nChan] &= 0xfff0;
            oldFreq[nChan] |= c&0x0f;
            setfreq(nChan, oldFreq[nChan]);
        }
        break;

    default:                                        // data for whatever is latched
        {
            int nChan = (latch_byte&0x60)>>5;
            if (latch_byte&0x10) {
                setvol(nChan, c&0x0f);
            } else if (nChan==3) {
                setfreq(3, c&0x07);
            } else {
                oldFreq[nChan] &= 0xf;
                oldFreq[nChan] |= (c&0x3f)<<4;
                setfreq(nChan, oldFreq[nChan]);
            }
        }
        break;
    }
}

// Chosen so GBA_CLOCK/16/SAMPLE_RATE lands almost exactly on an integer
// (3579545/16/16000 == 13.98...), keeping pitch error under 0.2%.
#define SAMPLE_RATE 16000
#define CLOCKS_PER_SAMPLE 14

static void synthesize(short *buf, int nSamples) {
    int noiseCnt;
    switch (nRegister[3]&0x03) {
        case 0: noiseCnt=0x10; break;
        case 1: noiseCnt=0x20; break;
        case 2: noiseCnt=0x40; break;
        case 3: noiseCnt=(nRegister[2]?nRegister[2]:0x400); break;
        default: noiseCnt=0x10; break;
    }

    while (nSamples) {
        for (int idx=0; idx<3; idx++) {
            if (nRegister[idx] <= CLOCKS_PER_SAMPLE) {
                // too high to represent at this sample rate - mute rather than alias
                nOutput[idx] = 0;
            } else {
                if (nOutput[idx] == 0) nOutput[idx] = 1;
                nCounter[idx] -= CLOCKS_PER_SAMPLE;
                if (nCounter[idx] <= 0) {
                    nCounter[idx] += nRegister[idx];
                    nOutput[idx] *= -1;
                }
            }
        }

        nCounter[3] -= CLOCKS_PER_SAMPLE;
        if (nCounter[3] <= 0) {
            nCounter[3] += noiseCnt;
            nNoisePos *= -1;
            if (nNoisePos > 0) {
                int in = 0;
                if (nRegister[3]&0x4) {
                    if (((LFSR&0x0002)>>1)^(LFSR&0x0001)) in = 0x4000;
                    if (LFSR&0x01) {
                        if (nOutput[3] == 0) nOutput[3] = 1; else nOutput[3] *= -1;
                    }
                } else {
                    if (LFSR&0x0001) { in = 0x4000; nOutput[3] = 1; } else nOutput[3] = 0;
                }
                LFSR >>= 1;
                LFSR |= in;
            }
        }

        nSamples--;

        int output = nOutput[0]*nVolumeTable[nVolume[0]] +
                     nOutput[1]*nVolumeTable[nVolume[1]] +
                     nOutput[2]*nVolumeTable[nVolume[2]] +
                     nOutput[3]*nVolumeTable[nVolume[3]];
        // max |output| is ~131068 (4 channels * 32767) - shift by 2 instead of
        // GBA's 2+8 since we're filling 16-bit samples, not 8-bit DMA bytes
        output >>= 2;
        *(buf++) = (short)output;
    }
}

static AudioStream musicStream;
static bool musicStreamValid = false;

static void musicAudioCallback(void *buffer, unsigned int frames) {
    synthesize((short*)buffer, (int)frames);
}

// sn chip sim init - sets up the music AudioStream (call before gbastartaudio())
void gbasninit() {
    if (!IsAudioDeviceReady()) InitAudioDevice();

    setvol(0,15); setvol(1,15); setvol(2,15); setvol(3,15);   // mute the simulated generators

    if (!musicStreamValid) {
        musicStream = LoadAudioStream(SAMPLE_RATE, 16, 1);
        SetAudioStreamCallback(musicStream, musicAudioCallback);
        musicStreamValid = true;
    }
}

// start/stop the music AudioStream (DMA1/FIFO_A equivalent)
void gbastartaudio() {
    if (musicStreamValid) PlayAudioStream(musicStream);
}
void gbastopaudio() {
    if (musicStreamValid) StopAudioStream(musicStream);
}

// no-op: musicAudioCallback() above is pulled by raylib's own audio thread
// whenever it needs more samples - there's no interrupt-driven refill to do.
void snupdateaudio() { }

void raylibSetMusicVolume(float v) {
    if (musicStreamValid) SetAudioStreamVolume(musicStream, v);
}

//*********************
// raw PCM sound effects ("channel B" / Direct Sound B equivalent)
//*********************

#define SFX_CACHE_SIZE 16
static struct { const unsigned char *ptr; Sound snd; } sfxCache[SFX_CACHE_SIZE];
static int sfxCacheCount = 0;
static Sound currentSfx;
static bool currentSfxValid = false;
static float sfxVolume = 1.0f;

// sfx data is static const for the program's lifetime, so the pointer alone
// is a stable cache key - lazily build a Sound the first time each one plays.
static Sound getOrLoadSfx(const unsigned char *pSnd, int len) {
    for (int i = 0; i < sfxCacheCount; i++) {
        if (sfxCache[i].ptr == pSnd) return sfxCache[i].snd;
    }

    Wave w = {0};
    w.frameCount = (unsigned int)len;
    w.sampleRate = 8000;   // GBA SFX are 8khz raw 8-bit unsigned (see music.c)
    w.sampleSize = 8;
    w.channels = 1;
    w.data = (void*)pSnd;
    Sound s = LoadSoundFromWave(w);

    if (sfxCacheCount < SFX_CACHE_SIZE) {
        sfxCache[sfxCacheCount].ptr = pSnd;
        sfxCache[sfxCacheCount].snd = s;
        sfxCacheCount++;
    }
    return s;
}

// equivalent of music.c's startsfx() hardware path (DMA2+Timer0+FIFO_B)
void raylibStartSfx(const unsigned char *pSnd, int len) {
    if (!IsAudioDeviceReady()) InitAudioDevice();

    if (currentSfxValid) StopSound(currentSfx);
    currentSfx = getOrLoadSfx(pSnd, len);
    currentSfxValid = true;
    SetSoundVolume(currentSfx, sfxVolume);
    PlaySound(currentSfx);
}

void raylibStopSfx() {
    if (currentSfxValid) StopSound(currentSfx);
}

void raylibSetSfxVolume(float v) {
    sfxVolume = v;
    if (currentSfxValid) SetSoundVolume(currentSfx, v);
}

//*********************
// song player (CPlayer.c - decodes the compressed song format, calls snsim())
//*********************

// USE_SN_PSG is passed via -D on the command line (see Makefile.pc), same as
// the GBA build's Makefile - matches WRITE_BYTE_TO_SOUND_CHIP below.
#define WRITE_BYTE_TO_SOUND_CHIP(mutes,chan,x) \
    if (((mutes)&(0x80>>((chan)-1)))==0) snsim(x);

#include "CPlayer.c"

#endif // RAYLIB
