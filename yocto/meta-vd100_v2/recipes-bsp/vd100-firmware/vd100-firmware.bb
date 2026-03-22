# meta-vd100_v2/recipes-bsp/vd100-firmware/vd100-firmware.bb
SUMMARY = "VD100 PL LED firmware (PDI)"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

DEPENDS = "sdt-artifacts"

do_install() {
    install -d ${D}/usr/lib/firmware/xilinx
    install -m 0644 ${RECIPE_SYSROOT}${datadir}/sdt/${MACHINE}/led.pdi \
        ${D}/usr/lib/firmware/xilinx/led.pdi
    install -m 0644 ${RECIPE_SYSROOT}${datadir}/sdt/${MACHINE}/led.pdi \
        ${D}/usr/lib/firmware/led.pdi
}

FILES:${PN} = "/usr/lib/firmware/led.pdi \
               /usr/lib/firmware/xilinx/led.pdi \
	      "
