#include "speech.h"

#ifdef TI99
void load_speech_addr(int phrase_addr) {
  SPCHWT = SPCH_CMD_ADDR | (char)(phrase_addr & 0x000F);
  SPCHWT = SPCH_CMD_ADDR | (char)((phrase_addr >> 4) & 0x000F);
  SPCHWT = SPCH_CMD_ADDR | (char)((phrase_addr >> 8) & 0x000F);
  SPCHWT = SPCH_CMD_ADDR | (char)((phrase_addr >> 12) & 0x000F);

  SPCHWT = SPCH_CMD_ADDR; // end addr
  delay_asm_42(); // I have observed on real hardware, without the delay the speech chip goes a little crazy.
}
#endif

#ifdef COLECO
#endif

#ifdef GBA
#endif
