FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append = " file://platform-top.h file://bsp.cfg"

do_configure:prepend() {
    install -m 0644 ${WORKDIR}/platform-top.h \
        ${S}/include/configs/platform-top.h
}
