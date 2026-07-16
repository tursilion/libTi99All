// DSR interface code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

#include "files.h"
#include "string.h"

#ifdef TI99

// Path to the file (LFN will be used if longer than 10 characters)
// VDP address to use as a name buffer, separate one for data buffer
// first sector to read
// number of sectors to read - if 0 then first is ignored, and the file information is generated instead
// buf is the CPU buffer for the info block (if NULL, will be placed at 0x8352). It's 14 bytes
unsigned char readfilesect(const char *szPath, unsigned int vdppath, unsigned int vdpdata, unsigned int start, unsigned int count, char *buf) {
    // find the device separator so we know the length of the filename
    int dot = dsr_writefilename(szPath, vdppath, 1);
    if (dot == 0) {
        return DSR_ERR_BADATTRIBUTE;
    }

    // set the unit
    if (buf == 0) buf = (char*)SBR_DEFAULT_INFO_ADR;
    SBR_UNIT_NUMBER = szPath[dot-1] - '0';
    SBR_CODE = count;
    SBR_PATH_PTR = vdppath;
    SBR_INFO_PTR = (((unsigned int)buf)-0x8300)&0xff;
    
    DSR_FILE_INFO_STRUCT *pStruct = (DSR_FILE_INFO_STRUCT*)buf;
    pStruct->buffer = vdpdata;
    pStruct->first_sector = start;

    unsigned char code = getsbrbase(szPath) | SBR_FILESECTORREAD;
    if (sbrlnk(code) != DSR_ERR_NONE) {
        return DSR_ERR_DSRNOTFOUND;
    }

    if (SBR_CODE != count) {
        // then an error occured and not all sectors were read
        return DSR_ERR_DEVICEERROR;
    }

    return sbr_error_check(szPath);
}

#else 
unsigned char readfilesect(const char *szPath, unsigned int vdp, unsigned int start, unsigned int count, char *buf) { return DSR_ERR_DSRNOTFOUND; }

#endif
