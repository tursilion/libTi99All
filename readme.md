(n/a)

This library is released by Tursi aka Mike Brent for TI-99/4A coding via GCC. It is released to the public domain with no restrictions (but credit would be nice if you use it). Likewise, I make no guarantees or promises!

Documentation is found in each of the .h files. The .h files are briefly described here:

- grom.h   - helper definitions and functions for accessing GROM
- kscan.h  - include file for using the console's kscan code (including joysticks)
- player.h - include file for using my compressed VGM audio player
- sound.h  - helper definitions for the sound chip and console sound interrupt player
- vdp.h    - helper definitions and functions for the VDP and console interrupt
- puff.h   - deflate implementation - from the zlib library and created by Mark Adler - madler@alumni.caltech.edu
- conio.h  - console functions (including cprintf) for basic screen I/O
- files.h  - dsrlnk and related functions and constants for native TI device IO.
- math.h   - numeric functions (sqrt, abs)
- string.h - string manipulation and conversion functions
- system.h - halt and exit functions 

Note that the current version of puff has all error handling disabled, it just calls halt().
So test your streams before using it!

The GCC patches and build instructions (not related to this lib) are here: http://atariage.com/forums/topic/164295-gcc-for-the-ti/page__st__125

...

Bug in testlib1:

Only in 64-column mode, only with the new compiler, and only with libti99ALL, the printf test fails:

(03d)
-12345 =0-12345 (should be -12345 = -12345)

(Hello)
3Hello'      (should be 'Hello')
|     Hell*  (*=garbage character, should be '     Hello')
oHello    *  (should be 'Hello     ')
 Hel'        (should be 'Hel')

Decimal (123 -456): 12* -456     (should be 123 -456)
Unsigned (123 65080): 12* 65080  (should be 123 65080)

Octal (17001):c17001  (should be 17001 (space instead of 'c'))

Screen is 6462*, Cursor at40423  (should be Screen is 64x24, Cursor at 0,23)

