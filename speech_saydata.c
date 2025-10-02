#include "speech.h"

#ifdef TI99

void say_data(const char* addr, int len) {
  struct LpcPlaybackCtx ctx;
  ctx.addr = (char*) addr;
  ctx.remaining = len;

  speech_start(&ctx);
  while(ctx.remaining > 0) {
    // Wait for VDP Interrupt via CRU
    __asm__ volatile (
      "clr r12\n\t"
      "tb 2\n\t"
      "jeq -4\n\t"
      "movb @>8802,r12"
      : : : "r12"
    );

    speech_continue(&ctx);
  }
}

#endif

#ifdef COLECO
void say_data(const char* addr, int len) { (void)addr; (void)len; }
#endif

#ifdef GBA
void say_data(const char* addr, int len) { (void)addr; (void)len; }
#endif

#ifdef CLASSIC99
void say_data(const char* addr, int len) { (void)addr; (void)len; }
#endif
