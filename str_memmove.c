// String code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

// needs to copy in the right direction to be overlap safe
void *memmove(void *dest, const void *src, int cnt) {
  char *d = (char*)dest;
  char *s = (char*)src;
  
  if (d == s) {
    // nothing to do anyway
    return;
  }
  
  if ((d < s)||(s+cnt < d)) {
    // forward copy safe
    while (cnt > 0) {
        *(d++) = *(s++);
        --cnt;
    }
  } else {
    // need to do a backward copy
    d+=cnt-1;
    s+=cnt-1;
    while (cnt > 0) {
        *(d--) = *(s--);
        --cnt;
    }
  }
  return dest;
}
