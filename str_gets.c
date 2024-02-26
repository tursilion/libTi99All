#include <vdp.h>
#include <kscan.h>

// read from keyboard - limits to maxlen chars
// no autorepeat :)
void gets(char *buf, int maxlen) {
  char oldch;
  int cnt = maxlen;
  
  oldch = 255;
  
  while (cnt) {
    vsetchar(nTextPos, 30);  // cursor
    while (kscan(5) == oldch) {  // wait for key, allow interrupts
      VDP_INT_ENABLE;
      VDP_INT_DISABLE;
    }
    oldch = KSCAN_KEY;
    switch (oldch) {
      case '\r':
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
