// DSR interface code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

#include "files.h"
#include "string.h"

#ifdef TI99

// Path can be just the device name, vdp address for buffer, and sector number (up to 32-bits for larger devices)
// sector_most will be zero on most TI devices. You hit 16MB before you need more. Only WDS, SCS and IDE support.
// only supports "DSKx" style device names, and assumes indexes match unit numbers.
// Longer or shorter device names will malfunction.
unsigned char readsector(const char *szPath, unsigned int vdp, unsigned int sector_low, unsigned int sector_high) {
    if (strlen(szPath) < 4) {
        // we don't do much more validation, we just assume it's ???X, where X is the drive number.
        // in setups where it's letters and stuff, we don't support that.
        return DSR_ERR_BADATTRIBUTE;
    }

    // set the unit
    int dot = 4;    // assumption
    SBR_UNIT_NUMBER = szPath[dot-1] - '0';
    SBR_CODE = SBR_SECTOR_READ;
    SBR_PATH_PTR = vdp;
    SBR_SECTOR_LOW = sector_low;
    SBR_SECTOR_HIGH = sector_high;
    
    unsigned char code = getsbrbase(szPath) | SBR_SECTOR;
    if (sbrlnk(code) != DSR_ERR_NONE) {
        return DSR_ERR_DSRNOTFOUND;
    }

    return sbr_error_check(szPath);
}

#else 
unsigned char readsector(const char *szPath, unsigned int vdp, unsigned int sector_low, unsigned int sector_high) { return DSR_ERR_DSRNOTFOUND; }

#endif
