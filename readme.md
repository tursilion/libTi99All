(n/a)

This library is released by Tursi aka Mike Brent for TI-99/4A coding via GCC. It is released to the public domain with no restrictions (but credit would be nice if you use it). Likewise, I make no guarantees or promises!

** TI-99/4A **
This code is intended to be used with the version 1.30 patches to GCC 4.4.0 as described in this thread on AtariAge:

https://forums.atariage.com/topic/164295-gcc-for-the-ti/page/45/#comments

And found at this repository:
https://github.com/mburkley/tms9900-gcc

Recommended build switches are: -Os -fno-builtin -fno-function-cse -fno-peephole2

-Os - optimize for size (or -O0) produces the fastest code as well in most cases on the 9900
-fno-builtin - prevents replacing calls to library functions with simpler ones that may not exist in libti99
-fno-function-cse - prevents some very inefficient function call semantics that help only in very limited cases
-fno-peephole2 - turns off peepholes which contain a code-breaking bug in the 1.30 patches (peep-movhi-cmphi)

** ColecoVision, SMS, [MSX?] **
This code is tested with SDCC dated 20230715

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

......

Coleco Testlib:
- fastline draw and erase are inverted
- there is no fastline xor (it's draw)
- 64 character mode is still borked (probably memory?)

SMS Example:
- colors are wrong

SMS TestLib:
- first screen not displaying correctly (do we have enough RAM for this app?)
- can't answer Y/N anyway

........

** GBA **

This code is tested with devkitarm's GCC 12.1.0 (devkitARM release 58)

- this is never meant to be a first tier target, I'm just using it to help a porting effort
- the testlib app is largely non-functional and makes a lot of noise due to not servicing the audio emulator, but some of it works!
- you can use setGBAAutoRender() to render graphics or bitmap onto the GBA screen automatically when reading status
- you can also use gbaRender() to force a draw. Note that the drawing takes many frames and will glitch audio
- the SN sound chip is faithfully emulated as long as it's updated frequently enough
- the GBA screen is smaller than the TMS9900 so bottom and right edge are clipped in the FULL render mode

........

For docs:

- remember to include -DTI99 (or appropriate for others) on your C command line
- inline rules follow https://gcc.gnu.org/onlinedocs/gcc/Inline.html - most calls should end up inlined most of the time
- Note that where #define code is used, I have no sympathy for people who don't use braces in single-line conditionals.
I have done enough late nights debugging such code and I'm not making accomodations for it in hobby work. ;) If you propose the while(0) solution, the most polite thing I will do is ignore you. It's a bad answer to a bad habit.
