#include "vdp.h"
#include "f18a.h"
#include "string.h"

// TODO: text modes should not rely on conio support if possible...
extern unsigned int conio_scrnCol; // conio_bgcolor.c

#if 0
static void gpu_scroll(void)
{
     __asm__ volatile (
// copy code starting at loop0 to VDP address >4000
   "li r0,>3f16\n"   // source
"	li r1,>4000\n"   // dest
"   li r2,>34/2\n"   // count
"loopx\n"
"	mov *r0+,*r1+\n"
"	dec r2\n"
"   jne loopx\n"
"   b @>4000\n"

// move 29 rows of chars up
"loop0\n"
"	li r0,80\n"
"	clr r1\n"
"	li r2,80*29/2\n"
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
// move 29 rows of colors up
"	li r0,0x1800+80\n"
"	li r1,0x1800\n"
"	li r2,80*29/2\n"
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

#ifndef GBA
static const unsigned char gpu_scroll80x30[] = {
    // this is the assembled code of the above routine
    0x02,0x00,0x3F,0x16,0x02,0x01,0x40,0x00,
    0x02,0x02,0x00,0x1A,0xCC,0x70,0x06,0x02,
    0x16,0xFD,0x04,0x60,0x40,0x00,0x02,0x00,
    0x00,0x50,0x04,0xC1,0x02,0x02,0x04,0x88,
    0xCC,0x70,0x06,0x02,0x16,0xFD,0x02,0x00,
    0x20,0x20,0x02,0x02,0x00,0x28,0xCC,0x40,
    0x06,0x02,0x16,0xFD,0x02,0x00,0x18,0x50,
    0x02,0x01,0x18,0x00,0x02,0x02,0x04,0x88,
    0xCC,0x70,0x06,0x02,0x16,0xFD,0x03,0x40,
    0x10,0xE6
};

static void fast_scrn_scroll_80color() {
    // uses the GPU for fastest scroll
    const int line = nTextEnd - nTextRow + 1;

    // use GPU code for fastest scrolling
    VDP_SET_REGISTER(0x38,1); // trigger GPU code

    VDP_SET_REGISTER(0x0f,2); // status register to read = SR2
    while (VDPST & 0x80);     // wait for GPU status to be idle
    VDP_SET_REGISTER(0x0f,0); // status register to read = SR0

    vdpmemset(nTextRow + gColor, conio_scrnCol, line);  // clear the last line

    return;
}
#endif
#ifdef GBA
extern unsigned char vdp_ram[16384] DATA_IN_EWRAM;
static void fast_scrn_scroll_80color() {
    // GBA easily scrolls the whole screen

    // text from 80 to 0, 29*80 bytes
    memmove(&vdp_ram[0], &vdp_ram[0x80], 29*80);
    // clear the bottom line with spaces
    memset(&vdp_ram[29*80], ' ', 80);

    // color from 0x1800+80 to 0x1800, 29*80 bytes
    memmove(&vdp_ram[0x1800+80], &vdp_ram[0x1800], 29*80);
    // set color externally
    memset(&vdp_ram[0x1800+29*80], conio_scrnCol, 80);
}
#endif

static void vdpchar80col(int pAddr, unsigned char ch) {
	VDP_SET_ADDRESS_WRITE(pAddr);
	VDPWD(ch);
        
    // for 80 column color we also write a color byte (TODO: remove CONIO dependence)
    VDP_SET_ADDRESS_WRITE(pAddr-gImage+gColor);
    VDPWD(conio_scrnCol);
}

void set_text80x30_color(void)
{
    unsigned char x = set_text80x30_color_raw();
    VDP_SET_REGISTER(VDP_REG_MODE1, x);
    VDP_REG1_KSCAN_MIRROR = x;
}

// requires F18A!!
unsigned char set_text80x30_color_raw() {
    // unlock the F18A (should be done before setting the mode)
    unlock_f18a();
    nTextRow = 80 * 29;
    nTextEnd = (80 * 30) - 1;
    nTextPos = nTextRow;
	nTextFlags = TEXT_FLAG_IS_F18A | TEXT_FLAG_HAS_ATTRIBUTES | TEXT_WIDTH_80 | TEXT_HEIGHT_30 | TEXT_CUSTOM_VSETCHAR;
    vsetchar = vdpchar80col;

    int unblank = VDP_MODE1_16K | VDP_MODE1_UNBLANK | VDP_MODE1_TEXT | VDP_MODE1_INT;
    VDP_SET_REGISTER(VDP_REG_MODE0, VDP_MODE0_80COL);
    VDP_SET_REGISTER(VDP_REG_MODE1, VDP_MODE1_16K | VDP_MODE1_TEXT);
    VDP_SET_REGISTER(0x31, 0x40); // set 30 row mode
    VDP_SET_REGISTER(0x32, 0x02); // set Position-based tile attributes
    VDP_SET_REGISTER(VDP_REG_SIT, 0x00);
    gImage = 0x000; // to 0x0960
    VDP_SET_REGISTER(VDP_REG_PDT, 0x02);
    gPattern = 0x1000; // to 0x1800
    VDP_SET_REGISTER(VDP_REG_CT, 0x60);
    gColor = 0x1800; // to 0x2160
    VDP_SET_REGISTER(VDP_REG_SAL, 0x20);
    gSprite = 0x0A00; // to 0x0B00
    VDP_SET_REGISTER(VDP_REG_SDT, 0x02); // sprites can use any of the font patterns.

    scrn_scroll = fast_scrn_scroll_80color;

    // sprites are active when F18A is unlocked
    VDP_SET_REGISTER(0x33, 0x00);
    vdpmemset(gSprite, 0xd0, 128);
    vdpmemset(gColor, conio_scrnCol, nTextEnd+1);	// clear the color table

#ifndef GBA
    // load GPU scroll function
    vdpmemcpy(0x3f00, (unsigned char*)gpu_scroll80x30, sizeof(gpu_scroll80x30));
	startgpu_f18a(0x3f00);
#endif

    return unblank;
}


