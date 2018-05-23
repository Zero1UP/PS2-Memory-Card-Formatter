EE_BIN = mcFormatter.elf
EE_OBJS = main.o
EE_LDFLAGS = -L$(PS2SDK)/sbv/lib 
EE_LIBS = -lpad -ldebug -lmc -lc
all: $(EE_BIN)

clean:
	rm -f *.elf *.o *.a

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
