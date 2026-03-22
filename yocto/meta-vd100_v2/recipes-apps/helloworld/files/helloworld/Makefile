#CC      = gcc
#CFLAGS  = -g++
RM      = rm -f
LDFLAGS += -lgpiod
LDFLAGS += -lgpiodcxx

default: all

all: build install

build:
	$(CXX) $(CFLAGS) helloworld.cpp -o helloworld $(LDFLAGS)
#I$(INCLUDE)	

install:
	install -d $(DESTDIR)$(BINDIR)
	install -m 0755 helloworld $(DESTDIR)$(BINDIR)

clean veryclean:
	$(RM) helloworld

