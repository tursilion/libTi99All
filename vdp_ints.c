// VDP code for the TI-99/4A by Tursi
// You can copy this file and use it at will if it's useful

#ifdef CLASSIC99
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

/* Define SIO_UDP_CONNRESET if not available */
#ifndef SIO_UDP_CONNRESET
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
#endif
/* Link with ws2_32.lib */
#pragma comment(lib, "ws2_32.lib")
#endif

#include "vdp.h"
#include "f18a.h"
#include "sound.h"

#ifdef COLECO
#include <stdint.h>

// Coleco specific init code so that the nmi counts like the TI interrupt
// it also provides the user interrupt hook
// no other TI functionality is simulated (automotion, automusic, quit)
// but if needed, this is where it goes

// storage for VDP status byte
volatile unsigned char VDP_STATUS_MIRROR = 0;

// lock variable to prevent NMI from doing anything
// Z80 int is edge triggered, so it won't break us
// 0x80 = interrupt pending, 0x01 - interrupts enabled
// (This must be defined in the crt0.s)
//volatile unsigned char vdpLimi = 0;		// NO ints by default!

// address of user interrupt function
static void (*userint)() = 0;

// interrupt counter
volatile unsigned char VDP_INT_COUNTER = 0;

// used by vdpwaitvint - make certain it's reset
extern unsigned char gSaveIntCnt;

// May be called from true NMI or from VDP_INTERRUPT_ENABLE, depending on
// the flag setting when the true NMI fires.
void my_nmi() {
	// I think we're okay from races. There are only three conditions this is called:
	//
	// VDP_INTERRUPT_ENABLE - detects that vdpLimi&0x80 was set by the interrupt code.
	//						  Calls this code. But the interrupt line is still active,
	//						  so we can't retrigger until it's cleared by the VDPST read
	//						  below. At that time the vdpLimi is zeroed, and so we can't loop.
	//
	// nmi -				  detects that vdpLimi&0x01 is valid, and calls directly.
	//						  Again, the interrupt line is still active.
	//
	// maskable int (spinner)-detects that vdpLimi&0x80 was set by the interrupt code.
	//                        this one might be a little bit racey... still need
	//                        to think it all the way through...                        
	//
	// I think the edge cases are covered. Except if the user is manually reading VDPST,
	// then the state of vdpLimi could be out of sync with the real interrupt line, and cause
	// a double call. User apps should only read the mirror variable. Should I enforce that?

    VDP_CLEAR_VBLANK;           // release the VDP - we could instantly trigger again, but the vdpLimi is zeroed, so no loop
	VDP_INT_COUNTER++;			// count up the frames

	// the TI is running with ints off, so it won't retrigger in the
	// user code, even if it's slow. Our current process won't either because
	// the vdpLimi is set to 0.
	if (0 != userint) userint();

	// the TI interrupt would normally exit with the ints disabled
	// if it fired, so we will do the same here and not reset it.
}

// called automatically by crt0.S
void vdpinit() {
	volatile unsigned int x;
	
	// shut off the sound generator - if the cart skips the BIOS screen, this is needed.
	SOUND(0x9f);
	SOUND(0xbf);
	SOUND(0xdf);
	SOUND(0xff);
	
	// also silence and reset the AY sound chip in case it's present (if not present these
	// port writes should go nowhere)
    // note: turns out this is also important to work around a Phoenix AY powerup bug
    AY_REGISTER = AY_VOLA;
    AY_DATA_WRITE = 0x0;
    AY_REGISTER = AY_VOLB;
    AY_DATA_WRITE = 0x0;
    AY_REGISTER = AY_VOLC;
    AY_DATA_WRITE = 0x0;
    // To work around another Phoenix AY bug, make sure the tone C generator is 
    // running at an audible rate
    AY_REGISTER = AY_PERIODC_LOW;
    AY_DATA_WRITE = 0x10;

    // zero variables
    VDP_STATUS_MIRROR = 0;
    userint = (void (*)())0;
    VDP_INT_COUNTER = 1;
    gSaveIntCnt = 0;

	// interrupts off
	vdpLimi = 0;

	// before touching VDP, a brief delay. This gives time for the F18A to finish
	// initializing before we touch the VDP itself. This is needed on the Coleco if
	// you don't use the BIOS startup delay. This is roughly 200ms.
	x=60000;
	while (++x != 0) { }		// counts till we loop at 65536

    // reset the system and accomodate known alternate VDPs
    // First, reset then lock the F18A if any - this also turns off the screen
    reset_f18a();
    lock_f18a();

    VDP_STATUS_MIRROR = VDPST();	// init and clear any pending interrupt
}

