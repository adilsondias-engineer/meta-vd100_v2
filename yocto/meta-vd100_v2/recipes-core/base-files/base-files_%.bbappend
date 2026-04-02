do_install:append() {
    echo "VD100 Linux 1.0" > ${D}${sysconfdir}/issue
    echo "VD100 Linux 1.0 \n \l" > ${D}${sysconfdir}/issue.net
    cat > ${D}${sysconfdir}/hosts << 'EOF'
127.0.0.1       localhost
127.0.1.1       vd100
::1             localhost ip6-localhost ip6-loopback
fe00::0         ip6-localnet
ff00::0         ip6-mcastprefix
ff02::1         ip6-allnodes
ff02::2         ip6-allrouters
EOF
cat > ${D}${sysconfdir}/hostname << 'EOF'
vd100
EOF
cat > ${D}${sysconfdir}/motd << 'EOF'
Welcome to VD100 Linux 1.0!!
By Adilson Dias
EOF
}
