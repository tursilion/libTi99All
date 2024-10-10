// Note: this is an inline function provided only for cases where
// the compiler doesn't want to (or is told not to) inline. Apparently
// in 2013ish they decided it was stupid for the compiler, the tool making
// the decision anyway, not to compile a body in that case.
// Should be no overhead on this code if it's not needed.

#include "vdp.h"

// get a screen offset for 40x24 text mode
extern inline int VDP_SCREEN_TEXT(unsigned int r, unsigned int c);
