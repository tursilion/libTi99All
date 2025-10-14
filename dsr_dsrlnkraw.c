// DSR interface code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)
#ifdef CLASSIC99
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#endif

#include "files.h"
#include "vdp.h"

#ifdef TI99
// uses: scratchpad >8340-8348, >8354, >8355, >8356, >83d0, >83d2, GPLWS

#define DSR_NAME_LEN	*((volatile unsigned int*)0x8354)

// This returns an error (non-zero) only if the lookup fails 
// - a DSR will return error codes in the PAB Status byte
unsigned char __attribute__((noinline)) dsrlnkraw(unsigned int vdp) {
	// modified version of the e/a DSRLNK, for data >8 (DSR) only
	// this one does not modify data in low memory expansion so "boot tracking" there may not work.
	unsigned char * const buf = (unsigned char*)0x8340;	// 8 bytes of memory for a name buffer
	unsigned int status = vdp + 1;
	unsigned int ret = 0;  // assume success

	vdp+=9;
	DSR_PAB_POINTER = vdp;
 
	unsigned char size = vdpreadchar(vdp);
	unsigned char cnt=0;
	while (cnt < 8) {
		buf[cnt] = VDPRD();	// still in the right place after the readchar above got the length
		if (buf[cnt] == '.') {
			break;
		}
		cnt++;
	}
	if ((cnt == 0) || (cnt > 7)) {
		// illegal device name length
		VDP_SET_ADDRESS_WRITE(status);
		VDPWD(DSR_ERR_FILEERROR);
		return 1;
	}
	// save off the device name length (asm below uses it!)
	DSR_LEN_COUNT=cnt;

	unsigned int CRU = 0;
	DSR_NAME_LEN = cnt;
	++cnt;
	DSR_PAB_POINTER += cnt;
	
	// make sure the error byte is zeroed before we start
	vdpchar(status, 0);

	// TODO: we could rewrite the rest of this in C, just adding support for SBO, SBZ and the actual call which
	// needs to be wrapped with LWPI....
	__asm__ volatile (
	"		ai r10,-34				; make stack room to save workspace & zero word\n"
    "		mov %1,@>83ec           ; move buffer address to GPLWS R6\n"
	"		lwpi 0x83e0				; get gplws\n"
	"		li r0,0x8300			; source wp for backup\n"
	"		mov @0x8314,r1			; get r10 for destination\n"
	"bkupl1	mov *r0+,*r1+			; copy register\n"
	"		ci r0,>8322				; test for end of copy\n"
	"		jne bkupl1\n"
	"		jmp begin\n"
	"dsrdat data >aa00\n"
	"begin  clr  r1					; r1=0\n"
	"       li   r12,0x0f00			; cru base to >0f00 (first card -1)\n"
	"       jmp  a2316              ; skip card off.\n"
	"a2310  sbz  0					; card off\n"
	"a2316  ai   r12,0x0100			; next card (>1000 for first)\n"
	"       clr  @0x83d0			; clear cru tracking at >83d0\n"
	"       ci   r12,0x2000			; check if all cards are done\n"
	"       jeq  axxx				; if yes, we didn't find it, so error out\n"
	"       mov  r12,@0x83d0		; save cru base\n"
	"       sbo  0					; card on\n"
	"       li   r2,0x4000			; read card header bytes\n"
	"       cb   *r2,@dsrdat		; >aa = header\n"
	"       jne  a2310				; no: loop back for next card\n"
	"       ai   r2,8            	; offset (contains the data statement, so 8 for a device, for >4008)\n"
	"       jmp  a2340				; always jump into the loop from here\n"
	"a233a  mov  @0x83d2,r2         ; next sub\n"
	"       jeq  a2310              ; if no pointer, link back to get next card\n"
	"a2340  mov  *r2,r2             ; grab link pointer to next\n"
	"       mov  r2,@0x83d2         ; save link address in >83d2\n"
	"       inct r2					; point to entry address\n"
	"       mov  *r2+,r9            ; save address in r9\n"
	"       movb @0x8355,r5			; get dsr name length (low byte of >8354)\n"
	"       jeq  a2364              ; size=0, so take it \n"
	"       cb   r5,*r2+			; compare length to length in dsr\n"
	"       jne  a233a              ; diff size: loop back for next\n"
	"       srl  r5,8			    ; make length a word count\n"
	"       mov  r6,r0              ; start search from r6\n"
	"a235c  cb   *r0+,*r2+          ; check name - pointer in r0\n"
	"       jne  a233a              ; diff name: loop back for next entry\n"
	"       dec  r5					; count down length\n"
	"       jne  a235c              ; not done yet: next char\n"
	"a2364  inc  r1                 ; if we get here, everything matched, increment # calls\n"
	"       bl   *r9                ; link\n"
	"       jmp  a233a              ; check next entry on the same card -- most dsrs will skip this \n"
	"       sbz  0                  ; card off\n"
	"       clr  r12                ; clear tmp error flag\n"
	"clnup  li r0,>8300				; load register for restore\n"
	"		mov @0x8314,r1			; get r10 for source\n"
	"rslp1	mov *r1+,*r0+			; copy register\n"
	"		ci r0,>8322\n"
	"		jne rslp1\n"
	"a2388  lwpi 0x8300             ; restore workspace\n"
	"		ai r10,34				; restore stack\n"
	"       jmp alldn\n"
	"axxx   seto r12                ; set tmp error flag\n"
	"       jmp clnup               ; go back and restore\n"
	"alldn  mov @>83f8,%0           ; get the error flag\n"

        : "=r" (ret)
		: "r" (buf)
	);

    // this is a little awkward, but it makes for cleaner asm above
    // will clean it up when we convert the above to C
    if (ret) {
        return 1;
    } else {
        return 0;
    }

}
#endif

