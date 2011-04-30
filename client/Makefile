PREFIX=./

SRCDIR=$(PREFIX)src
BINDIR=$(PREFIX)bin
EXEDIR=$(PREFIX)exec

TCPLIBDIR=$(SRCDIR)/TCPLib
SRCP = player.$(CPP)
SRCS = ClientSocket.$(CPP) netClient.$(CPP) queue.$(CPP) RTPpacket.$(CPP) Socket.$(CPP)

FRSEXEC = client
SCNEXEC = player
OBJS += $(addsuffix .o,$(basename $(SRCS)))
OBJP += $(addsuffix .o,$(basename $(SRCP)))
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

all: $(OBJS) $(OBJP)
	$(CPPC) $(CPPFLAGS) $(STDFLAGS) $(BINOBJS) $(LDFLAGS) -o $(EXEDIR)/$(FRSEXEC)
$(OBJP): $(SRCDIR)/$(SRCP)
	$(CPPC) $(CPPFLAGS) $(SRCDIR)/$(SRCP) $(VDFLAGS) -o $(EXEDIR)/$(SCNEXEC)
	cp $(SRCDIR)/runClient $(PREFIX)runClient
%.o: $(SRCDIR)/%.$(CPP)
	$(COMPILE) $< -o $(BINDIR)/$@
%.o: $(TCPLIBDIR)/%.$(CPP)
	$(COMPILE) $< -o $(BINDIR)/$@
clean:
	rm $(BINDIR)/*.o $(EXEDIR)/$(FRSEXEC) $(EXEDIR)/$(SCNEXEC) $(PREFIX)runClient
