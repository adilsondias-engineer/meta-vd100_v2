FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
SRC_URI += "file://bsp.cfg \
            "
KERNEL_FEATURES:append = " bsp.cfg"

