// check if QUIT is pressed, and reboot if so

#ifdef TI99
void checkquit() {
__asm__("clr @>83C4\n\tli r12,>0024\n\tldcr @>0012,3\n\tsrc r12,7\n\tli r12,6\n\tstcr r0,8\n\tandi r0,>1100\n\tjne 4\n\tblwp @>0000" : : : "r12","r0");
}
#endif

#ifdef COLECO
// TODO: we could detect a key sequence to reset, but the hardware has a reset button
void checkquit();
#endif
