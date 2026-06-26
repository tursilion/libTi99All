// RAYLIB (Linux/PC) GBASNPlay.h - mirrors vgmcomp2's Players/libgbavgm2/GBASNPlay.h.
// gbasninit()/gbastartaudio()/gbastopaudio()/snupdateaudio()/snsim() are
// implemented in raylib_audio.c (a from-scratch port of GBAPlayerSN.c's chip
// emulator, feeding a raylib AudioStream instead of GBA DMA/FIFO hardware).
// StartSong()/StopSong()/SongLoop()/songNote/songVol come from the vendored,
// unmodified CPlayer.c (also included by raylib_audio.c).

#ifndef INCLUDE_GBASNPLAY_H
#define INCLUDE_GBASNPLAY_H

#include "CPlayer.h"

#define CALL_PLAYER_SN \
    SongLoop();

// helpful wrapper
#define isSNPlaying ((songNote[3]&SONGACTIVEACTIVE) != 0)

// call this to set up the initial registers
void gbasninit();

// call this to start or stop the audio output. When started, real hardware
// needs snupdateaudio() called every vblank - the RAYLIB backend doesn't,
// it's pulled directly by the audio thread, but the call is kept as a no-op
// so the (unmodified) call site in superspaceacer.c still links.
void gbastartaudio();
void gbastopaudio();
void snupdateaudio();

// snsim sends SN byte commands to the emulated chip
void snsim(unsigned char x);

#endif  // file include