#ifdef COLECO
// no equivalent at this time - maybe Adam someday?

unsigned char dsrlnkraw(unsigned int vdp) {
    (void)vdp;
    return 1;   // return failed
}

#endif

#ifdef GBA
// no equivalent

unsigned char dsrlnkraw(unsigned int vdp) {
    (void)vdp;
    return 1;   // return failed
}

#endif

#ifdef CLASSIC99
// I guess we can keep the emulated CPU buffer here
unsigned char CPU[65536];
extern void debug_write(char *s, ...);

// for some reason the _snwprintf I used in Classic99 isn't working here
// I just need raw ASCII convert, so I can do it by hand
void makewide(wchar_t *pw, int max, char *p) {
    char *pout = (char*)pw;
    while ((max--) && (*p)) {
        *(pout++) = *(p++);
        *(pout++) = 0;
    }
    if (max <= 0) {
        pout -= 2;
    }
    *(pout++) = 0;
    *(pout++) = 0;
}

// get a file from the web and store in RAM
// caller is responsible for freeing data
// NULL on failure
// Name can be PI.HTTP://stuff (or HTTPS)
unsigned char *getWebFile(const char *filename, int *outSize) {
    // adapted from https://stackoverflow.com/questions/23038973/c-winhttp-get-response-header-and-body
#define MAX_URL_LEN 2048
#define MAX_RAM_FILE 8192
    DWORD dwSize;
    DWORD dwDownloaded;
    DWORD headerSize = 0;
    BOOL  bResults = FALSE;
    HINTERNET hSession;
    HINTERNET hConnect;
    HINTERNET hRequest;
    wchar_t host[MAX_URL_LEN];
    wchar_t resource[MAX_URL_LEN];
    int secure = FALSE;
    unsigned char *buf = NULL;
    int outPos = 0;
    *outSize = 0;

    // Parse out pi.http[s] vs urix
    //
    // it's a URI request - if PI make sure it's http[s]
    // TODO: not sure if multiple web files are allowed to
    // be open! This code assumes only one...
    char url[MAX_URL_LEN];
    if (strlen(filename)+1 >= MAX_URL_LEN) {
        return NULL;
    }
    memset(url, 0, sizeof(url));

    if ((0 == memcmp(filename, "PI.", 3) == 0) || (0 == memcmp(filename, "pi.", 3) == 0)) {
        if ((0 != memcmp(filename+3, "http", 4)) && (0 != memcmp(filename+3, "HTTP", 4))) {
            debug_write("Can't load from '%s'!", filename);
            return NULL;
        }
        // get just the URL
        strcpy(url, filename+3);
    } else {
        debug_write("Can't load from '%s'?", filename);
        return NULL;
    }

    // split up the path and make it wide
    char *p = strchr(url, ':');
    if (NULL == p) {
        // okay, assume no http part
        secure = FALSE;
        p = url;
    } else if (p == url) {
        // what is this nonsense?
        secure = FALSE;
        ++p;
    } else if ((*(p-1) == 'S')||(*(p-1) == 's')) {
        // https:
        secure = TRUE;
        ++p;
    } else {
        // probably http:, but not checking
        // we shouldn't be called with other methods...
        secure = FALSE;
        ++p;
    }
    while (*p == '/') ++p;
    char *p2 = strchr(p, '/');
    if (NULL != p2) {
        *p2 = '\0';
        ++p2;
        makewide(host, MAX_URL_LEN, p);
        makewide(resource, MAX_URL_LEN, p2);
    } else {
        makewide(host, MAX_URL_LEN, p);
        makewide(resource, MAX_URL_LEN, p2);
    }

    hSession = WinHttpOpen( L"LibTI99TipiSim/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0 );
    if (NULL == hSession) {
        debug_write("Failed to create web session, code %d", GetLastError());
        return NULL;
    }

    hConnect = WinHttpConnect( hSession, host, secure ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT, 0 );
    if (NULL == hConnect) {
        debug_write("Web connect request failed, code %d", GetLastError());
        WinHttpCloseHandle(hSession);
        return NULL;
    }
        
    hRequest = WinHttpOpenRequest( hConnect, L"GET", resource, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, secure ? WINHTTP_FLAG_SECURE : 0 );
    if (NULL == hRequest) {
        debug_write("Web open request failed, code %d", GetLastError());
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return NULL;
    }

    bResults = WinHttpSendRequest( hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, NULL, 0, 0, 0 );
    if (!bResults) {
        debug_write("Web Send Request failed, code %d", GetLastError());
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return NULL;
    }

    bResults = WinHttpReceiveResponse( hRequest, NULL );
    if (!bResults) {
        debug_write("Web Receive failed, code %d", GetLastError());
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return NULL;
    }

    /* store headers...
    bResults = WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, NULL, WINHTTP_NO_OUTPUT_BUFFER, &headerSize, WINHTTP_NO_HEADER_INDEX);
    if ((!bResults) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
    {
        responseHeader.resize(headerSize / sizeof(wchar_t));
        if (responseHeader.empty())
        {
            bResults = TRUE;
        }
        else
        {
            bResults = WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, NULL, &responseHeader[0], &headerSize, WINHTTP_NO_HEADER_INDEX);
            if( !bResults ) headerSize = 0;
            responseHeader.resize(headerSize / sizeof(wchar_t));
        }
    }
    */

    // TODO: this needs optimizing...
    do
    {
        // Check for available data.
        dwSize = 0;
        bResults = WinHttpQueryDataAvailable( hRequest, &dwSize );
        if (!bResults) {
            debug_write("Failed to query web data available, code %d", GetLastError());
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            free(buf);
            return NULL;
        }

        if (dwSize == 0) {
            // all done
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return buf;
        }

        // grab what is so-far available
        do
        {
            // Allocate space for the buffer.
            if (outPos + (signed)dwSize > *outSize) {
                buf = (unsigned char*)realloc(buf, outPos+dwSize);
                *outSize = outPos+dwSize;
                if (*outSize >= MAX_RAM_FILE) {
                    debug_write("Web file exceeds max size (%dk) (LibTI99 limit) - failing", MAX_RAM_FILE/1024);
                    WinHttpCloseHandle(hRequest);
                    WinHttpCloseHandle(hConnect);
                    WinHttpCloseHandle(hSession);
                    free(buf);
                    return NULL;
                }
            }
            // Read the data.
            bResults = WinHttpReadData( hRequest, &buf[outPos], dwSize, &dwDownloaded );
            if (!bResults) {
                debug_write("Failed reading web data, code %d", GetLastError());
                // is this not an error?
                dwDownloaded = 0;
            }

            outPos += dwDownloaded;
            dwSize -= dwDownloaded;

            if (dwDownloaded == 0) {
                break;
            }
        }
        while (dwSize > 0);
    }
    while (TRUE);
}

// since we need to open a file, we have to extract the PAB here
// I need to support local files (will treat DSKx as a subfolder)
// and remote files through PI.HTTP.
// Even better, I want to support CPU buffers. However, for now,
// ** I will only support the LOAD opcode **
unsigned char dsrlnkraw(unsigned int vdp) {
    // what's going to make this REALLY gross is I'm going to
    // go ahead and READ THE PAB BACK FROM CLASSIC99. Yes.
    // we assume it will fit in 128 bytes - this is just a hack. Sorry.
    unsigned char pab[128];
    unsigned char buf[16384];   // up to full range of VDP memory

    // hack part 2 - just read it all - we assume PAB is in VDP but CPU doesn't have to be
    vdpmemread(vdp, pab, sizeof(pab));

    // now look at what we got
    // remember that pName is NOT a pointer but the start of the string
    struct PAB *pPab = (struct PAB*)pab;
    if (pPab->OpCode != DSR_LOAD) {
        vdpchar(vdp+1, DSR_ERR_ILLEGALOPCODE);
        return 0;
    }
    vdpchar(vdp+1, DSR_ERR_NONE);   // for now...

    // extract the filename - it starts at offset 10
    char szStr[128];
    int strPos = 0;
    memset(szStr, 0, sizeof(szStr));
    VDP_SET_ADDRESS(vdp+10);
    for (int i=0; i<pPab->NameLength; ++i) {
        szStr[strPos++] = VDPRD();
    }
    // So we have either DSK1.FILENAME or PI.HTTPS://URL/FILE
    if (0 == memcmp(szStr, "DSK", 3)) {
        // swap dots and slashes and treat it as a local path
        for (int i=0; i<strPos; ++i) {
            if (szStr[i] == '.') szStr[i]='\\';
            else if ((szStr[i]=='/')||(szStr[i]=='\\')) szStr[i]='.';
        }
        // now we have DSK1\FILENAME
        debug_write("open %s\n", szStr);
        FILE *fp = fopen(szStr, "rb");
        if (NULL == fp) {
            vdpchar(vdp+1, DSR_ERR_FILEERROR);
            return 0;
        }
        int nsiz = fread(buf, 1, sizeof(buf), fp);
        fclose(fp);
        if ((nsiz > pPab->CharCount) || ((pPab->VDPBuffer < 0x4000) && (pPab->CharCount+pPab->VDPBuffer > 0x4000))) {
            // refuse to load
            debug_write("File doesn't fit in PAB buffer or exceeds VDP\n");
            vdpchar(vdp+1, DSR_ERR_MEMORYFULL);
            return 0;
        }
        // else write to CPU or VDP ram based on address
        if (pPab->VDPBuffer < 0x4000) {
            debug_write("Copy file to VDP >%04X\n", pPab->VDPBuffer);
            vdpmemcpy(pPab->VDPBuffer, buf, nsiz);
        } else {
            // it's a CPU write - so we can keep it locally
            debug_write("Copy file to CPU >%04X\n", pPab->VDPBuffer);
            memcpy(&CPU[pPab->VDPBuffer], buf, nsiz);
        }
    } else if (0 == memcmp(szStr, "PI.HTTP", 7)) {
        // it's a URL request
        // pName can be PI.HTTP://stuff (or HTTPS)
        int outSize = 0;
        debug_write("Try to fetch %s\n", szStr);
        unsigned char *buf = getWebFile(szStr, &outSize);
        if (NULL == buf) {
            vdpchar(vdp+1, DSR_ERR_FILEERROR);
            return 0;
        }
        if (outSize > pPab->RecordNumber) {
            free(buf);
            debug_write("Returned file too large.");
            vdpchar(vdp+1, DSR_ERR_FILEERROR);
            return 0;
        }
        // else write to CPU or VDP ram based on address
        if (pPab->VDPBuffer < 0x4000) {
            debug_write("Copy file to VDP >%04X\n", pPab->VDPBuffer);
            vdpmemcpy(pPab->VDPBuffer, buf, outSize);
        } else {
            // it's a CPU write - so we can keep it locally
            debug_write("Copy file to CPU >%04X\n", pPab->VDPBuffer);
            memcpy(&CPU[pPab->VDPBuffer], buf, outSize);
        }
        free(buf);
    }
    return 0;
}

#endif
