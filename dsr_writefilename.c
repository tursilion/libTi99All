// DSR interface code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

#include "vdp.h"
#include "files.h"
#include "string.h"

#ifdef TI99

// parses a filename to find the split between device and file, and also
// checks lengths and manages long filename support for SBR functions
// Returns the index of the first '.' or 0 if not found (usually an error)
// if bTest is 0, then does not test or strip, and the return value should be ignored
unsigned char dsr_writefilename(const char *szPath, unsigned int vdp, int bTest) {
    // find the device separator so we know the length of the filename
    int len = 0;
    int dot = 0;

    if (bTest) {
        while ((szPath[dot]) && (szPath[dot] != '.')) ++dot;
        if (szPath[dot] != '.') {
            return 0;
        }

        len = strlen(&szPath[dot+1]);
        if ((dot < 4) || (len < 1)) {
            // invalid for sure...
            return 0;
        }
    } else {
        len = strlen(szPath);
        if (len < 1) {
            return 0;
        }
        --dot;  // warning: dot is now before the buffer to account for +1s below
    }

    if (len > 10) {
        // need to use long filename semantics
        VDP_SET_ADDRESS_WRITE(vdp);
        VDPWD(' ');
        VDPWD(len);
        for (int i=0; i<len; ++i) {
            VDPWD(szPath[i+dot+1]);
        }
    } else {
        // short enough to fit
        vdpmemset(vdp, ' ', 10);
        vdpmemcpy(vdp, &szPath[dot+1], len);
    }

    return dot;
}

#else

unsigned char dsr_writefilename(const char *szPath, unsigned int vdp, int bTest) {
    return 0;
}

#endif
