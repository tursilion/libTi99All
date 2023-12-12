#include "vdp.h"

void fast_hexprint(unsigned char x) {
	char buf[3];

#ifdef TI99
    // we can cheat and do this cause we know we're big endian
    *(unsigned int*)buf = byte2hex[x];
#else
	unsigned int dat = byte2hex[x];
	buf[0] = dat>>8;
	buf[1] = dat&0xff;
#endif

	buf[2]='\0';

	putstring(buf);
}
