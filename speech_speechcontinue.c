#include "speech.h"

#ifdef TI99
void speech_continue(struct LpcPlaybackCtx* ctx) {
  // Next check for buffer low, and add upto 8 bytes at a time
  if (((int)call_safe_read()) & SPCH_STATUS_LOW) {
    // there is room for at least 8 bytes in the FIFO, so send upto 8
    int i = 8;
    while (i > 0 && ctx->remaining > 0) {
      SPCHWT = *ctx->addr++;
      ctx->remaining--;
      i--;
    }
  }
}
#endif

#ifdef COLECO
void speech_continue(struct LpcPlaybackCtx* ctx) { (void)ctx; }
#endif

#ifdef GBA
void speech_continue(struct LpcPlaybackCtx* ctx) { (void)ctx; }
#endif
