#include "speech.h"

#ifdef TI99
void say_vocab(int phrase_addr) {
  load_speech_addr(phrase_addr);
  SPCHWT = SPCH_CMD_VOCAB; // say the phrase
  delay_asm_12();
}
#endif

#ifdef COLECO
void say_vocab(int phrase_addr) { (void)phrase_addr; }
#endif

#ifdef GBA
void say_vocab(int phrase_addr) { (void)phrase_addr; }
#endif
