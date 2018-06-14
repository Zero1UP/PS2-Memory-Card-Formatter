EE_BIN = mcFormatter.elf
EE_BIN_PACKED = mcF-packed.elf
EE_OBJS = main.o poweroff.o
EE_LDFLAGS = -L$(PS2SDK)/sbv/lib 
EE_LIBS = -lpadx -lmtap -ldebug -lmc -lc -lpatches -ldebug -lkernel -lpoweroff
EE_INCS = -I$(PS2SDK)/ports/include -I$(PS2SDK)/sbv/include
EE_LDFLAGS = -nostartfiles -L$(PS2SDK)/ports/lib \
	-L$(PS2SDK)/sbv/lib -L. -s
IRX = $(PS2SDK)/iop/irx

all: $(EE_BIN)

# Un-Comment to Enable Compression of the ELF. you will need ps2packer in the project dir
#all: $(EE_BIN)
	ps2_packer/ps2_packer -p zlib $(EE_BIN) $(EE_BIN_PACKED)
	cp -f --remove-destination $(EE_BIN_PACKED) $(EE_BIN_DIR)/$(EE_BIN)


clean:
	rm -f *.elf *.o *.a poirx.c

poweroff_irx.c: $(PS2SDK)/iop/irx/poweroff.irx
	bin2c $(PS2SDK)/iop/irx/poweroff.irx poirx.c poweroff_irx

	
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
