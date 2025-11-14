#include "conio.h"
#include "string.h"

// a smaller printf with no formatting, and only ints and strings can be printed

// minimal
void cprintfmini(const char *fmt, ...) {
    int i=0;
    unsigned int u=0;
    char *s=0;

    va_list argp;
    va_start(argp, fmt);

    while (*fmt) {
        if (*fmt != '%') {
            cputc(*fmt);
        } else {
            ++fmt;
            switch (*fmt) {
                case 'c':   // char
                   i = va_arg(argp, int);
                   cputc(i);
                   break;

                case 'd':   // decimal
                    i = va_arg(argp, int);
                    s = int2str(i);
                    while (*s) cputc(*(s++));
                    break;

                case 'u':   // unsigned decimal
                    u = va_arg(argp, unsigned int);
                    s = uint2str(u);
                    while (*s) cputc(*(s++));
                    break;

                case 's':   // string
                    s = va_arg(argp, char*);
                    while (*s) cputc(*(s++));
                    break;

                case 'X':   // uppercase hex
                case 'x':   // hex (a little inefficient..)
                    u = va_arg(argp, unsigned int);
                    i = (u&0xf000)>>12;
                    if (i>9) i+=7;
                    cputc(i+'0');
                    i = (u&0xf00)>>8;
                    if (i>9) i+=7;
                    cputc(i+'0');
                    i = (u&0xf0)>>4;
                    if (i>9) i+=7;
                    cputc(i+'0');
                    i = (u&0xf);
                    if (i>9) i+=7;
                    cputc(i+'0');
                    break;

                case '%':   // percent sign
                    cputc('%');
                    break;

                case '\0':
                    // error - end of string
                    --fmt;
                    break;

                default:
                    cputc(*fmt);
                    break;
            }
        }
        ++fmt;
    }
}

