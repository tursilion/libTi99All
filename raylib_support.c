// RAYLIB (Linux/PC) support for libTi99All - by Tursi aka Mike Brent
// This code and library released into the Public Domain
// You can copy this file and use it at will ;)
//
// This is the PC equivalent of gba_support.c. Two things live here:
//
// 1) A virtual 16KB 9918A/F18A VDP (vdp_ram/vdp_reg), exactly like the GBA target's,
//    so the portable vdp.h/f18a.h API (vdpmemcpy, sprite(), loadpal_f18a, etc) works.
//    Game logic writes pattern/sprite/color data here.
//
// 2) A tiny software "GBA PPU" that composites a frame from the *same* fake GBA
//    hardware surface the real GBA port pokes directly (REG_DISPCNT, BG_RAM_BASE,
//    OAM_BASE, SPR_RAM_BASE, the palettes) and presents it via raylib. We get this
//    for free for all the GBA-native game code (trampolines.c, boss draw, f18load,
//    scaling, attract/title/win screens, etc.) by mmap'ing real memory at the exact
//    physical addresses those files already use (0x05000000 palettes, 0x06000000
//    VRAM, 0x07000000 OAM) - see raylibInitHardware(). Nothing in those files needs
//    to change; they're poking "real" memory again, just on a PC instead of a GBA.

#define _GNU_SOURCE
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <raylib.h>

// rlgl low-level functions used for the perspective quad - forward-declared
// here to avoid the rlgl.h / raylib.h header interaction on some builds.
#define RL_QUADS 0x0007
extern int  rlCheckRenderBatchLimit(int vCount);
extern void rlSetTexture(unsigned int id);
extern void rlBegin(int mode);
extern void rlEnd(void);
extern void rlColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
extern void rlNormal3f(float x, float y, float z);
extern void rlTexCoord2f(float x, float y);
extern void rlVertex2f(float x, float y);
extern void rlVertex3f(float x, float y, float z);
extern void rlDisableBackfaceCulling(void);
extern void rlEnableBackfaceCulling(void);

#include <vdp.h>
#include <tursigb.h>
#include <setjmp.h>

// vdp.h redirects the game's calls to exit() into raylibSoftReset() (see below) -
// this file's own exit() calls (fatal init errors, window-close handling) need
// to be the real thing.
#undef exit

#define SCREEN_W 192
#define SCREEN_H 256
#define RENDER_SCALE 4
// GBA MODE4 (bitmap) scanline stride is fixed at 240 pixels by hardware.
// The framebuffer is wider/taller; pixels outside the bitmap show backdrop.
#define GBA_BITMAP_W 240
#define GBA_BITMAP_H 160

extern jmp_buf raylibRebootPoint;
void raylibSoftReset(int n) {
    (void)n;
    longjmp(raylibRebootPoint, 1);
}

//*********************
// fake GBA hardware backing store
//*********************

// map a fixed-address region so every literal 0x0500_0000 / 0x0600_0000 / 0x0700_0000 /
// 0x0400_0000 access in the borrowed GBA-native code lands on real memory.
static void mapFixed(uintptr_t addr, size_t len, const char *name) {
    void *p = mmap((void*)addr, len, PROT_READ|PROT_WRITE,
                    MAP_FIXED_NOREPLACE|MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED || (uintptr_t)p != addr) {
        fprintf(stderr, "raylib_support: failed to map %s at %p (got %p): %s\n",
                name, (void*)addr, p == MAP_FAILED ? NULL : p, strerror(errno));
        exit(1);
    }
}

// some game code stores/reads a real (GBA = 4 byte) pointer at the very top of
// a region (e.g. INT_VECTOR at the last word of IWRAM) - on this 64-bit host
// "void*" is 8 bytes, so that same access overruns the nominal GBA region size
// by a few bytes. Pad every mapping by a page so that harmlessly lands on
// still-mapped (if unused) memory instead of faulting.
#define SAFETY_MARGIN 0x1000

void raylibInitHardware() {
    mapFixed(0x02000000, 0x40000+SAFETY_MARGIN, "EWRAM");
    mapFixed(0x03000000, 0x08000+SAFETY_MARGIN, "IWRAM");
    mapFixed(0x04000000, 0x10000+SAFETY_MARGIN, "IO registers");
    mapFixed(0x05000000, 0x00400+SAFETY_MARGIN, "palette RAM");
    // VRAM gets extra padding: some game code (e.g. processFlame()'s flame-sprite
    // mapping) derives a VRAM offset from flameOffset before it's initialized,
    // underflowing to a ~64KB-too-far address. On real GBA hardware that just
    // aliases into a VRAM mirror (no MMU, no fault); here it needs real backing
    // store so it doesn't segfault instead.
    mapFixed(0x06000000, 0x18000+0x20000+SAFETY_MARGIN, "VRAM (BG+OBJ tiles)");
    mapFixed(0x07000000, 0x00400+SAFETY_MARGIN, "OAM");
    mapFixed(0x0e000000, 0x10000+SAFETY_MARGIN, "cartridge SRAM (high scores)");

    // sensible defaults - REG_KEYINPUT is active-low, default to "nothing pressed"
    REG_KEYINPUT = 0xffff;
}

// DMA-backed on real GBA, just real copies/fills here - no hardware to drive
void fastcopy16(u32 pDest, u32 pSrc, u32 nCount) { memcpy((void*)(uintptr_t)pDest, (void*)(uintptr_t)pSrc, nCount); }
void fastset16(u32 pDest, u16 value, u32 nCount) {
    for (u32 i = 0; i < nCount; i += 2) *(u16*)(uintptr_t)(pDest+i) = value;
}
void fastcopy(u32 pDest, u32 pSrc, u32 nCount) { memcpy((void*)(uintptr_t)pDest, (void*)(uintptr_t)pSrc, nCount); }
void fastset(u32 pDest, u32 value, u32 nCount) {
    for (u32 i = 0; i < nCount; i += 4) *(u32*)(uintptr_t)(pDest+i) = value;
}

//*********************
// virtual VDP (mirrors gba_support.c's gbaVDPxx exactly)
//*********************

volatile unsigned char raylibIntCtrlDummy;
volatile unsigned char raylibRegKscanMirrorDummy;

unsigned char vdp_ram[16384];
unsigned char vdp_reg[64];          // to cover F18A
unsigned int  vdp_addr;
unsigned char vdp_prefetch;
unsigned char vdp_flipflop;
volatile unsigned char vdp_status;  // set true once per presented frame

unsigned char raylibVDPRD() {
    unsigned char ret = vdp_prefetch;
    vdp_prefetch = vdp_ram[vdp_addr];
    ++vdp_addr;
    vdp_addr &= 0x3fff;
    vdp_flipflop = 0;
    return ret;
}
unsigned char raylibVDPST() {
    unsigned char ret = vdp_status;
    vdp_status = 0;
    vdp_flipflop = 0;
    return ret;
}
unsigned char raylibVDPSTCRU() {
    return vdp_status;
}
void raylibVDPWA(unsigned char x) {
    if (vdp_flipflop) {
        vdp_addr = (vdp_addr & 0xff) | ((x & 0x3f) << 8);
        if (x & 0x80) {
            int reg = x & 0x3f;
            vdp_reg[reg] = vdp_addr & 0xff;
        }
        if ((x & 0xc0) == 0) {
            vdp_prefetch = vdp_ram[vdp_addr];
            ++vdp_addr;
            vdp_addr &= 0x3fff;
        }
        vdp_flipflop = 0;
    } else {
        vdp_addr = (vdp_addr & 0xff00) | x;
        vdp_flipflop = 1;
    }
}
void raylibVDPWD(unsigned char x) {
    vdp_prefetch = x;
    vdp_ram[vdp_addr] = x;
    ++vdp_addr;
    vdp_addr &= 0x3fff;
    vdp_flipflop = 0;
}

//*********************
// sound - stubbed for now, real SN76489 + raylib AudioStream comes later
//*********************

void raylibSnSim(unsigned char x) {
    (void)x;
}

// no-op: raylibPresentFrame() always composites fresh, there's nothing to "turn on"
void setGBAAutoRender(unsigned short mode) {
    (void)mode;
}

//*********************
// mini GBA PPU - composites MODE4 bitmap / MODE0 BG0 tiles + OAM sprites
//*********************

static inline Color gbaColor(u16 c) {
    Color out;
    out.r = (unsigned char)(((c)       & 0x1f) << 3);
    out.g = (unsigned char)(((c >> 5)  & 0x1f) << 3);
    out.b = (unsigned char)(((c >> 10) & 0x1f) << 3);
    out.a = 255;
    return out;
}

