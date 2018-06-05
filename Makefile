EE_BIN = mcFormatter.elf
EE_BIN_PACKED = mcF-packed.elf
EE_OBJS = main.o
EE_LDFLAGS = -L$(PS2SDK)/sbv/lib 
EE_LIBS = -lpadx -lmtap -ldebug -lmc -lc

all: $(EE_BIN)

# Un Comment to Enable Compression of the ELF. you will need ps2packer in the project dir
# all: $(EE_BIN)
	# ps2_packer/ps2_packer -p zlib $(EE_BIN) $(EE_BIN_PACKED)
	# cp -f --remove-destination $(EE_BIN_PACKED) $(EE_BIN_DIR)/$(EE_BIN)


clean:
	rm -f *.elf *.o *.a


	
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
