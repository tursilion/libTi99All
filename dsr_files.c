// DSR interface code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

#include "files.h"

void files(unsigned char count) {
#ifdef TI99
    // requires the new count at DSR_FILES_COUNT, and has no return value
    DSR_FILES_COUNT = count;
    sbrlnk(SBR_FILES | SBR_PREFIX_DEFAULT);     // always default
#endif
// no coleco or GBA equivalent
}
