EE_BIN = MFU.elf
EE_BIN_PACKED = MFU-packed.elf

# C File
EE_OBJS = main.o mtaphelper.o
# SW Module
EE_OBJS += freesio2.o freepad.o mcman.o mcsrv.o freemtap.o poweroff.o
 
EE_LIBS = -lpadx -lmtap -ldebug -lmc -lc -lpatches -ldebug -lkernel -lpoweroff



all: $(EE_BIN)
	rm -rf *.o *.s

clean:
	rm -f *.elf *.o *.s
	
#poweroff Module
poweroff.s:
	bin2s $(PS2SDK)/iop/irx/poweroff.irx poweroff.s poweroff

#IRX Modules
freemtap.s:
	bin2s $(PS2SDK)/iop/irx/freemtap.irx freemtap.s mtapman
	
freesio2.s:
	bin2s $(PS2SDK)/iop/irx/freesio2.irx freesio2.s freesio2
	
freepad.s:
	bin2s $(PS2SDK)/iop/irx/freepad.irx freepad.s freepad
	
mcman.s:
	bin2s $(PS2SDK)/iop/irx/mcman.irx mcman.s mcman
	
mcsrv.s:
	bin2s $(PS2SDK)/iop/irx/mcserv.irx mcsrv.s mcserv
	
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal