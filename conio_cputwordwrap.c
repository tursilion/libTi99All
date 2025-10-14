#include "conio.h"
#include "string.h"

// cnt is used to limit the string to before the NUL termination
// if zero, then the whole string is used - no control codes, direct to output function
void cputwordwrap(int x, int y, const char *pWork, int cnt) {
    int width = 32;
    // I do not like this, set width in a variable like everything else
	if (nTextFlags&TEXT_WIDTH_40) {
        width=40;
	} else if (nTextFlags&TEXT_WIDTH_80) {
        width=80;
	} else if (nTextFlags&TEXT_WIDTH_64) {
        width=64;
    }
    if (cnt == 0) {
        cnt = strlen(pWork);
    }

    while (*pWork) {
        // check for space
        if (*pWork == ' ') {
            // ignore leading spaces
            if (x > 0) {
                gotoxy(x,y);
                vsetchar(getscreenoffset(x, y), ' ');  // direct output
                x++;
                if (x>=width) {
                    x=0;
                    ++y;
                    if (y > 23) break; // out of lines
                }
            }
            pWork++;
            if (--cnt == 0) break;
            continue;
        }
        // check first if this line fits
        int pos = 0;
        while (pWork[pos]) {
            if (pWork[pos] == ' ') break;
            ++pos;
            if (pos+x >= width) break;
        }
        if (pos+x >= width) {
            if (x > 0) {
                // too long, and wasn't the whole line, so next line
                y++;
                if (y > 23) break; // out of lines
                x=0;
            } else {
                // just reduce to one line
                --pos;
            }
        }
        // output the characters
        while (pos--) {
            gotoxy(x,y);
            vsetchar(getscreenoffset(x, y), *(pWork++));  // direct output
            x++;
            if (*pWork == '\0') break;
            if (--cnt == 0) break;
        }
        if (cnt == 0) break;    // again
    }

}
