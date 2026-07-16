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
// VDP buffer must be large enough to contain the path in full plus two bytes.
static unsigned char dsr_mkdir_shared(const char *szPath, unsigned int vdp, int bRemove) {
    // first we have to set the current path to the existing folder
    // if more than 39 characters, use the long filename extension

    // Note: can't use writefilename because that enforces 10 characters from the start,
    // we have a longer buffer and need to work backwards
    int separator = strlen(szPath);
    if (separator < 4) {
        // invalid for sure...
        return DSR_ERR_BADATTRIBUTE;
    }
    --separator;    // start before the NUL
    while ((separator > 3) && (szPath[separator] != '.')) {
        --separator;
    }
    // there MUST be a dot in there somewhere... use "WDS." for root folder.
    if (szPath[separator] != '.') {
        return DSR_ERR_BADATTRIBUTE;
    }

    unsigned char ret = dsr_setpath(szPath, separator+1, vdp);
    if (ret) {
        return ret;
    }

    // now we need to actually make the subfolder
    // here we do NOT include len because we do NOT want the terminating NUL
    ++separator;
    int len = strlen(&szPath[separator]);
    if (len > 10) {
        // need to use long filename semantics
        VDP_SET_ADDRESS_WRITE(vdp);
        VDPWD(' ');
        VDPWD(len);
        for (int i=0; i<len; ++i) {
            VDPWD(szPath[i+separator]);
        }
    } else {
        // short enough to fit
        vdpmemset(vdp, ' ', 10);
        vdpmemcpy(vdp, &szPath[separator], len);
    }

    // set the unit
    SBR_UNIT_NUMBER = szPath[3] - '0';
    SBR_PATH_PTR = vdp;
    SBR_ERROR_RET = 0;

    unsigned char code = getsbrbase(szPath) | (bRemove ? SBR_RMDIR : SBR_MKDIR);
    if (sbrlnk(code) != DSR_ERR_NONE) {
        return DSR_ERR_DSRNOTFOUND;
    }

    // this is the real return I want to send
    ret = sbr_error_check(szPath);

    // but first I want to reset the path to prevent confusing errors
    // just going to do it blindly... for better or for worse...
    // We know all valid paths are like "DSK1." - 5 characters.
    dsr_setpath(szPath, 5, vdp);

    return ret;
}

// would not normally combine these, but the wrappers are so small...
unsigned char dsr_mkdir(const char *szPath, unsigned int vdp) {
    return dsr_mkdir_shared(szPath, vdp, 0);
}
unsigned char dsr_rmdir(const char *szPath, unsigned int vdp) {
    return dsr_mkdir_shared(szPath, vdp, 1);
}


#else 
unsigned char dsr_mkdir(const char *szPath, unsigned int vdp) { return DSR_ERR_DSRNOTFOUND; }
unsigned char dsr_rmdir(const char *szPath, unsigned int vdp) { return DSR_ERR_DSRNOTFOUND; }

#endif
