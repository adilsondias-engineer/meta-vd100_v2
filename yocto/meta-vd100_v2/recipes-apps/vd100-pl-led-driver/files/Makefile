#CC      = gcc
#CFLAGS  = -g++
RM      = rm -f
LDFLAGS += -lgpiod
LDFLAGS += -lgpiodcxx

default: all

all: build install

build: 
	$(CXX) $(CFLAGS) pl_led_driver.cpp -o pl_led_driver $(LDFLAGS)
#I$(INCLUDE)	

install:
	install -d $(DESTDIR)$(BINDIR)
	install -m 0755 pl_led_driver $(DESTDIR)$(BINDIR)

clean veryclean:
	$(RM) pl_led_driver

