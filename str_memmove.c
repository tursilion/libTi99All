// String code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

#include "string.h"

// needs to copy in the right direction to be overlap safe
void *memmove(void *dest, const void *src, int cnt) {
    char *d = (char*)dest;
    char *s = (char*)src;

    if ((d == s) || (cnt == 0)) {
        // nothing to do anyway
        return dest;
    }

    if ((d < s)||(s+cnt < d)) {
        return memcpy(dest, src, cnt);
    }

    // need to do a backward copy
    d+=cnt-1;
    s+=cnt-1;
    while (cnt > 0) {
        *(d--) = *(s--);
        --cnt;
    }
    return dest;
}