static Color framebuffer[SCREEN_H][SCREEN_W];

static void compositeBG2Bitmap() {
    // MODE4 supports page flipping (REG_DISPCNT's PAGE2 bit) between the two
    // GBA_BITMAP_W×GBA_BITMAP_H bitmap pages. Stride is GBA_BITMAP_W bytes
    // per scanline regardless of the wider framebuffer.
    const u8 *vram = (const u8*)((REG_DISPCNT & PAGE2) ? BG_RAM_PAGE2 : BG_RAM_BASE);
    const u16 *pal = (const u16*)BG_PALETTE;
    // simple whole-screen darken (used for the fadeingfx()-style fade-in/out effect)
    int dark = 16;
    if (REG_BLDCNT & DARKEN) {
        dark = 16 - (REG_BLDY & 0x1f);
        if (dark < 0) dark = 0;
        if (dark > 16) dark = 16;
    }
    // Fill entire (wider/taller) framebuffer with backdrop first.
    Color backdrop = gbaColor(pal[0]);
    if (dark != 16) {
        backdrop.r = (unsigned char)((backdrop.r * dark) / 16);
        backdrop.g = (unsigned char)((backdrop.g * dark) / 16);
        backdrop.b = (unsigned char)((backdrop.b * dark) / 16);
    }
    for (int y = 0; y < SCREEN_H; ++y)
        for (int x = 0; x < SCREEN_W; ++x)
            framebuffer[y][x] = backdrop;
    // Overlay the GBA bitmap (GBA_BITMAP_W×GBA_BITMAP_H, stride=GBA_BITMAP_W).
    // Clamp x to SCREEN_W: if SCREEN_W < GBA_BITMAP_W the right edge is cropped;
    // if SCREEN_W > GBA_BITMAP_W the extra columns were already filled with backdrop.
    int blit_w = GBA_BITMAP_W < SCREEN_W ? GBA_BITMAP_W : SCREEN_W;
    for (int y = 0; y < GBA_BITMAP_H; ++y) {
        for (int x = 0; x < blit_w; ++x) {
            Color c = gbaColor(pal[vram[y*GBA_BITMAP_W+x]]);
            if (dark != 16) {
                c.r = (unsigned char)((c.r * dark) / 16);
                c.g = (unsigned char)((c.g * dark) / 16);
                c.b = (unsigned char)((c.b * dark) / 16);
            }
            framebuffer[y][x] = c;
        }
    }
}

// draws a regular (non-affine) tiled BG - used for BG0 (and would work for BG1/BG3).
// skipBackdrop: if true, index-0 pixels are left untouched (so a layer drawn on top of
// another doesn't clobber it); if false, index-0 pixels are painted with the backdrop color
// (use this for whichever enabled BG is the back-most layer).
static void compositeRegularBG(unsigned short ctrl, int skipBackdrop) {
    const u8 *vram = (const u8*)BG_RAM_BASE;
    const u16 *pal = (const u16*)BG_PALETTE;
    int charBank = (ctrl >> 2) & 0x3;
    int mapBank = (ctrl >> 8) & 0x1f;
    int is256 = ctrl & COLORS_256;
    const u8 *tiles = vram + charBank*0x4000;
    const u16 *map = (const u16*)(vram + mapBank*0x800);

    if (!skipBackdrop) {
        Color backdrop = gbaColor(pal[0]);
        for (int y = 0; y < SCREEN_H; ++y) for (int x = 0; x < SCREEN_W; ++x) framebuffer[y][x] = backdrop;
    }

    for (int ty = 0; ty < SCREEN_H/8; ++ty) {
        for (int tx = 0; tx < SCREEN_W/8; ++tx) {
            u16 entry = map[ty*32+tx];
            int tileIdx = entry & 0x3ff;
            int hflip = entry & 0x0400;
            int vflip = entry & 0x0800;
            int palBank = (entry >> 12) & 0xf;

            const u8 *tile = tiles + tileIdx * (is256 ? 64 : 32);
            for (int py = 0; py < 8; ++py) {
                int sy = vflip ? (7-py) : py;
                for (int px = 0; px < 8; ++px) {
                    int sx = hflip ? (7-px) : px;
                    int idx;
                    if (is256) {
                        idx = tile[sy*8+sx];
                    } else {
                        unsigned char byte = tile[sy*4 + sx/2];
                        idx = (sx & 1) ? (byte >> 4) : (byte & 0xf);
                        if (idx) idx += palBank*16;
                    }
                    if (idx == 0) continue;  // transparent
                    int dx = tx*8+px, dy = ty*8+py;
                    if ((unsigned)dx < SCREEN_W && (unsigned)dy < SCREEN_H) {
                        framebuffer[dy][dx] = gbaColor(pal[idx]);
                    }
                }
            }
        }
    }
}

