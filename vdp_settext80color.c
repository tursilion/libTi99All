#include "vdp.h"
#include "f18a.h"

// TODO: text modes should not rely on conio support if possible...

#if 0
static void gpu_scroll(void)
{
     __asm__(
// copy code starting at loop0 to VDP address >4000
   "li r0,>3f16\n"   // source
"	li r1,>4000\n"   // dest
"   li r2,>34/2\n"   // count
"loopx\n"
"	mov *r0+,*r1+\n"
"	dec r2\n"
"   jne loopx\n"
"   b @>4000\n"

// move 23 rows of chars up
"loop0\n"
"	li r0,80\n"
"	clr r1\n"
"	li r2,80*23/2\n"
"loop1\n"
"	mov *r0+,*r1+\n"
"	dec r2\n"
"	jne loop1\n"
// fill bottom row with spaces
"	li r0,>2020\n"
"   li r2,80/2\n"
"loop1a\n"
"	mov r0,*r1+\n"
"	dec r2\n"
"	jne loop1a\n"
// move 23 rows of colors up
"	li r0,0x800+80\n"
"	li r1,0x800\n"
"	li r2,80*23/2\n"
"loop2\n"
"	mov *r0+,*r1+\n"
"	dec r2\n"
"	jne loop2\n"
// stop gpu and restart when triggered
"	idle\n"
"	jmp loop0\n"
     :::
     "r0","r1","r2"
     );
}
#endif

static const unsigned char gpu_scroll[] = {
    // this is the assembled code of the above routine
    0x02,0x00,0x3F,0x16,0x02,0x01,0x40,0x00,
    0x02,0x02,0x00,0x1A,0xCC,0x70,0x06,0x02,
    0x16,0xFD,0x04,0x60,0x40,0x00,0x02,0x00,
    0x00,0x50,0x04,0xC1,0x02,0x02,0x03,0x98,
    0xCC,0x70,0x06,0x02,0x16,0xFD,0x02,0x00,
    0x20,0x20,0x02,0x02,0x00,0x28,0xCC,0x40,
    0x06,0x02,0x16,0xFD,0x02,0x00,0x08,0x50,
    0x02,0x01,0x08,0x00,0x02,0x02,0x03,0x98,
    0xCC,0x70,0x06,0x02,0x16,0xFD,0x03,0x40,
    0x10,0xE6
};

extern unsigned int conio_scrnCol; // conio_bgcolor.c

static void vdpchar80color(int pAddr, unsigned char ch) {
    VDP_SET_ADDRESS_WRITE(pAddr);
    VDPWD=ch;
    VDP_SET_ADDRESS_WRITE(pAddr-gImage+gColor);
    VDPWD=conio_scrnCol;
}

static void fast_scrn_scroll_80color() {
    const int line = nTextEnd - nTextRow + 1;

    // use GPU code for fastest scrolling
    VDP_SET_REGISTER(0x38,1); // trigger GPU code

    VDP_SET_REGISTER(0x0f,2); // status register to read = SR2
    while (VDPST & 0x80);    // wait for GPU status to be idle
    VDP_SET_REGISTER(0x0f,0); // status register to read = SR0

    extern unsigned int conio_scrnCol; // conio_bgcolor.c
    vdpmemset(nTextRow + gColor, conio_scrnCol, line);  // clear the last line

    return;
}

// requires F18A!!
unsigned char set_text80_color_raw() {
    // unlock the F18A (should be done before setting the mode)
    unlock_f18a();

    vdpchar = vdpchar80color;
    scrn_scroll = fast_scrn_scroll_80color;

    int unblank = VDP_MODE1_16K | VDP_MODE1_UNBLANK | VDP_MODE1_TEXT | VDP_MODE1_INT;
    VDP_SET_REGISTER(VDP_REG_MODE0, VDP_MODE0_80COL);
    VDP_SET_REGISTER(VDP_REG_MODE1, VDP_MODE1_16K | VDP_MODE1_TEXT);
    VDP_SET_REGISTER(VDP_REG_SIT, 0x00);	gImage = 0x000;
    VDP_SET_REGISTER(VDP_REG_PDT, 0x02);	gPattern = 0x1000;
    VDP_SET_REGISTER(VDP_REG_CT, 0x20);		gColor = 0x800;
    // sprites are active when F18A is unlocked
    VDP_SET_REGISTER(VDP_REG_SAL, 0x1800/0x80); gSprite = 0x1800; vdpchar_default(gSprite, 0xd0);
    vdpmemset(gColor, conio_scrnCol, nTextEnd+1);	// clear the color table
    VDP_SET_REGISTER(0x32, 0x02);  // set Position-based tile attributes
	
    nTextRow = 80 * 23;
    nTextEnd = (80 * 24) - 1;
    nTextPos = nTextRow;
	nTextFlags = TEXT_FLAG_IS_F18A | TEXT_FLAG_HAS_ATTRIBUTES | TEXT_WIDTH_80;

    // load GPU scroll function
    vdpmemcpy(0x3f00, gpu_scroll, sizeof(gpu_scroll));
	startgpu_f18a(0x3f00);

    return unblank;
}

void set_text80_color(void)
{
    unsigned char x = set_text80_color_raw();
    VDP_SET_REGISTER(VDP_REG_MODE1, x);
    VDP_REG1_KSCAN_MIRROR = x;
}
