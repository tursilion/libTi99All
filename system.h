#ifndef SYSTEM_H
#define SYSTEM_H

// We had to stop using exit() due to conflicts, use lib99_exit(int) now.

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TI99
// Halt function -- stops the program and waits for
// the user to press QUIT (Fctn-=). Use this if you
// do not want to simply exit main (which in my startup
// will reset the console, and in the default setup
// will just enter an infinite loop).
void halt() __attribute__ ((noreturn));

// Exit function -- reboots the console.
void lib99_exit(int n) __attribute__ ((noreturn));
#endif

#ifdef COLECO
// Halt still freezes and waits, but only the reset button gets you out
void halt();

// Exit restarts the cartridge
void lib99_exit(int n);
#endif

#ifdef GBA
// Halt still freezes and waits, but only the reset button gets you out
void halt()  __attribute__ ((noreturn));

// Exit restarts the cartridge
void lib99_exit(int n)  __attribute__ ((noreturn));
#endif

#ifdef CLASSIC99
// Halt still freezes and waits, but only the reset button gets you out
void halt();

// Exit restarts the cartridge - still want the override here so Classic99 gets the message
void lib99_exit(int n);

// Commands for the Classic99 debug stub
#define STUB_ADDRESS 0xA100
#define STUB_LIMI_2 1
#define STUB_LIMI_0 2
#define STUB_EXIT 3

#endif

// return true if quit/reset is pressed
// if you have F18A in your system, call reset_f18a()
// then use lib99_exit() to reboot
unsigned char check_reset();

#ifdef __cplusplus
}   // extern C
#endif

#endif /* SYSTEM_H */