// draws BG2 in affine (rotation/scaling) mode - always 256-color, byte-indexed map.
// This is a single whole-frame matrix, not a true per-scanline HBLANK simulation - fine
// for menu/text screens where the game leaves BG2 at the identity transform (stretchoff()),
// but won't reproduce the per-row "stretch" effect used during live gameplay (stretchon(),
// driven by VCOUNT inside myInterruptProcess()'s real HBLANK handler).
static void compositeAffineBG2(unsigned short ctrl, int skipBackdrop) {
    const u8 *vram = (const u8*)BG_RAM_BASE;
    const u16 *pal = (const u16*)BG_PALETTE;
    int charBank = (ctrl >> 2) & 0x3;
    int mapBank = (ctrl >> 8) & 0x1f;
    int sizeClass = (ctrl >> 14) & 0x3;
    int wrap = ctrl & SCREEN_WRAP;
    int mapTiles = 16 << sizeClass;       // 16/32/64/128 tiles per side
    int mapPixels = mapTiles * 8;
    const u8 *tiles = vram + charBank*0x4000;
    const u8 *map = vram + mapBank*0x800;

    // BG2X/Y are 28-bit signed 20.8 fixed point; PA-PD are 16-bit signed 8.8 fixed point
    int32_t refX = ((int32_t)(REG_BG2X << 4)) >> 4;
    int32_t refY = ((int32_t)(REG_BG2Y << 4)) >> 4;
    int16_t pa = (int16_t)REG_BG2PA, pb = (int16_t)REG_BG2PB;
    int16_t pc = (int16_t)REG_BG2PC, pd = (int16_t)REG_BG2PD;

    if (!skipBackdrop) {
        Color backdrop = gbaColor(pal[0]);
        for (int y = 0; y < SCREEN_H; ++y) for (int x = 0; x < SCREEN_W; ++x) framebuffer[y][x] = backdrop;
    }

    for (int y = 0; y < SCREEN_H; ++y) {
        int32_t srcX = refX + y*pb;
        int32_t srcY = refY + y*pd;
        for (int x = 0; x < SCREEN_W; ++x, srcX += pa, srcY += pc) {
            int texX = srcX >> 8, texY = srcY >> 8;
            if (wrap) {
                texX &= (mapPixels-1);
                texY &= (mapPixels-1);
            } else if ((unsigned)texX >= (unsigned)mapPixels || (unsigned)texY >= (unsigned)mapPixels) {
                continue;  // outside the map - transparent
            }
            int tileIdx = map[(texY/8)*mapTiles + (texX/8)];
            const u8 *tile = tiles + tileIdx*64;
            int idx = tile[(texY&7)*8 + (texX&7)];
            if (idx == 0) continue;
            framebuffer[y][x] = gbaColor(pal[idx]);
        }
    }
}

// width/height in pixels, indexed by [shape][size]
static const int objDim[3][4] = {
    {8,16,32,64},   // shape 0 (square)
    {16,32,32,64},  // shape 1 (wide)
    {8,8,16,32},    // shape 2 (tall) - widths
};
static const int objDimH[3][4] = {
    {8,16,32,64},   // shape 0 (square) - heights
    {8,8,16,32},    // shape 1 (wide) - heights
    {16,32,32,64},  // shape 2 (tall) - heights
};

