PREFIX=./
SRCDIR=$(PREFIX)src
BINDIR=$(PREFIX)bin
TCPLIBDIR=$(SRCDIR)/TCPLib
SRCS = queue.$(CPP) RTPpacket.$(CPP) rtspServer.$(CPP) ServerSocket.$(CPP) Socket.$(CPP) audio.$(C) video.$(C) server.$(C) util.$(C)

BIN = Server
OBJS += $(addsuffix .o,$(basename $(SRCS)))
BINOBJS += $(addprefix $(BINDIR)/,$(OBJS))

C=c
CC=gcc
CPP=cpp
CPPC=g++
CLFLAGS ?= -g
CLFLAGS += -Wall 
STDFLAGS += "-std=gnu99"

OBJCPP = $(CPPC) $(CLFLAGS) -c
OBJC = $(CC) -c            # The proper way is to add $(CLFLAGS) here, but at the mmoment throws some warnings

ifeq ($(shell uname -s), Linux)
LDFLAGS += -lboost_thread -lavutil -lavformat -lavcodec -lavdevice -lswscale
endif

all:$(OBJS)
	$(CPPC) $(CLFLAGS) $(STDFLAGS) $(BINOBJS) $(LDFLAGS) -o $(BIN)
%.o:$(SRCDIR)/%.$(C)
	$(OBJC) $< -o $(BINDIR)/$@
%.o:$(SRCDIR)/%.$(CPP)
	$(OBJCPP) $< -o $(BINDIR)/$@
%.o: $(TCPLIBDIR)/%.$(CPP) 
	$(OBJCPP) $< -o $(BINDIR)/$@
clean:
	rm $(BINDIR)/*.o $(PREFIX)$(BIN)
