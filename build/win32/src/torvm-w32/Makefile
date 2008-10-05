# Makefile for Tor VM win32 process and network control utility

.PHONY: all clean distclean install 

BINDIR=C:\Tor_VM
CPPFLAGS += -I. -I/usr/include -I/usr/local/include -MMD -MP
CPPFLAGS += -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
#LDFLAGS += -L/usr/lib -L/usr/local/lib -lws2_32 -liphlpapi
LDFLAGS += -lws2_32 -liphlpapi

all: torvm$(EXESUF)

OBJS+=torvm.o torvmrc.o

torvm$(EXESUF): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

torvm.o: torvm.c
	$(CC) $(CPPFLAGS) -c -o $@ $<

torvmrc.o: torvm.rc
	windres torvm.rc torvmrc.o

clean:
	rm -f *.o *.d torvm$(EXESUF)


install: all
	ls -l torvm$(EXESUF)
	cp -f torvm$(EXESUF) $(BINDIR)
