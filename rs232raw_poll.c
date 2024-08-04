// RS232 code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

#include "rs232.h"

// test if a byte is available at the specied serial port (returns 0 if not, other value for true)
int rs232raw_poll(int rawCRU) {
#ifdef TI99
    int ret;

    __asm__  volatile (
        "MOV %1,R12\n\tCLR %0\n\tTB 21\n\tJNE NCH\n\tSETO %0\nNCH" : "=rm" (ret) : "r" (rawCRU) : "r12" 
    );

    return ret;
#endif
#ifdef COLECO
    (void)rawCRU;
	return 0;
#endif
#ifdef GBA
    (void)rawCRU;
	return 0;
#endif
}
