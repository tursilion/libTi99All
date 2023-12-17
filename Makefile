all:
	echo Try make ti, make coleco, or make sms. Try make clean between them.

# files to delete to clean
.phonya wipefolder:
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

# clean one folder
.phonyb cleanti:
	-mkdir buildti
	cp Makefile.stub buildti/Makefile
	make -C buildti wipefolder
	cd ..
	
.phonyc cleancoleco:    
	-mkdir buildcoleco
	cp Makefile.stub buildcoleco/Makefile
	make -C buildcoleco wipefolder
	cd ..

.phonyd cleansms:
	-mkdir buildsms
	cp Makefile.stub buildsms/Makefile
	make -C buildsms wipefolder
	cd ..

# Recipe to clean all compiled objects
.phonye clean: cleanti cleancoleco cleansms
	@echo Done

.phonyf ti:
	-mkdir buildti
	cp Makefile.stub buildti/Makefile
	make -C buildti -f ../Makefile.ti99 all
	
.phonyg coleco:
	-mkdir buildcoleco
	cp Makefile.stub buildti/Makefile
	make -C buildcoleco -f ../Makefile.coleco all
	
.phonyh sms:
	-mkdir buildsms
	cp Makefile.stub buildti/Makefile
	make -C buildsms -f ../Makefile.mastersys all
