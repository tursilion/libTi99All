// F18A - by Tursi - public domain

#include <vdp.h>
#include <f18a.h>

void startgpu_f18a(unsigned int adr) {
#ifdef TI99
    // the TI GCC currently doesn't optimize the MSB access very well, so this saves two 8 bit shifts
    // and a handful of instructions. Even using pointers the C code is still 4 instructions long
    __asm__ volatile ("movb %0,@>8c02" : : "r"(adr) : "cc");
    VDPWA = F18A_REG_GPUMSB|0x80;
    __asm__ volatile ("swpb %0\n\tmovb %0,@>8c02\n\tswpb %0" : : "r"(adr) : "cc");
    VDPWA = F18A_REG_GPULSB|0x80;
#else
    VDP_SET_REGISTER(F18A_REG_GPUMSB, (adr>>8));     // write MSB to register 0x36
    VDP_SET_REGISTER(F18A_REG_GPULSB, (adr&0xff));   // write LSB to register 0x37 (causes start)
#endif
}
