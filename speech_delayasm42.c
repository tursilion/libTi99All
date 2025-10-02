#include "speech.h"

#ifdef TI99
void delay_asm_42() {
  __asm__ volatile (
    "\tLI r12,10\n"
    "loop%=\n\t"
    "DEC r12\n\t"
    "JNE loop%="
    : : : "r12"
  );
}

#endif

#ifdef COLECO
#endif

#ifdef GBA
#endif

#ifdef CLASSIC99
#endif
