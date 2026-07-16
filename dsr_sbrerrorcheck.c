// DSR interface code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

#include "files.h"
#include "string.h"

#ifdef TI99

// check the error code at >8350, adapting for the SCS bug
unsigned char sbr_error_check(const char *p) {
    // check for SCS
    unsigned char ret = SBR_ERROR_RET;
    if (0 == strnicmp(p, "SCS.", 4)) { 
        ret <<= 5;  // move to correct place, per Fred's docs
    }
    return GET_ERROR(ret);
}

#else 
unsigned char sbr_error_check(const char *p) { return DSR_ERR_DSRNOTFOUND; }

#endif
