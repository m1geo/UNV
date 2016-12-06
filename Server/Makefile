PREFIX=./
SRCDIR=$(PREFIX)src
BINDIR=$(PREFIX)bin
TCPLIBDIR=$(SRCDIR)/TCPLib
SRCS = queue.$(CPP) RTPpacket.$(CPP) Server.$(CPP) ServerSocket.$(CPP) Socket.$(CPP)

BIN = Server
OBJS += $(addsuffix .o,$(basename $(SRCS)))
BINOBJS += $(addprefix $(BINDIR)/,$(OBJS))

CPP=cpp
CPPC=g++
CPPFLAGS ?= -g
CPPFLAGS += -Wall 
STDFLAGS += "-std=gnu99"

COMPILE = $(CPPC) $(CPPFLAGS) -c

ifeq ($(shell uname -s), Linux)
LDFLAGS += -lboost_thread -lavutil -lavformat -lavcodec -lavdevice -lswscale
endif

all:$(OBJS)
	$(CPPC) $(CPPFLAGS) $(STDFLAGS) $(BINOBJS) $(LDFLAGS) -o $(BIN)
%.o: $(SRCDIR)/%.$(CPP)
	$(COMPILE) $< -o $(BINDIR)/$@
%.o: $(TCPLIBDIR)/%.$(CPP) 
	$(COMPILE) $< -o $(BINDIR)/$@
clean:
	rm $(BINDIR)/*.o $(PREFIX)$(BIN)
