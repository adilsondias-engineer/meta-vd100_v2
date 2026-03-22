DESCRIPTION = "Hello World application"
LICENSE = "CLOSED"
SRC_URI = "file://helloworld"

S =  "${WORKDIR}/helloworld"

DEPENDS = "libgpiod"

TARGET_CC_ARCH += "${LDFLAGS}"

do_compile() {	
 oe_runmake build	 
}

do_install () {
 oe_runmake install DESTDIR=${D} BINDIR=${bindir}	
}

do_clean () {
	oe_runmake clean
}

do_cleanall () {
	oe_runmake clean
}

