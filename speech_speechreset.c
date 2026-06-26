#include "speech.h"

#ifdef TI99
void speech_reset() {
  copy_safe_read();
  SPCHWT = SPCH_CMD_RESET;
  READ_WITH_DELAY(); // EA manual says this is necessary
}
#endif

#ifdef COLECO
void speech_reset() { }
#endif

#if defined(GBA) || defined(RAYLIB)
void speech_reset() { }
#endif

#ifdef CLASSIC99
void speech_reset() { }
#endif
