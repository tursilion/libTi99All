#include "speech.h"

#ifdef TI99
int detect_speech() {
  speech_reset();
  load_speech_addr(0);
  SPCHWT = 0x10;
  delay_asm_12();
  READ_WITH_DELAY();
  return SPEECH_BYTE_BOX == 0xAA;
}

#endif

#ifdef COLECO
int detect_speech() { return 0; }
#endif

#ifdef GBA
int detect_speech() { return 0; }
#endif