// %$%$32ing pcs. This many layers of emulation shouldn't even work. ;)
// NOTE: We are using the GBA scaling control bit as a sine bit for the new larger y axis
extern int spriteClipY;
static void compositeSprites() {
    const u16 *oam = (const u16*)OAM_BASE;
    const u8 *objTiles = (const u8*)SPR_RAM_BASE;
    const u16 *pal = (const u16*)SPR_PALETTE;

    // sprite 0 should end up drawn on top, like real OBJ priority for equal layer priority
    for (int n = 127; n >= 0; --n) {
        u16 a0 = oam[n*4+0];
        u16 a1 = oam[n*4+1];
        u16 a2 = oam[n*4+2];

        int affine = 0;     // a0 & 0x0100; (we are not using this flag anymore)
        int disable = a0 & 0x0200;
        if (!affine && disable) continue;   // hidden

        int shape = (a0 >> 14) & 0x3;
        int size = (a1 >> 14) & 0x3;
        if (shape == 3) continue;  // invalid
        int w = objDim[shape][size];
        int h = objDimH[shape][size];

        int sy = a0 & 0x1ff;    // extra bit used here for sign, instead of affine
        //if (sy >= 256-h && sy >= SCREEN_H) sy -= 256;
        if (sy&0x100) sy -= 512;

        int sx = a1 & 0x1ff;
        if (sx >= 512-w && sx >= SCREEN_W) sx -= 512;

        if (sy >= SCREEN_H || sy+h <= 0 || sx >= SCREEN_W || sx+w <= 0) continue;

        int is256 = a0 & 0x2000;
        int hflip = !affine && (a1 & 0x1000);
        int vflip = !affine && (a1 & 0x2000);
        int baseTile = a2 & 0x3ff;
        int palBank = (a2 >> 12) & 0xf;
        int tilesWide = w/8, tilesHigh = h/8;
        int unit = is256 ? 2 : 1;  // index units per 8x8 tile column (2D mapping)

        for (int py = 0; py < h; ++py) {
            int dy = sy+py;
            if ((unsigned)dy >= SCREEN_H) continue;
            if ((unsigned)dy < spriteClipY) continue;
            int spy = vflip ? (h-1-py) : py;
            int tileRow = spy/8, rowInTile = spy%8;
            for (int px = 0; px < w; ++px) {
                int dx = sx+px;
                if ((unsigned)dx >= SCREEN_W) continue;
                int spx = hflip ? (w-1-px) : px;
                int tileCol = spx/8, colInTile = spx%8;

                int tileIdx = baseTile + tileRow*32 + tileCol*unit;
                const u8 *tile = objTiles + tileIdx*32;
                int idx;
                if (is256) {
                    idx = tile[rowInTile*8+colInTile];
                } else {
                    unsigned char byte = tile[rowInTile*4 + colInTile/2];
                    idx = (colInTile & 1) ? (byte >> 4) : (byte & 0xf);
                    if (idx) idx += palBank*16;
                }
                if (idx == 0) continue;  // transparent
                framebuffer[dy][dx] = gbaColor(pal[idx]);
            }
        }
    }
}

// BG2 is regular-tiled in MODE0, affine (rotation/scale) in MODE1/MODE2
static void drawBG2(unsigned int mode, int skipBackdrop) {
    if (mode == MODE0) compositeRegularBG(REG_BG2CNT, skipBackdrop);
    else compositeAffineBG2(REG_BG2CNT, skipBackdrop);
}

static void compositeFrame() {
    unsigned int mode = REG_DISPCNT & 0x7;
    if ((mode == MODE4) && (REG_DISPCNT & BG2_ENABLE)) {
        compositeBG2Bitmap();
    } else {
        int bg0on = REG_DISPCNT & BG0_ENABLE;
        int bg2on = REG_DISPCNT & BG2_ENABLE;
        if (!bg0on && !bg2on) {
            for (int y = 0; y < SCREEN_H; ++y) for (int x = 0; x < SCREEN_W; ++x) framebuffer[y][x] = BLACK;
        } else {
            int bg0pri = REG_BG0CNT & 0x3, bg2pri = REG_BG2CNT & 0x3;
            // equal priority is a tie-break in BG0's favor (more in front) on real hardware
            int bg2InFront = bg2on && (!bg0on || bg2pri < bg0pri);
            if (bg2InFront) {
                if (bg0on) compositeRegularBG(REG_BG0CNT, 0);
                drawBG2(mode, bg0on);
            } else {
                if (bg2on) drawBG2(mode, 0);
                if (bg0on) compositeRegularBG(REG_BG0CNT, bg2on);
            }
        }
    }
    if (REG_DISPCNT & SPRITE_ENABLE) {
        compositeSprites();
    }
}

//*********************
// presentation / frame pump
//*********************

static Texture2D screenTexture;
static bool inited = false;

