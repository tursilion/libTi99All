// DSR interface code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

#include "files.h"
#include "string.h"

#ifdef TI99

// a slightly simpler search for SBRLNK functions (offset 10)
// since we only need to match 1 character, we don't need as much prep as dsrlnk
unsigned char __attribute__((noinline)) sbrlnk(unsigned char subprogram) {
	unsigned int ret;

	// TODO: we could rewrite the rest of this in C, just adding support for SBO, SBZ and the actual call which
	// needs to be wrapped with LWPI....
	__asm__ volatile (
    /* warning: GCC likes to assign %0 and %1 to the same reg, so order matters here */
    "mov %1,@>83e8      ; move subprogram to gplws r4\n\t"  
	"seto %0			; not found\n\t"
	"lwpi 0x83e0		; gplws\n\t"
    "swpb r4            ; setting up for match\n\t"
    "andi r4,>00FF      ; just the subprogram\n\t"
    "ori r4,>0100       ; length and subprogram\n\t"
    "li r12,>1000       ; first cru\n"
"crulp\n\t"
    "sbo 0				; turn on the rom (set above)\n\t"
    "li r2,>aa00        ; desired header\n\t"
    "movb @0x4000,r1    ; check card header\n\t"
    "cb r2,r1           ; test\n\t"
    "jne filedon        ; nope\n\t"
	"mov @0x400a,r1		; get pointer to the subprogram list\n"
"filelp\n\t"
	"jeq filedon		; no subprograms\n\t"
	"mov *r1+,r3		; link to next item\n\t"
	"mov *r1+,r2		; address of this one\n\t"
	"mov *r1+,r0		; we are looking for length 1, name >16\n\t"
	"c r0,r4\n\t"
	"jeq filegt\n\t"
	"mov r3,r1			; nope, get next\n\t"
	"jmp filelp\n"
"filegt\n\t"
	"lwpi 0x8300\n\t"
	"clr %0				; mark success in gcc workspace\n\t"
	"lwpi 0x83e0\n\t	; we aren't done yet\n\t"
	"bl *r2				; go ahead and call it\n\t"
	"nop				; skipped on success (we ignore failure, then)\n\t"
	"jmp alldon\n"
"filedon\n\t"
	"sbz 0				; turn off the rom (we assume r12 was not altered, it shouldn't be!)\n\t"
    "ai r12,>0100       ; next cru\n\t"
    "ci r12,>2000       ; are we done?\n\t"
    "jne crulp\n"
"alldon\n\t"
	"sbz 0				; turn off the rom (we assume r12 was not altered, it shouldn't be!)\n\t"
	"lwpi 0x8300		; our own ws back\n"

        : "=r" (ret)
        : "r" (subprogram)
	);

    if (ret) {
        return DSR_ERR_DSRNOTFOUND;
    } else {
        return DSR_ERR_NONE;
    }
}

#endif
