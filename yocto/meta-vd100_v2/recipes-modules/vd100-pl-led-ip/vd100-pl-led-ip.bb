SUMMARY = "VD100 PL LED AXI-lite kernel driver"
SECTION = "VD100/modules"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

INHIBIT_PACKAGE_STRIP = "1"

SRC_URI = "file://Makefile \
           file://pl_led_ip.c \
           file://COPYING \
          "

S = "${WORKDIR}"

KERNEL_MODULE_AUTOLOAD += "pl_led_ip"
