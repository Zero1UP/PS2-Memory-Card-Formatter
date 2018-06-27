EE_BIN = mcFormatter.elf
EE_BIN_PACKED = mcF-packed.elf
####
# C File Objects
EE_OBJS = main.o
# SW Module Objects
EE_OBJS += freesio2.o freepad.o freemtap.o
# Other IRX
EE_OBJS += poweroff.o
####
EE_LIBS = -lpadx -lmtap -ldebug -lmc -lc -lpatches -ldebug -lkernel -lpoweroff


all: $(EE_BIN)
	rm -rf *.o *.s

# Un Comment to Enable Compression of the ELF. you will need ps2packer in the project dir

# all: $(EE_BIN)
	# ps2_packer/ps2_packer -p zlib $(EE_BIN) $(EE_BIN_PACKED)
	# cp -f --remove-destination $(EE_BIN_PACKED) $(EE_BIN_DIR)/$(EE_BIN)
	# rm -rf *.o *.s


clean:
	rm -f *.elf *.o *.s


#poweroff Module

poweroff.s:
	bin2s $(PS2SDK)/iop/irx/poweroff.irx poweroff.s poweroff

#IRX Modules

freesio2.s:
	bin2s $(PS2SDK)/iop/irx/freesio2.irx freesio2.s freesio2

freepad.s:
	bin2s $(PS2SDK)/iop/irx/freepad.irx freepad.s freepad
	
freemtap.s:
	bin2s $(PS2SDK)/iop/irx/freemtap.irx freemtap.s mtapman

	
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
