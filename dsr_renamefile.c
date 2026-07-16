// DSR interface code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

#include "files.h"
#include "string.h"

#ifdef TI99

// szPath is the complete path to the file to rename
// vdp is a pointer to a VDP buffer large enough for both filenames plus 4 extra bytes
// szNewName is the new name to set - no path attached
// isSubdir must be set if it is in a subdirectory so that setpath can be called
// isDir indicates we must call rename directory, rather than rename file
unsigned char dsr_renamefile(const char *szPath, unsigned int vdp, const char *szNewName, unsigned int isSubdir, unsigned int isDir) {
    // this gets us the first dot - but if it's a subdir, we'll have to try again.
    unsigned int dot = dsr_writefilename(szPath, vdp, 1);
    if (dot == 0) {
        return DSR_ERR_BADATTRIBUTE;
    }
    unsigned char device = szPath[dot-1] - '0';
    unsigned int pathLen = strlen(szPath);
    
    // check if we need to set a subdir
    if (isSubdir) {
        // we need to set the path and then tweak the starting point of szPath
        int pos = pathLen-1;
        while ((pos > 3) && (szPath[pos] != '.')) --pos;
        // we already checked there is at least one dot above, so this is fine
        unsigned char ret = dsr_setpath(szPath, pos+1, vdp);
        if (ret) {
            return ret;
        }
        // now write the filename again
        dsr_writefilename(&szPath[pos+1], vdp, 0);
    }

    // second filename is bare - need to add an extra 2 in case the first filename was LFN
    unsigned int vdp2 = vdp+pathLen+2;
    dsr_writefilename(szNewName, vdp2, 0);

    // set the unit
    SBR_UNIT_NUMBER = device;
    SBR_PATH_PTR = vdp2;
    SBR_PATH2_PTR = vdp;
    
    unsigned char code = getsbrbase(szPath) | (isDir ? SBR_RENAMEDIR : SBR_RENAME);
    if (sbrlnk(code) != DSR_ERR_NONE) {
        return DSR_ERR_DSRNOTFOUND;
    }

    // this is the real return I want to send
    unsigned char ret = sbr_error_check(szPath);

    if (isSubdir) {
        // but first I want to reset the path to prevent confusing errors
        // just going to do it blindly... for better or for worse...
        // We know all valid paths are like "DSK1." - 5 characters.
        dsr_setpath(szPath, 5, vdp);
    }

    return ret;
}

#else 
unsigned char dsr_renamefile(const char *szPath, unsigned int vdp, const char *szNewName, unsigned int isSubdir, unsigned int isDir) {
    return DSR_ERR_DSRNOTFOUND; 
}

#endif
