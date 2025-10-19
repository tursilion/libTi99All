
#include "vdp.h"

void charsetreverse() {
    char buf[8];    // pattern buffer

    for (unsigned char idx=0; idx<128; ++idx) {
        vdpmemread(gPattern+(idx<<3), buf, 8);
        for (unsigned char i2=0; i2<8; ++i2) {
            buf[i2] ^= 0xff;
        }
        vdpmemcpy(gPattern+(idx<<3), buf, 8);
    }
}
