#INCLUDE_DIRS = 
#LIB_DIRS = 
#CC=gcc

#CDEFS=
#CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
#CFLAGS=  -O0 -g $(CDEFS)
LIBS= 

default: all

all: build install

build: pthread.o
	$(CC) $(LDFLAGS) $(CFLAGS) pthread.c -o pthread -lpthread

depend:

install:
	install -d $(DESTDIR)$(BINDIR)
	install -m 0755 pthread $(DESTDIR)$(BINDIR)

clean:
	-rm -f *.o *.d
	-rm -f pthread
