#include "speech.h"

#ifdef TI99
void delay_asm_12() {
  __asm__ volatile (
    "NOP\n\t"
    "NOP"
  );
}

#endif

#ifdef COLECO
#endif

#ifdef GBA
#endif
