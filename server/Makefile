PREFIX=./
SRCDIR=$(PREFIX)src
BINDIR=$(PREFIX)bin
TCPLIBDIR=$(SRCDIR)/TCPLib
SRCS = queue.$(CPP) RTPpacket.$(CPP)  rtspServer.cpp ServerSocket.$(CPP) Socket.$(CPP) audio.c video.c server.c util.c

BIN = Server
OBJS += $(addsuffix .o,$(basename $(SRCS)))
BINOBJS += $(addprefix $(BINDIR)/,$(OBJS))

CPP=cpp
CC=gcc
CPPC=g++
CPPFLAGS ?= -g
CFLAGS=-c -g
CPPFLAGS += -Wall  -g
STDFLAGS += "-std=gnu99"

COMPILE = $(CPPC) $(CPPFLAGS) -c

ifeq ($(shell uname -s), Linux)
LDFLAGS += -lboost_thread -lavutil -lavformat -lavcodec -lavdevice -lswscale
endif

all:$(OBJS)
	$(CPPC) $(CPPFLAGS) $(STDFLAGS) $(BINOBJS) $(LDFLAGS) -o $(BIN)
server.o :
	$(CC) $(CFLAGS) $(SRCDIR)/server.c -o $(BINDIR)/$@

audio.o:
	$(CC) $(CFLAGS) $(SRCDIR)/audio.c -o $(BINDIR)/$@

video.o:
	$(CC) $(CFLAGS) $(SRCDIR)/video.c -o $(BINDIR)/$@

util.o:
	$(CC) $(CFLAGS) $(SRCDIR)/util.c -o $(BINDIR)/$@

%.o: $(SRCDIR)/%.$(CPP)
	$(COMPILE) $< -o $(BINDIR)/$@
%.o: $(TCPLIBDIR)/%.$(CPP) 
	$(COMPILE) $< -o $(BINDIR)/$@
clean:
	rm $(BINDIR)/*.o $(PREFIX)$(BIN)
