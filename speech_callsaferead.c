#include "speech.h"

#ifdef TI99
unsigned char __attribute__((noinline)) call_safe_read() {
  READ_WITH_DELAY();
  return SPEECH_BYTE_BOX;
}

#endif

#ifdef COLECO
#endif

#ifdef GBA
#endif
