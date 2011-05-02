PREFIX=./

SRCDIR=$(PREFIX)src
BINDIR=$(PREFIX)bin
EXEDIR=$(PREFIX)exec

TCPLIBDIR=$(SRCDIR)/TCPLib
SRCP = player.$(CPP)
SRCS = ClientSocket.$(CPP) netClient.$(CPP) queue.$(CPP) RTPpacket.$(CPP) Socket.$(CPP)
RUNC = runClient.$(CPP)

FRSEXEC = Client
SCNEXEC = player
TRDEXEC = runClient
OBJS += $(addsuffix .o,$(basename $(SRCS)))
OBJP += $(addsuffix .o,$(basename $(SRCP)))
OBJR += $(addsuffix .o,$(basename $(RUNC)))
OBJFILES := $(patsubst %.cpp,%.o,$(wildcard $(SRCS)))
BINOBJS += $(addprefix $(BINDIR)/,$(OBJS))

CPP=cpp
CPPC=g++
CPPFLAGS ?= -g
CPPFLAGS += -Wall
STDFLAGS += "-std=gnu99"

COMPILE = $(CPPC) $(CPPFLAGS) -c

ifeq ($(shell uname -s), Linux)
VDFLAGS += -lvlc
LDFLAGS += -lboost_thread $(VDFLAGS)
endif

all: $(OBJS) $(OBJP) runClient.o
	$(CPPC) $(CPPFLAGS) $(STDFLAGS) $(BINOBJS) $(LDFLAGS) -o $(EXEDIR)/$(FRSEXEC)
$(OBJP): $(SRCDIR)/$(SRCP)
	$(CPPC) $(CPPFLAGS) $(SRCDIR)/$(SRCP) $(VDFLAGS) -o $(EXEDIR)/$(SCNEXEC)
$(OBJR): $(SRCDIR)/$(RUNC)
	$(CPPC) $(CPPFLAGS) $(SRCDIR)/$(RUNC) -o $(PREFIX)$(TRDEXEC)
%.o: $(SRCDIR)/%.$(CPP)
	$(COMPILE) $< -o $(BINDIR)/$@
%.o: $(TCPLIBDIR)/%.$(CPP)
	$(COMPILE) $< -o $(BINDIR)/$@
clean:
	rm $(BINDIR)/*.o $(EXEDIR)/$(FRSEXEC) $(EXEDIR)/$(SCNEXEC) $(PREFIX)$(TRDEXEC)
