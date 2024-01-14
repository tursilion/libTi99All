#include "vdp.h"
#include "conio.h"

// TODO: we don't want to rely on conio.h

// put a text character on the screen at the passed vram address,
// which is normally valid but might not be.
// This handles special cases for the modes that we know.
// I haven't decided if it should be abstracted, for now, 
// it's not.
void vsetchar(int pAddr, unsigned char ch) {
    // for all modes except 64, we just write the byte
    if (0 == (nTextFlags & TEXT_WIDTH_64)) {
	    VDP_SET_ADDRESS_WRITE(pAddr);
	    VDPWD=ch;
        
        // for 80 column color we also write a color byte
        if ((nTextFlags&(TEXT_FLAG_IS_F18A|TEXT_FLAG_HAS_ATTRIBUTES|TEXT_WIDTH_80)) == (TEXT_FLAG_IS_F18A|TEXT_FLAG_HAS_ATTRIBUTES|TEXT_WIDTH_80)) {
            VDP_SET_ADDRESS_WRITE(pAddr-gImage+gColor);
            VDPWD=conio_scrnCol;
        }
    } else {
        // 64 column has to translate the fake address into something real
        vdpchar64(pAddr, ch);
    }
}
