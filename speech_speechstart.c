#include "speech.h"

#ifdef TI99
void speech_start(struct LpcPlaybackCtx* ctx) {
  SPCHWT = SPCH_CMD_EXT; // say loose data in CPU RAM
  delay_asm_12();
  // Load upto the first 16 bytes
  int i = 16;
  while(i > 0 && ctx->remaining > 0) {
    SPCHWT = *ctx->addr++;
    ctx->remaining--;
    i--;
  }
}
#endif

#ifdef COLECO
void speech_start(struct LpcPlaybackCtx* ctx) { (void)ctx; }
#endif

#ifdef GBA
void speech_start(struct LpcPlaybackCtx* ctx) { (void)ctx; }
#endif
