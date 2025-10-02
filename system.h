#ifndef SYSTEM_H
#define SYSTEM_H

#ifdef TI99
// Halt function -- stops the program and waits for
// the user to press QUIT (Fctn-=). Use this if you
// do not want to simply exit main (which in my startup
// will reset the console, and in the default setup
// will just enter an infinite loop).
void halt() __attribute__ ((noreturn));

// Exit function -- reboots the console.
void exit() __attribute__ ((noreturn));
#endif

#ifdef COLECO
// Halt still freezes and waits, but only the reset button gets you out
void halt();

// Exit restarts the cartridge
void exit();
#endif

#ifdef GBA
// Halt still freezes and waits, but only the reset button gets you out
void halt();

// Exit restarts the cartridge
void exit();
#endif

#ifdef CLASSIC99
// Halt still freezes and waits, but only the reset button gets you out
void halt();

// Exit restarts the cartridge
#define exit exit_ti
void exit();
#endif


// return true if quit/reset is pressed
// if you have F18A in your system, call reset_f18a()
// then use exit() to reboot
unsigned char check_reset();

#endif /* SYSTEM_H */