static void initPresentation() {
    InitWindow(SCREEN_W * RENDER_SCALE, SCREEN_H * RENDER_SCALE, "Super Space Acer");
    SetTargetFPS(60);
    Image img = GenImageColor(SCREEN_W, SCREEN_H, BLACK);
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    screenTexture = LoadTextureFromImage(img);
    UnloadImage(img);
    // Clamp so negative V coords (used by the perspective strip formula) show
    // the first texture row rather than wrapping to the bottom.
    SetTextureWrap(screenTexture, TEXTURE_WRAP_CLAMP);

    inited = true;
}

extern void raylibCallUserInt();
extern void gbastopaudio();

void raylibPresentFrame() {
    if (!inited) initPresentation();

    // live keyboard state for any direct REG_KEYINPUT reads (active low)
    unsigned short keys = 0xffff;
    if (IsKeyDown(KEY_Z) || IsKeyDown(KEY_PERIOD)) keys &= ~BTN_A;
    if (IsKeyDown(KEY_SPACE)) keys &= ~BTN_B;
    if (IsKeyDown(KEY_BACKSPACE)) keys &= ~BTN_SELECT;
    if (IsKeyDown(KEY_ENTER)) keys &= ~BTN_START;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_X)) keys &= ~BTN_RIGHT;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_S)) keys &= ~BTN_LEFT;
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_E)) keys &= ~BTN_UP;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_D)) keys &= ~BTN_DOWN;
    REG_KEYINPUT = keys;

    compositeFrame();
    UpdateTexture(screenTexture, framebuffer);

    extern unsigned short isStretched;
    SetTextureFilter(screenTexture, isStretched ? TEXTURE_FILTER_BILINEAR : TEXTURE_FILTER_POINT);
    BeginDrawing();
    ClearBackground(BLACK);
    // we shouldn't be reaching into the app for this... though I suppose could make it a feature...
    if (isStretched) {
        float sw = (float)GetScreenWidth();
        float sh = (float)GetScreenHeight();
        // Strips: 256 thin trapezoids taper horizontally from pinch inset at
        // the top to full width at the bottom. Affine UV error per strip is
        // sub-pixel at this count.
        const int STRIPS = 256;
        // pinch: pixels inset from each side at the top of the screen.
        //   sw*0.25 → top edge = 50% width (dramatic); sw*0.15 → 70%; sw*0.10 → 80%.
        // V is linear (skew=0) so V spans exactly 0..1 — bottom aspect is always
        // correct. Sprites at the top appear proportionally taller by 1/(1-2p/sw)
        // where p=pinch; reducing pinch reduces that top stretch.
        float pinch = sw * 0.15f;
        // V is linear: no squish at the bottom, some stretch at the top.
        rlSetTexture(screenTexture.id);
        float texv = 0.0f;
        for (int i = 0; i < STRIPS; i++) {
            float t0 = (float)i       / STRIPS;
            float t1 = (float)(i + 1) / STRIPS;
            float step = 1.0f / STRIPS;
            float v0 = texv;
            float v1 = texv + step;
            texv = v1;
            float y0 = sh * t0,  y1 = sh * t1;
            float l0 = pinch * (1.0f - t0), l1 = pinch * (1.0f - t1);
            float r0 = sw - l0,             r1 = sw - l1;
            rlCheckRenderBatchLimit(4);
            rlBegin(RL_QUADS);
                rlColor4ub(255, 255, 255, 255);
                rlNormal3f(0.0f, 0.0f, 1.0f);
                rlTexCoord2f(0.0f, v0); rlVertex2f(l0, y0); // TL
                rlTexCoord2f(0.0f, v1); rlVertex2f(l1, y1); // BL
                rlTexCoord2f(1.0f, v1); rlVertex2f(r1, y1); // BR
                rlTexCoord2f(1.0f, v0); rlVertex2f(r0, y0); // TR
            rlEnd();
        }
        rlSetTexture(0);
    } else {
        // just copy it square
        Rectangle src = {0, 0, (float)screenTexture.width, (float)screenTexture.height};
        Rectangle dst = {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()};
        DrawTexturePro(screenTexture, src, dst, (Vector2){0,0}, 0.0f, WHITE);
    }
    EndDrawing();

    VDP_INT_COUNTER++;
    vdp_status = VDP_ST_INT;
    raylibCallUserInt();

    if (WindowShouldClose()) {
        fprintf(stderr, "Window should close.\n");
        gbastopaudio();     // stop music stream before tearing down audio
        CloseAudioDevice(); // joins the audio thread cleanly
        CloseWindow();
        _Exit(0);           // skip atexit/stdio flush - no cleanup can hang us
    }

}
