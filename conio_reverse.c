#include "conio.h"

// this is either 0x00 or 0x80 depending on how you call reverse()
unsigned char conio_reverseMask = 0;

// What you get out of reverse will depend on what you load in the character set
// Call charsetreverse() to take whatever you loaded in the first 127 chars and
// automatically reverse it for the second half
// Returns the old value
unsigned char reverse(unsigned char x) {
    unsigned char ret = conio_reverseMask;
    conio_reverseMask = x ? 0x80 : 0x00;
    return ret;
}
