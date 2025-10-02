all:
	echo make [ti|coleco|sms|gba|classic99].

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
	-rm *.obj

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

.phonye cleangba:
	-mkdir buildgba
	cp Makefile.stub buildgba/Makefile
	make -C buildgba wipefolder
	cd ..

.phonyl cleanclassic99:
	-mkdir buildclassic99
	cp Makefile.stub buildclassic99/Makefile
	make -C buildclassic99 wipefolder
	cd ..

# Recipe to clean all compiled objects
.phonyf clean: cleanti cleancoleco cleansms cleangba cleanclassic99
	@echo Done

.phonyg ti:
	-mkdir buildti
	cp Makefile.stub buildti/Makefile
	make -C buildti -f ../Makefile.ti99 all
	
.phonyh coleco:
	-mkdir buildcoleco
	cp Makefile.stub buildti/Makefile
	make -C buildcoleco -f ../Makefile.coleco all
	
.phonyi sms:
	-mkdir buildsms
	cp Makefile.stub buildti/Makefile
	make -C buildsms -f ../Makefile.mastersys all

.phonyj gba:
	-mkdir buildgba
	cp Makefile.stub buildgba/Makefile
	make -C buildgba -f ../Makefile.gba all

.phonyk classic99:
	-mkdir buildclassic99
	cp Makefile.stub buildclassic99/Makefile
	make -C buildclassic99 -f ../Makefile.classic99 all
