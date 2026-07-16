// DSR interface code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

#include "files.h"
#include "string.h"

#ifdef TI99

// Although SBRLNK doesn't typically require a PAB setup, we do one anyway so we can reuse
// the code in dsrlnkraw. You need about 12 bytes. You must have set up the rest of scratchpad
// for your particular subprogram before calling this.
unsigned char getsbrbase(const char *pszPath) {
    if (strnicmp(pszPath, "WDS.", 4) == 0) {
        return SBR_PREFIX_WDS;
    }
    if (strnicmp(pszPath, "SCS.", 4) == 0) {
        return SBR_PREFIX_WDS;
    }
    if (strnicmp(pszPath, "IDE.", 4) == 0) {
        return SBR_PREFIX_IDE;
    }
    if (strnicmp(pszPath, "HDX.", 4) == 0) {
        return SBR_PREFIX_WDS;
    }
    return SBR_PREFIX_DEFAULT;
}

#endif
