RM = cmd //c del

all:
	echo Try make ti, make coleco, or make sms. Try make clean between them.

# Recipe to clean all compiled objects
.phony clean:
	-rm *.o
	-rm *.a
	-rm *.s
	-rm *.i
	-rm *.elf
	-rm *.map
	-rm *.bin
	-rm *.rel *.map *.lst *.lnk 
	-rm *.sym *.asm *~ *.rel
	-rm *.relbj *.ihx *.rom 
	-rm *.a *.lib
	-rm *.ihx *.lk *.noi
	-rm TESTLI*
	-rm EXAMPL*
	-rm *.rom
	-rm *.sms
	
.phony2 ti:
	make -f Makefile.ti99 all
	
.phony3 coleco:
	make -f Makefile.coleco all
	
.phony4 sms:
	make -f Makefile.mastersys all
    
