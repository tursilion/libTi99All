// DSR interface code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

#include "vdp.h"
#include "files.h"
#include "string.h"

#ifdef TI99

// you must pass a complete path, not a relative one. Note that relatively few
// devices support subdirectories. Uses set current path first, as required by the spec.
// Supports long filenames but note that even fewer devices support them.
// Does NOT support CPU buffers. How complex do we need to make this? ;)
// VDP buffer must be large enough to contain the path in full plus two more bytes 
// in case of LFN (and one more even if not, so just say two.)
// We'll try to determine the correct subprogram number by name, but bleh.
// I guess they did that to make dsr chaining easier, since older cards did
// not foresee the arrival of newer ones.
// We specify the len so the string doesn't need to be modified for mkdir, but
// if len is 0 we'll do a strlen for you.
unsigned char dsr_setpath(const char *szPath, unsigned int len, unsigned int vdp) {
    // if more than 39 characters, use the long filename extension
    if (len == 0) {
        len = strlen(szPath);
    }
    if (len < 4) {
        // invalid for sure...
        return DSR_ERR_BADATTRIBUTE;
    }

    // first try to set the base path - ONLY this function allows more than 10 characters
    // NOTE NOTE NOTE NOTE!!! THIS IS A PASCAL STYLE STRING WITH A LENGTH BYTE!
    // So we basically write it the same either way but without the space if it's "short"...
    VDP_SET_ADDRESS_WRITE(vdp);
    if (len > 39) {
        // need to use long filename semantics
        VDPWD(' ');
    }
    VDPWD(len);
    for (int i=0; i<len; ++i) {
        VDPWD(szPath[i]);
    }

    // set the unit
    SBR_UNIT_NUMBER = szPath[3] - '0';
    SBR_PATH_PTR = vdp;
    SBR_ERROR_RET = 0;

    unsigned char code = getsbrbase(szPath) | SBR_SETPATH;
    if (sbrlnk(code) != DSR_ERR_NONE) {
        return DSR_ERR_DSRNOTFOUND;
    }

    return sbr_error_check(szPath);
}

#else 
unsigned char dsr_setpath(const char *szPath, unsigned int vdp) { }

#endif
