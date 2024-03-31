#include <vdp.h>
#include <kscan.h>
#include <string.h>

// TODO: the response time on this is terrible, you can barely type.
// getchar might have to be less fussy about releases and just look for change.

// read from keyboard - limits to maxlen chars
// Coleco uses '*' for return
// no autorepeat :)
void gets(char *buf, int maxlen) {
  int oldch;
  int cnt = maxlen;
  
  while (cnt) {
    vsetchar(nTextPos, 30);  // cursor
    oldch = getchar();
    switch (oldch) {
      case '\r':
#ifdef COLECO
      case '*':
#endif
        cnt = 0;
        *buf = '\0';
        vsetchar(nTextPos, ' ');
        putchar('\n');
        // wait for enter to be released
        while (kscan(5) == oldch) {
          VDP_INT_ENABLE;
          VDP_INT_DISABLE;
        }
        break;
      
      case 8:   // backspace
        if (cnt < maxlen) {
          ++cnt;
          vsetchar(nTextPos, ' ');
          --nTextPos;
          --buf;
        }
        break;
      
      default:
        if ((oldch >= ' ') && (oldch <= 'z') && (cnt > 1)) {
          putchar(oldch);
          *(buf++)=oldch;
          --cnt;
        }
        break;
    }
  }
}
