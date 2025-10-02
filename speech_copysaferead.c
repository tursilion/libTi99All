#include "speech.h"

#ifdef TI99
// This will be copied to scratch pad for execution at SAFE_READ_PAD + 2
// it is 12 bytes long (gcc will insert the return)
void safe_read() {
  __asm__ volatile (
    "MOVB @>9000,@%0\n\t"
    "NOP\n\t"
    "NOP\n\t"
    : : "i" (SAFE_READ_PAD)
  );
}

void copy_safe_read() {
  int* source = (int*) safe_read;
  int* target = (int*) (SAFE_READ_PAD+2);
  int len = 12; // bytes
  while(len) {
    *target++ = *source++;
    len -= 2;
  }
}

#endif

#ifdef COLECO
#endif

#ifdef GBA
#endif

#ifdef CLASSIC99
#endif
