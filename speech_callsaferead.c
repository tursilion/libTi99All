#include "speech.h"

#ifdef TI99
unsigned char __attribute__((noinline)) call_safe_read() {
  READ_WITH_DELAY();
  return SPEECH_BYTE_BOX;
}

#endif

#ifdef COLECO
#endif

#if defined(GBA) || defined(RAYLIB)
#endif

#ifdef CLASSIC99
unsigned char SPCHRD;
unsigned char SPCHWT;
#endif
