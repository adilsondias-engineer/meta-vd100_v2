do_install:append() {
    sed -i 's/PASS_MAX_DAYS.*/PASS_MAX_DAYS   99999/' ${D}${sysconfdir}/login.defs
}