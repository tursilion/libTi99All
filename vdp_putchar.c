#include "vdp.h"

// using ints so that it works with SDCC's libs

// write a character with end of line detection
int putchar(int x) {
  if (x == '\n') {
    scrn_scroll();
    nTextPos = nTextRow;
  } else if (x == '\r') {
	nTextPos = nTextRow;
  } else if (x) {
    vsetchar(nTextPos, x&0xff);
    ++nTextPos;
    if (nTextPos > nTextEnd) {
      scrn_scroll();
      nTextPos = nTextRow;
    }
  }
  return x;
}

