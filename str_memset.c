// String code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

void *memset(void *dest, int src, int cnt) {
#ifdef TI99
    // speed up aligned sets. We could check if it's possible to align, but not today.
    if ( (((unsigned int)dest)&1) == 0) {
        // do a 16 bit set instead
        src = src | (src<<8);
        unsigned int *d = (unsigned int *)dest;
        while (cnt > 1) {
            *(d++) = src;
            cnt -= 2;
        }
    }
#endif

  char *d = (char*)dest;
  while (cnt > 0) {
    *(d++) = src;
    --cnt;
  }
  return dest;
}
