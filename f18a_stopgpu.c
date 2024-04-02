// F18A - by Tursi - public domain

#include <vdp.h>
#include <f18a.h>

void stopgpu_f18a() {
    VDP_SET_REGISTER(F18A_REG_GPUCFG, 0);   // request the GPU to stop
}
