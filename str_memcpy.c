// String code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

void *memcpy(void *dest, const void *src, int cnt) {
    
  if ((dest == src) || (cnt == 0)) {
    // nothing to do anyway
    return dest;
  }
    
#ifdef TI99
    // speed up aligned copies. We could check if it's possible to align, but not today.
    if ( ( (((unsigned int)dest)&1) == 0) && ( (((unsigned int)src)&1) == 0) ) {
        // do a 16 bit copy instead
        unsigned int *d = (unsigned int *)dest;
        unsigned int *s = (unsigned int *)src;
        while (cnt > 1) {
            *(d++) = *(s++);
            cnt -= 2;
        }
    }
#endif

    char *d = (char*)dest;
    char *s = (char*)src;
    while (cnt > 0) {
        *(d++) = *(s++);
        --cnt;
    }
    return dest;
}
