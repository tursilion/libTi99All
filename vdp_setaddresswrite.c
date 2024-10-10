// Note: this is an inline function provided only for cases where
// the compiler doesn't want to (or is told not to) inline. Apparently
// in 2013ish they decided it was stupid for the compiler, the tool making
// the decision anyway, not to compile a body in that case.
// Should be no overhead on this code if it's not needed.

#include "vdp.h"

extern inline void VDP_SET_ADDRESS_WRITE(unsigned int x);
