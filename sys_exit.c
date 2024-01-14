// exit reboots the system

#ifdef TI99
void exit() {
  __asm__ volatile ("clr @>83c4\n\tBLWP @>0000");
}
#endif

#ifdef COLECO
void exit() {
	// reboots the cart but not the BIOS, address defined by the CRT0 in use
	static void (* const hwreboot)()=0x802c;
	hwreboot();
}
#endif
