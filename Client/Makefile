PREFIX=./

SRCDIR=$(PREFIX)src
BINDIR=$(PREFIX)bin
EXEDIR=$(PREFIX)exec

TCPLIBDIR=$(SRCDIR)/TCPLib


SRCS = ClientSocket.$(CPP) netClient.$(CPP) RTPpacket.$(CPP) Socket.$(CPP)
RUNC = runClient.$(CPP)


FRSEXEC = Client
TRDEXEC = runClient

OBJS += $(addsuffix .o,$(basename $(SRCS)))

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

all: $(OBJS) $(OBJR) 
	$(CPPC) $(CPPFLAGS) $(STDFLAGS) $(BINOBJS) $(LDFLAGS) -o $(EXEDIR)/$(FRSEXEC)
$(OBJR): $(SRCDIR)/$(RUNC)
	$(CPPC) $(CPPFLAGS) $(SRCDIR)/$(RUNC) -o $(PREFIX)$(TRDEXEC)
%.o: $(SRCDIR)/%.$(CPP)
	$(COMPILE) $< -o $(BINDIR)/$@
%.o: $(TCPLIBDIR)/%.$(CPP)
	$(COMPILE) $< -o $(BINDIR)/$@
clean:
	rm $(BINDIR)/*.o $(EXEDIR)/$(FRSEXEC) $(PREFIX)$(TRDEXEC) 
