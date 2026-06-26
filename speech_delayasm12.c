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

#if defined(GBA) || defined(RAYLIB)
#endif

#ifdef CLASSIC99
#endif