// NOT atomic! Do NOT call with interrupts enabled!
void setUserIntHook(void (*hookfn)()) {
	userint = hookfn;
}

// NOT atomic! Do NOT call with interrupts enabled!
void clearUserIntHook() {
	userint = 0;
}
#endif

#ifdef TI99
// contains default init for 9938 regs 8-15 that make it more compatible
const unsigned char REG9938Init[8] = {
    0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

void vdpinit() {
	// shut off the sound generator
	SOUND(0x9f);
	SOUND(0xbf);
	SOUND(0xdf);
	SOUND(0xff);
	
	// also silence and reset the SID if present (if not present nothing will be mapped when we write)
	// we write the keyboard select to guarantee the SID blaster is mapped in
	MAP_SID_BLASTER;
	// mute all
	SIDBLASTER_MODEVOL = 0;
	// and TEST all the channels to reset them
	SIDBLASTER_CR1 = SIDBLASTER_CR_TEST;
	SIDBLASTER_CR2 = SIDBLASTER_CR_TEST;
	SIDBLASTER_CR3 = SIDBLASTER_CR_TEST;

    // reset interrupts on the CRU - note this also DISABLES peripherial interrupts (CRU bit 1)
    // only VDP interrupts are enabled. If you need peripherals, you need to set 1 to CRU bit 1
    __asm__ volatile("li r12,>2\n\tli r1,>0002\n\tldcr r1,15" : : : "r12","r1","cc");

    // zero variables
    VDP_INT_HOOK = (void (*)())0;
    VDP_INT_COUNTER = 1;

    // reset the system and accomodate known alternate VDPs
    // First, reset then lock the F18A if any - this also turns off the screen
    reset_f18a();
    lock_f18a();

    // now load some default values that make the 9938 happy
    for (int i=0; i<8; ++i) {
        VDP_SET_REGISTER(i+8, REG9938Init[i]);
    }

    // NOTE: a stock 9918 will be very confused, but your first call should
    // be one of the set_xxx calls to init a display mode

	VDP_STATUS_MIRROR = VDPST();	// init and clear any pending interrupt
}
	
// NOT atomic! Do NOT call with interrupts enabled!
void setUserIntHook(void (*hookfn)()) {
	VDP_INT_HOOK = hookfn;
}

// NOT atomic! Do NOT call with interrupts enabled!
void clearUserIntHook() {
	VDP_INT_HOOK = 0;
}
#endif

#ifdef GBA
#include <tursigb.h>
#include <GBASNPlay.h>
#include "string.h"

// GBA specific init code so that the vblank counts like the TI interrupt
// it also provides the user interrupt hook
// no other TI functionality is simulated (automotion, automusic, quit)
// but if needed, this is where it goes

// storage for VDP status byte
volatile unsigned char VDP_STATUS_MIRROR = 0;

// address of user interrupt function (vblank)
static void (*userint)() = 0;

// address of user interrupt function (anything else) - argument is REG_IF flags
static void (*otherint)(unsigned int) = 0;

// interrupt counter
volatile unsigned char VDP_INT_COUNTER = 0;

// used by vdpwaitvint - make certain it's reset
extern unsigned char gSaveIntCnt;

// May be called from true interrupt or from VDP_INTERRUPT_ENABLE, depending on
// the flag setting when the true interrupt fires.
// CRT0 expects it to be called InterruptProcess
void InterruptProcess() {
    unsigned int intType = REG_IF;

    // master disable ints
    REG_IME = 0;

    if (intType & INT_VBLANK) {
        VDP_CLEAR_VBLANK;           // release the VDP - we could instantly trigger again, but the interrupt is disabled
	    VDP_INT_COUNTER++;			// count up the frames

        // feed the audio fifos
        snupdateaudio();

        // ints off, so it won't retrigger in the
	    // user code, even if it's slow.
    	if (0 != userint) userint();
    } 
    if (intType & (~(INT_VBLANK))) {
        // for all interrupts that are not vblank
        if (0 != otherint) otherint(intType);
    }

    // clear and acknowledge all interrupts
    *(volatile unsigned short *)0x3007FF8 |= intType;   // To BIOS
    REG_IF = intType;   // To Hardware

	// the TI interrupt would normally exit with the ints disabled
	// if it fired, so we will do the same here and not reset vblank
    // But everything else needs to be back on
    REG_IME = 1;
}

void gbavidinit() {
    // set up for mode 4, eventually we'll add scaling to fit the whole screen
    // but for now we'll deal with cropping once the draw is done
    // mode 4 is 240x160x8 bit, and there are two pages so we can page flip
    // No sprites, we're going to draw everything. Otherwise we can't really scale it.
    REG_DISPCNT = MODE4 | BG2_ENABLE;
    REG_BG2CNT = COLORS_256;
    memset((char*)BG_RAM_BASE, 0, 240*160);
    reset_f18a();   // mostly to get the palette loaded
}

extern void intrwrap();
void gbainit() {
    // master interrupts off
    REG_IME = 0;

    // setup vblank and SN emulator
    INT_VECTOR = intrwrap; // assembly wrapper for interrupt handler
    REG_DISPSTAT = VBLANK_IRQ;
    REG_IE = INT_VCOUNT;     // VCOUNT triggers an audio reload
    
    // set up the GBA sound hardware - uses TIMER1 for frequency
    gbasninit();
    MUTE_SOUND();
    
    // set up the video to 8-bit color mode, we'll be emulating the F18A's 64 colors
    gbavidinit();

    // master interrupt enable
    REG_IME = 1;
}

// called automatically by crt0 code
// TODO: it's not automatic on gba cause I don't have crt0 source, so I fake it with wrap
// See __wrap_main() in gba_support.c
void vdpinit() {
    // init the gba basic hardware
    gbainit();
	
	// shut off the emulated sound generator
	SOUND(0x9f);
	SOUND(0xbf);
	SOUND(0xdf);
	SOUND(0xff);

    // zero variables
    VDP_STATUS_MIRROR = 0;
    
    userint = (void (*)())0;
    otherint = (void (*)(unsigned int))0;
    VDP_INT_COUNTER = 1;
    gSaveIntCnt = 0;

	// interrupts off
    VDP_INT_DISABLE;

    // reset the system and accomodate known alternate VDPs
    // First, reset then lock the F18A if any - this also turns off the screen
    reset_f18a();
    lock_f18a();

    // init status and clear any pending interrupt
    VDP_STATUS_MIRROR = VDPST();
}

// NOT atomic! Do NOT call with interrupts enabled!
void setUserIntHook(void (*hookfn)()) {
	userint = hookfn;
}

// NOT atomic! Do NOT call with interrupts enabled!
void clearUserIntHook() {
	userint = 0;
}

// NOT atomic! Do NOT call with interrupts enabled!
void setOtherIntHook(void (*hookfn)(unsigned int)) {
	otherint = hookfn;
}

// NOT atomic! Do NOT call with interrupts enabled!
void clearOtherIntHook() {
	otherint = 0;
}

#endif

#ifdef CLASSIC99
// TODO: was suggested after the fact, but using mmap to directly access Classic99's
// memory would be much faster and much less trouble than winsock... indeed we'd run
// at the speed of Windows for the most part.

// enable this define to enable shared VDP access, at least
#define VDP_HAS_SHARED_MEMORY

// we don't really get interrupts in this code - that'll make some
// timing tricky, but we should be able to get enough to get a good idea

// not used since we don't call KSCAN
unsigned char VDP_REG1_KSCAN_MIRROR;

// contains default init for 9938 regs 8-15 that make it more compatible
const unsigned char REG9938Init[8] = {
    0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

void debug_write(char *s, ...)
{
	char buf[1024];

	_vsnprintf(buf, 1023, s, (char*)((&s)+1));
	buf[1023]='\0';
    int n=strlen(buf);
    if (buf[n-1] == '\n') buf[n-1]='\0';

	// let all lines reach the external debugger
	OutputDebugString(buf);
	OutputDebugString("\n");
}

void vdpinit() {
	// shut off the sound generator
	SOUND(0x9f);
	SOUND(0xbf);
	SOUND(0xdf);
	SOUND(0xff);
	
    // zero variables
    //VDP_INT_HOOK = (void (*)())0;
    WriteByteToClassic99(0x8379, 1);    // VDP int counter reset

    // reset the system and accomodate known alternate VDPs
    // First, reset then lock the F18A if any - this also turns off the screen
    reset_f18a();
    lock_f18a();

    // now load some default values that make the 9938 happy
    for (int i=0; i<8; ++i) {
        VDP_SET_REGISTER(i+8, REG9938Init[i]);
    }

    // NOTE: a stock 9918 will be very confused, but your first call should
    // be one of the set_xxx calls to init a display mode
    VDP_CLEAR_VBLANK;
}

// pre-main entry point
extern int main(void);
void getMapping();
void classic99_main() {
#ifdef VDP_HAS_SHARED_MEMORY
    // gain access to shared VDP RAM
    getMapping();
#endif

    // first the 'crt' init
    vdpinit();

    // now 'take over' the TI by uploading a small program that spins the CPU
    // No interrupts, for now, but otherwise complete.
    // We gain control via the interrupt hook, so we assume GPL is running
    // There is the chance of race on writing the hook, but we'll risk it for now
    // to avoid, breakpoint the emulator when starting.
    WriteWordToClassic99(0xa000, 0x10FF);
    WriteWordToClassic99(0x83c4, 0xa000);

    // and off to main!
    main();
}

// NOT atomic! Do NOT call with interrupts enabled!
void setUserIntHook(void (*hookfn)(void)) {
	//VDP_INT_HOOK = hookfn;
}

// NOT atomic! Do NOT call with interrupts enabled!
void clearUserIntHook() {
	//VDP_INT_HOOK = 0;
}

// pending VDP address
unsigned int pendingVDP = -1;

// Some support functions - simpler than GBA so just left here
static unsigned int HandleTransaction(unsigned char *buffer, int cnt) {
    static int needInit = 1;
    static WSADATA wsaData = { 0 };
    static SOCKET sockfd = INVALID_SOCKET;
    struct sockaddr_in local_addr;
    struct sockaddr_in dest_addr;
    struct sockaddr_in recv_addr;
    int recv_addr_len;
    int result;
    int bytes_received;
    BOOL bNewBehavior = FALSE;
    DWORD dwBytesReturned = 0;

    if (needInit) {
        /* Initialize Winsock */
        result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            debug_write("WSAStartup failed: %d\n", result);
            // but try anyway
        }
        /* Create UDP socket */
        sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sockfd == INVALID_SOCKET) {
            debug_write("socket() failed: %d\n", WSAGetLastError());
        }

        /* Disable ICMP port unreachable error reporting */
        /* This prevents error 10054 (WSAECONNRESET) */
        WSAIoctl(sockfd, SIO_UDP_CONNRESET, &bNewBehavior, sizeof(bNewBehavior),
                 NULL, 0, &dwBytesReturned, NULL, NULL);

        /* Bind to a local port other than 9900 */
        memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin_family = AF_INET;
        local_addr.sin_port = htons(0);  /* Let OS choose a port */
        local_addr.sin_addr.s_addr = INADDR_ANY;
    
        if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) == SOCKET_ERROR) {
            debug_write("bind() failed: %d\n", WSAGetLastError());
        } else {
            needInit = 0;
        }
    }

    /* Setup destination address structure */
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(0x9900);
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Send the UDP packet */
    result = sendto(sockfd, buffer, cnt, 0,
                    (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    
    if (result == SOCKET_ERROR) {
        debug_write("sendto() failed: %d\n", WSAGetLastError());
        // no point waiting for a reply
        return 1;
    }
    
    //debug_write("Sent %d bytes to 127.0.0.1:0x9900\n", result);
    //debug_write("Waiting for response...\n");

    /* Receive response packet */
    recv_addr_len = sizeof(recv_addr);
    memset(buffer, 0, cnt);
    
    bytes_received = recvfrom(sockfd, buffer, cnt, 0,
                              (struct sockaddr *)&recv_addr, &recv_addr_len);
    
    if (bytes_received == SOCKET_ERROR) {
        debug_write("recvfrom() failed: %d\n", WSAGetLastError());
        return 1;
    } else {
        //debug_write("Received %d bytes from %s:%d\n", 
        //       bytes_received,
        //       inet_ntoa(recv_addr.sin_addr),
        //       ntohs(recv_addr.sin_port));
        return 0;
    }
}

unsigned char ReadByteFromClassic99(int cpu) {
    char buf[16];

    buf[0] = 0x00;  // read byte non-safe
    buf[1] = 0x01;  // CPU memory
    buf[2] = 0x00;  // 32-bit msb
    buf[3] = 0x00;
    buf[4] = (cpu>>8)&0xff;
    buf[5] = (cpu)&0xff;
    buf[6] = 0;
    buf[7] = 0;
    if (HandleTransaction(buf, 8)) {
        return 0xff;
    } else {
        // should be LSB
        return buf[7];
    }
}

void WriteByteToClassic99(int cpu, int val) {
    char buf[16];

    buf[0] = 0x80;  // write byte non-safe
    buf[1] = 0x01;  // CPU memory
    buf[2] = 0x00;  // 32-bit msb
    buf[3] = 0x00;
    buf[4] = (cpu>>8)&0xff;
    buf[5] = (cpu)&0xff;
    buf[6] = 0;
    buf[7] = val&0xff;
    HandleTransaction(buf, 8);
}

void WriteWordToClassic99(int cpu, int val) {
    char buf[16];

    buf[0] = 0x90;  // write word non-safe
    buf[1] = 0x01;  // CPU memory
    buf[2] = 0x00;  // 32-bit msb
    buf[3] = 0x00;
    buf[4] = (cpu>>8)&0xff;
    buf[5] = (cpu)&0xff;
    buf[6] = (val>>8)&0xff;
    buf[7] = val&0xff;
    HandleTransaction(buf, 8);
}

#ifdef VDP_HAS_SHARED_MEMORY

unsigned char *pVDP = NULL;
HANDLE hMapVDP = NULL;

void getMapping() {
    if (pVDP != NULL) {
        return;
    }
    if (hMapVDP == NULL) {
        hMapVDP = OpenFileMapping(
            FILE_MAP_ALL_ACCESS,     // Read/write access
            FALSE,                   // Do not inherit the name
            "Classic99VDPSharedMemory");      // Name of mapping object

        while (hMapVDP == NULL) {
            // give up
            exit(1);
        }
    }

    pVDP = MapViewOfFile(
        hMapVDP,                // Handle to mapping object
        FILE_MAP_ALL_ACCESS,     // Read/write permission
        0,
        0,
        16*1024);               // normal 9918A RAM size

    if (pVDP == NULL) {
        // give up
        exit(1);
    }
}

void SetVDPToClassic99(int pAddr, int ch, int cnt) {
    memset(pVDP+pAddr, ch, cnt);
    c99SetVDPAddress(pAddr+cnt-1);
}

void ReadVDPBlockFromClassic99(int pAddr, unsigned char *pDest, int cnt) {
    memcpy(pDest, pVDP+pAddr, cnt);
    c99SetVDPAddress(pAddr+cnt-1);
}

void WriteVDPBlockToClassic99(int pAddr, unsigned char *pSrc, int cnt) {
    memcpy(pVDP+pAddr, pSrc, cnt);
    c99SetVDPAddress(pAddr+cnt-1);
}

// some emulation for local address tracking
unsigned char vdp_prefetch = 0;		// vdp prefetch byte
unsigned char vdp_flipflop = 0;		// vdp address flipflop

void c99SetVDPAddress(int pAddr) {
    // NOT to be used for register sets - this is not sent to the real chip
    pendingVDP = pAddr;
    vdp_flipflop = 0;
    if (pendingVDP&0xc000) {
        // no prefetch
        pendingVDP &= 0x3fff;
    } else {
        vdp_prefetch=*(pVDP+pendingVDP);
        ++pendingVDP;
    }
}

// We are reading from the real RAM, so even F18A RAM writes will work here!
unsigned char c99VDPRD() {
	unsigned char ret = vdp_prefetch;
	vdp_prefetch = *(pVDP+pendingVDP);
	++pendingVDP;
	pendingVDP &= 0x3fff;
	vdp_flipflop = 0;
	return ret;
}
void c99VDPWD(unsigned char x) {
    vdp_prefetch = x;
    *(pVDP+pendingVDP) = x;
	++pendingVDP;
    pendingVDP &= 0x3fff;
    vdp_flipflop = 0;
}

// for status we still need to go through the CPU
unsigned char c99VDPST() {
	vdp_flipflop = 0;
    return ReadByteFromClassic99(0x8802);
}

// we need to do this remotely as it may be for registers,
// but we'll also update the local address emulation
void c99VDPWA(unsigned char x) {
    WriteByteToClassic99(0x8c02, x);
	if (vdp_flipflop) {
		pendingVDP = (pendingVDP & 0xff) | ((x & 0x3f) << 8);
		if ((x & 0xc0) == 0) {
			// prefetch
			vdp_prefetch = *(pVDP+pendingVDP);
			++pendingVDP;
			pendingVDP &= 0x3fff;
		}
		vdp_flipflop = 0;
	} else {
		pendingVDP = (pendingVDP & 0xff00) | x;
		vdp_flipflop = 1;
	}
}

#else
// Fall back on CPU access to the VDP - we still try to batch it up a bit

// faster block functions
void SetVDPToClassic99(int pAddr, int ch, int cnt) {
    char buf[125*8+16]; // up to 125 at a time, plus the address set

    // set the VDP address
    c99SetVDPAddress(pAddr|0x4000);

    int off = 0;

    buf[off++] = 0x80;  // write byte non-safe
    buf[off++] = 0x01;  // CPU memory
    buf[off++] = 0x00;  // 32-bit msb
    buf[off++] = 0x00;
    buf[off++] = 0x8C;  // VDPWA
    buf[off++] = 0x02;
    buf[off++] = 0;
    buf[off++] = pendingVDP&0xff;    // LSB first

    buf[off++] = 0x80;  // write byte non-safe
    buf[off++] = 0x01;  // CPU memory
    buf[off++] = 0x00;  // 32-bit msb
    buf[off++] = 0x00;
    buf[off++] = 0x8C;  // VDPWA
    buf[off++] = 0x02;
    buf[off++] = 0;
    buf[off++] = (pendingVDP>>8)&0xff;    // MSB

    pendingVDP = -1;

    while (cnt > 0) {
        int mx = 125;
        if (mx > cnt) mx=cnt;
        for (int idx=0; idx<mx; ++idx) {
            buf[off++] = 0x80;  // write byte
            buf[off++] = 0x01;  // cpu
            buf[off++] = 0x00;  // adr
            buf[off++] = 0x00;
            buf[off++] = 0x8c;
            buf[off++] = 0x00;  // VDPWD
            buf[off++] = 0;
            buf[off++] = ch&0xff;
        }
        HandleTransaction(buf, off);
        cnt -= mx;
        off = 0;
    }
}

void ReadVDPBlockFromClassic99(int pAddr, unsigned char *pDest, int cnt) {
    char buf[125*8+16]; // up to 125 at a time

    // set the VDP address
    c99SetVDPAddress(pAddr);

    int off = 0;

    buf[off++] = 0x80;  // write byte non-safe
    buf[off++] = 0x01;  // CPU memory
    buf[off++] = 0x00;  // 32-bit msb
    buf[off++] = 0x00;
    buf[off++] = 0x8C;  // VDPWA
    buf[off++] = 0x02;
    buf[off++] = 0;
    buf[off++] = pendingVDP&0xff;    // LSB first

    buf[off++] = 0x80;  // write byte non-safe
    buf[off++] = 0x01;  // CPU memory
    buf[off++] = 0x00;  // 32-bit msb
    buf[off++] = 0x00;
    buf[off++] = 0x8C;  // VDPWA
    buf[off++] = 0x02;
    buf[off++] = 0;
    buf[off++] = (pendingVDP>>8)&0xff;    // MSB

    pendingVDP = -1;

    while (cnt > 0) {
        int mx = 125;
        if (mx > cnt) mx=cnt;
        for (int idx=0; idx<mx; ++idx) {
            buf[off++] = 0x00;  // read byte
            buf[off++] = 0x01;  // cpu
            buf[off++] = 0x00;  // adr
            buf[off++] = 0x00;
            buf[off++] = 0x88;  // VDPRD
            buf[off++] = 0x00;
            buf[off++] = 0;
            buf[off++] = 0;
        }
        HandleTransaction(buf, off);
        // success or failure, copy out the data
        for (int idx=0; idx<mx; ++idx) {
            *(pDest++) = buf[idx*8+23];
        }
        cnt -= mx;
        off = 0;
    }
}

void WriteVDPBlockToClassic99(int pAddr, unsigned char *pSrc, int cnt) {
    char buf[125*8+16]; // up to 125 at a time

    // set the VDP address
    c99SetVDPAddress(pAddr|0x4000);

    int off = 0;

    buf[off++] = 0x80;  // write byte non-safe
    buf[off++] = 0x01;  // CPU memory
    buf[off++] = 0x00;  // 32-bit msb
    buf[off++] = 0x00;
    buf[off++] = 0x8C;  // VDPWA
    buf[off++] = 0x02;
    buf[off++] = 0;
    buf[off++] = pendingVDP&0xff;    // LSB first

    buf[off++] = 0x80;  // write byte non-safe
    buf[off++] = 0x01;  // CPU memory
    buf[off++] = 0x00;  // 32-bit msb
    buf[off++] = 0x00;
    buf[off++] = 0x8C;  // VDPWA
    buf[off++] = 0x02;
    buf[off++] = 0;
    buf[off++] = (pendingVDP>>8)&0xff;    // MSB

    pendingVDP = -1;

    while (cnt > 0) {
        int mx = 125;
        if (mx > cnt) mx=cnt;
        for (int idx=0; idx<mx; ++idx) {
            buf[off++] = 0x80;  // write byte
            buf[off++] = 0x01;  // cpu
            buf[off++] = 0x00;  // adr
            buf[off++] = 0x00;
            buf[off++] = 0x8c;  // VDPWD
            buf[off++] = 0x00;
            buf[off++] = 0;
            buf[off++] = *(pSrc++);
        }
        HandleTransaction(buf, off);
        cnt -= mx;
        off = 0;
    }
}

void c99SetVDPAddress(int pAddr) {
    pendingVDP = pAddr;
}

// put the main VDP RAM in 256k external, otherwise it's half our available memory
// we're way faster than coleco or TI, so it won't hurt our performance.
unsigned char c99VDPRD() {
    if (pendingVDP == -1) {
        return ReadByteFromClassic99(0x8800);
    }

    char buf[24];

    // set the VDP address
    buf[0] = 0x80;  // write byte non-safe
    buf[1] = 0x01;  // CPU memory
    buf[2] = 0x00;  // 32-bit msb
    buf[3] = 0x00;
    buf[4] = 0x8C;  // VDPWA
    buf[5] = 0x02;
    buf[6] = 0;
    buf[7] = pendingVDP&0xff;    // LSB first

    buf[8] = 0x80;  // write byte non-safe
    buf[9] = 0x01;  // CPU memory
    buf[10] = 0x00;  // 32-bit msb
    buf[11] = 0x00;
    buf[12] = 0x8C;  // VDPWA
    buf[13] = 0x02;
    buf[14] = 0;
    buf[15] = (pendingVDP>>8)&0xff;    // MSB

    buf[16] = 0x00;  // read byte non-safe
    buf[17] = 0x01;  // CPU memory
    buf[18] = 0x00;  // 32-bit msb
    buf[19] = 0x00;
    buf[20] = 0x88;  // VDPRD
    buf[21] = 0x00;
    buf[22] = 0;
    buf[23] = 0;

    pendingVDP = -1;
    if (HandleTransaction(buf, 24)) {
        return 0xff;
    } else {
        // should be LSB
        return buf[23];
    }
}
void c99VDPWD(unsigned char x) {
    if (pendingVDP == -1) {
        WriteByteToClassic99(0x8c00, x);
        return;
    }

    char buf[24];

    // set the VDP address
    buf[0] = 0x80;  // write byte non-safe
    buf[1] = 0x01;  // CPU memory
    buf[2] = 0x00;  // 32-bit msb
    buf[3] = 0x00;
    buf[4] = 0x8C;  // VDPWA
    buf[5] = 0x02;
    buf[6] = 0;
    buf[7] = pendingVDP&0xff;    // LSB first

    buf[8] = 0x80;  // write byte non-safe
    buf[9] = 0x01;  // CPU memory
    buf[10] = 0x00;  // 32-bit msb
    buf[11] = 0x00;
    buf[12] = 0x8C;  // VDPWA
    buf[13] = 0x02;
    buf[14] = 0;
    buf[15] = ((pendingVDP>>8)&0xff);    // MSB (assume caller set write bit if they meant it)

    buf[16] = 0x80;  // write byte non-safe
    buf[17] = 0x01;  // CPU memory
    buf[18] = 0x00;  // 32-bit msb
    buf[19] = 0x00;
    buf[20] = 0x8c;  // VDPWD
    buf[21] = 0x00;
    buf[22] = 0;
    buf[23] = x&0xff;

    pendingVDP = -1;
    HandleTransaction(buf, 24);
}

unsigned char c99VDPST() {
    return ReadByteFromClassic99(0x8802);
}
void c99VDPWA(unsigned char x) {
    WriteByteToClassic99(0x8c02, x);
}
#endif  // VDP shared

#endif  // classic99
