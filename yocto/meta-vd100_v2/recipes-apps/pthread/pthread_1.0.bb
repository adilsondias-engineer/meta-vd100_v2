DESCRIPTION = "Pthread application"
LICENSE = "GPLv2"
SRC_URI = "file://Makefile \
           file://pthread.c \
           file://COPYING \
          "
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

S =  "${WORKDIR}"

DEPENDS = ""

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
