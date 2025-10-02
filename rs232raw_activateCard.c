// RS232 code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

#include "rs232.h"

// turns off the card and the card LED
void rs232raw_activateCard(int card) {
#ifdef TI99
    __asm__  volatile (
        "mov %0,r12\n\tsbo 0\n\rsbo 7" : : "r"(card) : "r12" 
    );
#endif
#ifdef COLECO
    (void)card;
#endif
#ifdef GBA
    (void)card;
#endif
#ifdef CLASSIC99
    (void)card;
#endif
}
