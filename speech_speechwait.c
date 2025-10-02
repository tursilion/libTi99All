#include "speech.h"

#ifdef TI99
void speech_wait() {
  delay_asm_42();
  delay_asm_12();
  SPEECH_BYTE_BOX = SPCH_STATUS_TALK;
  while (SPEECH_BYTE_BOX & SPCH_STATUS_TALK) {
    READ_WITH_DELAY();
  }
}
#endif

#ifdef COLECO
void speech_wait() { }
#endif

#ifdef GBA
void speech_wait() { }
#endif

#ifdef CLASSIC99
void speech_wait() { }
#endif
