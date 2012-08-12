-include Makefile.local
# Makefile.local defines (or not) the following:
# MAC
# RELEASE

ifdef NUMA
CFLAGS += -D__NUMA_ENABLED__
LFLAGS += -lnuma
endif

ifdef MAC
CFLAGS += -I/opt/local/include/
CFLAGS += -D__MAC_OSX__
else
LFLAGS += -lrt
endif

ifdef DEBUG
CFLAGS    += -Wall -ggdb -fno-inline -DLOCAL_DEBUG_ENABLED
else
CFLAGS    += -Wall
OPTFLAG   += -O3
endif
LFLAGS += -Wall

INCLUDES  = -I.
CC = g++

OBJDIR  = ../obj
BINDIR  = ../bin

all: dirs $(BINDIR)/mem-latency
  
.PHONY: dirs clean

$(BINDIR)/mem-latency: $(OBJDIR)/mem-latency.o $(OBJDIR)/TimeDiff.o $(OBJDIR)/MallocAlign.o $(OBJDIR)/PointerChase.o
	$(CC) -o $(BINDIR)/mem-latency $(OBJDIR)/mem-latency.o $(OBJDIR)/TimeDiff.o $(OBJDIR)/MallocAlign.o $(OBJDIR)/PointerChase.o $(LFLAGS)

$(OBJDIR)/mem-latency.o: mem-latency.cpp
	$(CC) -c mem-latency.cpp -o $(OBJDIR)/mem-latency.o $(CFLAGS) $(OPTFLAG) $(INCLUDES)

$(OBJDIR)/MallocAlign.o: MallocAlign.cpp
	$(CC) -c MallocAlign.cpp -o $(OBJDIR)/MallocAlign.o $(CFLAGS) $(OPTFLAG) $(INCLUDES)

$(OBJDIR)/TimeDiff.o: TimeDiff.cpp
	$(CC) -c TimeDiff.cpp -o $(OBJDIR)/TimeDiff.o $(CFLAGS) $(OPTFLAG) $(INCLUDES)

$(OBJDIR)/PointerChase.o: PointerChase.cpp
	$(CC) -c PointerChase.cpp -o $(OBJDIR)/PointerChase.o $(CFLAGS) $(INCLUDES)

dirs:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)

clean:
	\rm -f $(OBJDIR)/*
	\rm -f $(BINDIR)/*
	\rm -f *~
	\rm -f *.o

