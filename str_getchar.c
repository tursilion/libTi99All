#include <kscan.h>
#include <vdp.h>

// stdio compliant getchar() - wait for a keypress
// int return so that it works with sdcc's library

int getchar() {
    while (kscan(5) != 0xff) {
        // wait for key release
        VDP_INT_ENABLE;
        VDP_INT_DISABLE;
    }
    while (kscan(5) == 0xff) {
        // wait for key, allow interrupts    
        VDP_INT_ENABLE;
        VDP_INT_DISABLE;
    }
    return KSCAN_KEY;
}
