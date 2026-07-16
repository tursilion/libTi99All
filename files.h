#ifndef FILES_H
#define FILES_H

#ifdef __cplusplus
extern "C" {
#endif

// DSR interface code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

#if defined(TI99)
#define DSR_FILES_COUNT	*((volatile unsigned char*)0x834C)
#define DSR_LEN_COUNT	*((volatile unsigned int* )0x8354)
#define DSR_PAB_POINTER *((volatile unsigned int* )0x8356)

#define SBR_UNIT_NUMBER *((volatile unsigned char*)0x834C)
#define SBR_CODE        *((volatile unsigned char*)0x834D)
#define SBR_PATH_PTR    *((volatile unsigned int *)0x834E)
#define SBR_ERROR_RET   *((volatile unsigned char*)0x8350)
#define SBR_INFO_PTR    *((volatile unsigned char*)0x8350)  // same address, but just a byte (added to 0x8300)
#define SBR_PATH2_PTR   *((volatile unsigned int* )0x8350)  // used in rename
#define SBR_SECTOR_LOW  *((volatile unsigned int* )0x8350)  // same address again
#define SBR_SECTOR_HIGH *((volatile unsigned int* )0x8352)

#define SBR_SECTOR_READ 1
#define SBR_SECTOR_WRITE 0

#define SBR_DEFAULT_INFO_ADR 0x8352
#endif

#define DSR_OPEN	0x00
#define DSR_CLOSE	0x01
#define DSR_READ	0x02
#define DSR_WRITE	0x03
#define DSR_REWIND	0x04
#define DSR_LOAD	0x05
#define DSR_SAVE	0x06
#define DSR_DELETE	0x07
#define DSR_SCRATCH	0x08	// note: most DSRs do not implement scratch
#define DSR_STATUS	0x09

#define GET_ERROR(status) ((status)&0xe0)
	#define DSR_ERR_NONE			0x00
	#define DSR_ERR_WRITEPROTECT	0x20
	#define DSR_ERR_BADATTRIBUTE	0x40
	#define DSR_ERR_ILLEGALOPCODE	0x60
	#define DSR_ERR_MEMORYFULL		0x80
	#define DSR_ERR_PASTEOF			0xA0
	#define DSR_ERR_DEVICEERROR		0xC0
	#define DSR_ERR_FILEERROR		0xE0
	// note that DSR_ERR_DSRNOTFOUND does not fit in the 3 reserved bits!
	#define DSR_ERR_DSRNOTFOUND     0xFF

// DSR_TYPE not intended for comparisons, since there are lots of 0x00 ;)
#define DSR_TYPE_VARIABLE	0x10
#define DSR_TYPE_FIXED		0x00

#define DSR_TYPE_INTERNAL	0x08
#define DSR_TYPE_DISPLAY	0x00

#define DSR_TYPE_UPDATE		0x00
#define DSR_TYPE_OUTPUT		0x02
#define DSR_TYPE_INPUT		0x04
#define DSR_TYPE_APPEND		0x06

#define DSR_TYPE_RELATIVE	0x01
#define DSR_TYPE_SEQUENTIAL	0x00

// these are valid masks for testing after a status call - some devices return after every call
// but this may not be relied upon.
#define DSR_STATUS_NOTFOUND		0x80
#define DSR_STATUS_PROTECTED	0x40
#define DSR_STATUS_INTERNAL		0x10		// else type is display
#define DSR_STATUS_PROGRAM		0x08		// else type is non-program
#define DSR_STATUS_VARIABLE		0x04		// else record length is fixed
#define DSR_STATUS_MEMORYFULL	0x02		// else device has space remaining
#define DSR_STATUS_EOF			0x01		// else not at end of file

// for TI SBRLNK
#define SBR_SECTOR              0x00
#define SBR_FORMAT              0x01
#define SBR_PROTECT             0x02
#define SBR_RENAME              0x03
#define SBR_FILESECTORREAD      0x04
#define SBR_FILESECTORWRITE     0x05
#define SBR_FILES               0x06        // note: always SBR_PREFIX_DEFAULT
#define SBR_SETPATH             0x07
#define SBR_MKDIR               0x08
#define SBR_RMDIR               0x09
#define SBR_RENAMEDIR           0x0a
#define SBR_IDENTIFYSCSI        0x0c        // note: always SBR_PREFIX_WDS or SBR_PREFIX_IDE, different returns

#define SBR_PREFIX_DEFAULT      0x10
#define SBR_PREFIX_WDS          0x20
#define SBR_PREFIX_IDE          0x80
#define SBR_PREFIX_HDX          0x90

typedef struct {
    unsigned int buffer;
    unsigned int first_sector;
    unsigned char flags;
    unsigned char recs_per_sect;
    unsigned char eof_offset;
    unsigned char record_length;
    unsigned int record_count;
    unsigned char msb_first_sect;       // extended if > 65536 sectors on some devices
    unsigned char msb_record_count;     // extended if > 65536 sectors on some devices
    unsigned int extended_record_length;    // geneve? if record_length is >255 bytes, the above field is 0 and this one used instead
} DSR_FILE_INFO_STRUCT;

// PAB struct
#ifdef TI99
// WARNING: gcc port has bugs when packed is declared - have to assume alignment is okay
//struct __attribute__((__packed__)) PAB {
struct PAB {
#endif
#ifdef COLECO
struct PAB {
#endif
#ifdef GBA
struct PAB {
#endif
#ifdef CLASSIC99
#pragma pack(push, 1)
struct PAB {
#endif
	unsigned char OpCode;			// see DSR_xxx list above
	unsigned char Status;			// file type and error code (DSR_ERR_xxx and DSR_TYPE_xxx)
	unsigned short VDPBuffer;		// address of the data buffer in VDP memory
	unsigned char RecordLength;	// size of records. Not used for PROGRAM type. >00 on open means autodetect
	unsigned char CharCount;		// number of bytes read or number of bytes to write
	unsigned short RecordNumber;	// record number for normal files, available bytes (LOAD or SAVE) for PROGRAM type
	unsigned char ScreenOffset;	// Used in BASIC for screen BIAS. Also returns file status on Status call. (DSR_STATUS_xxx)
	
	unsigned char NameLength;		// for this implementation only, set to zero to read the length from the string
	unsigned char *pName;			// for this implementation only, must be a valid C String even if length is set
};
#ifdef CLASSIC99
#pragma pack(pop)
#endif

// Set maximum number of open files
// Inputs: number of files (1-9 valid on most devices, consumes VRAM from the top of memory)
void files(unsigned char count);

// Perform a GPL DSRLNK - this uses the routine in the console GROMs and can handle assembly and GPL
// DSRs both. It requires the GPL areas of the scratchpad be unchanged.
// Inputs: pab - pointer to the populated PAB struct
//         vdp - address in VDP to store the PAB (10 bytes plus length of filename)
// Returns: 0 on success, or PAB error on failure
//unsigned char gpldsrlnk(struct PAB *pab, unsigned int vdp);		NOT IMPLEMENTED YET

// Perform a DSR function per the passed in PAB, which is installed in VRAM at the specified address
// Inputs: pab - pointer to the populated PAB struct
//         vdp - address in VDP to store the PAB (10 bytes plus length of filename)
// Returns: 0 on success, or PAB error on failure
unsigned char dsrlnk(struct PAB *pab, unsigned int vdp);

// Execute a DSR link on the PAB already in VDP. Use this if you know the VDP PAB is already
// updated and you don't want the overhead of copying it again.
// Inputs: pointer to PAB in VDP
// Returns 0 on success, or 1 on error. Read error code from PAB+1. If it's 0, the DSR was not found.
// For SBRLNK calls, refer to each individual documentation.
unsigned char dsrlnkraw(unsigned int vdppab);

//*********************************************************
//*********************************************************
//**** WARNING! All functions below this point are UNTESTED
//**** They may corrupt your files! Use with care!
//*********************************************************
//*********************************************************

// you must pass a complete path, not a relative one. Note that relatively few
// devices support subdirectories. Uses set current path first, as required by the spec.
// Supports long filenames but note that even fewer devices support them.
// Does NOT support CPU buffers. How complex do we need to make this? ;)
// VDP buffer must be large enough to contain the path in full - minimum of 39 characters.
// We'll try to determine the correct subprogram number by name, but bleh.
// I guess they did that to make dsr chaining easier, since older cards did
// not foresee the arrival of newer ones.
// We specify the len so the string doesn't need to be modified for mkdir, but
// if len is 0 we'll do a strlen for you.
// Warning: setpath is automatically set by mkdir, rmdir, and rename with subdirs,
// so this function is probably not useful to call directly. TI paths do not apply
// to any other operations.
unsigned char dsr_setpath(const char *szPath, unsigned int len, unsigned int vdp);

// you must pass a complete path, not a relative one. Note that relatively few
// devices support subdirectories. Uses set current path first, as required by the spec.
// Supports long filenames but note that even fewer devices support them.
// Does NOT support CPU buffers. How complex do we need to make this? ;)
// VDP buffer must be large enough to contain the path in full - minimum of 39 characters.
// We'll try to determine the correct subprogram number by name, but bleh.
// I guess they did that to make dsr chaining easier, since older cards did
// not foresee the arrival of newer ones.
unsigned char dsr_mkdir(const char *szPath, unsigned int vdp);
unsigned char dsr_rmdir(const char *szPath, unsigned int vdp);

// parses a filename to find the split between device and file, and also
// checks lengths and manages long filename support for SBR functions
// Returns the index of the first '.' or 0 if not found (usually an error)
// if bTest is 0, then does not test or strip, and the return value should be ignored
unsigned char dsr_writefilename(const char *szPath, unsigned int vdp, int bTest);

// Path to the file (LFN will be used if longer than 10 characters)
// VDP address to use as a name buffer, separate one for data buffer
// first sector to read
// number of sectors to read - if 0 then first is ignored, and the file information is generated instead
// buf is the CPU buffer for the info block (if NULL, will be placed at 0x8352). It's up to 14 bytes on some hardware
unsigned char readfilesect(const char *szPath, unsigned int vdppath, unsigned int vdpdata, unsigned int start, unsigned int count, char *info);

// Path to the file (LFN will be used if longer than 10 characters)
// VDP address to use as a name buffer, separate one for data buffer
// first sector to write
// number of sectors to write - if 0 then first is ignored, and the file information is used to start a new file instead
// buf is the CPU buffer for the info block (if NULL, will be placed at 0x8352). It's up to 14 bytes on some hardware
unsigned char writefilesect(const char *szPath, unsigned int vdppath, unsigned int vdpdata, unsigned int start, unsigned int count, char *info);

// Path can be just the device name, vdp address for buffer, and sector number (up to 32-bits for larger devices)
// sector_most will be zero on most TI devices. You hit 16MB before you need more. Only WDS, SCS and IDE support.
// only supports "DSKx" style device names, and assumes indexes match unit numbers.
// Longer or shorter device names will malfunction.
unsigned char readsector(const char *szPath, unsigned int vdp, unsigned int sector_low, unsigned int sector_high);

// Path can be just the device name, vdp address for buffer, and sector number (up to 32-bits for larger devices)
// sector_most will be zero on most TI devices. You hit 16MB before you need more. Only WDS, SCS and IDE support.
// only supports "DSKx" style device names, and assumes indexes match unit numbers.
// Longer or shorter device names will malfunction.
unsigned char writesector(const char *szPath, unsigned int vdp, unsigned int sector_low, unsigned int sector_high);

// szPath is the complete path to the file to rename
// vdp is a pointer to a VDP buffer large enough for both filenames
// szNewName is the new name to set - no path attached
// isSubdir must be set if it is in a subdirectory so that setpath can be called
// isDir indicates we must call rename directory, rather than rename file
unsigned char dsr_renamefile(const char *szPath, unsigned int vdp, const char *szNewName, unsigned int isSubdir, unsigned int isDir);

// check the error code at >8350, adapting for the SCS bug
unsigned char sbr_error_check(const char *p);

#if defined(TI99)

// Load and run a PROGRAM image file (TI-99/4A focused)
// You are expected to have already verified that it IS an EA#5 program
// return is not guaranteed in case of error, but any return means it failed
// If migrating to other platforms, this is the one to use
void dsr_ea5ld(const char *pPath);

// Load and run an Extended BASIC program. You are expected to verify that it is
// an Extended BASIC program in either PROGRAM or IV254 format. You must also verify
// that an Extended BASIC is in the console at GROM base 0, and pass the XB start address.
// This function is unlikely to return, but if it does, that means it failed.
void dsr_xbld(const char *pPath, unsigned int nstart);

// Although SBRLNK doesn't typically require a PAB setup, we do one anyway so we can reuse
// the code in dsrlnkraw. You need about 12 bytes. You must have set up the rest of scratchpad
// for your particular subprogram before calling this.
unsigned char sbrlnk(unsigned char subprogram);

// based on the device name, returns the SBR subprogram prefix (>10, >20, >80, >90)
unsigned char getsbrbase(const char *pszPath);

#endif

#ifdef __cplusplus
}   // extern C
#endif

#endif /* FILES_H */
