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
CFLAGS    += -Wall -O3
endif
LFLAGS += -Wall

INCLUDES  = -I.
CC = g++

OBJDIR  = ../obj
BINDIR  = ../bin

all: dirs $(BINDIR)/mem_latency
  
.PHONY: dirs clean

$(BINDIR)/mem_latency: $(OBJDIR)/mem_latency.o $(OBJDIR)/TimeDiff.o $(OBJDIR)/MallocAlign.o
	$(CC) -o $(BINDIR)/mem_latency $(OBJDIR)/mem_latency.o $(OBJDIR)/TimeDiff.o $(OBJDIR)/MallocAlign.o $(LFLAGS)

$(OBJDIR)/mem_latency.o: mem_latency.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c mem_latency.cpp -o $(OBJDIR)/mem_latency.o

$(OBJDIR)/MallocAlign.o: MallocAlign.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c MallocAlign.cpp -o $(OBJDIR)/MallocAlign.o

$(OBJDIR)/TimeDiff.o: TimeDiff.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c TimeDiff.cpp -o $(OBJDIR)/TimeDiff.o

dirs:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)

clean:
	\rm -f $(OBJDIR)/*
	\rm -f $(BINDIR)/*
	\rm -f *~
	\rm -f *.o

