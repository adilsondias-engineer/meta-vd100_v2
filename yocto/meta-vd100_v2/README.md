# meta-vd100_v2 — Custom Yocto Layer for the Alinx VD100

Custom Yocto layer for the Alinx VD100 (XCVE2302-SFVA784-1LP-E-S) board.  
Extends the AMD Embedded Development Framework (`meta-amd-edf`) with board-specific patches, a custom distro, applications, and kernel modules.

This is the v2 layer — it adds I2C, LM75, EEPROM, sysmon, GPIO, libgpiod userspace app (`helloworld`), and the AXI-lite PL LED kernel module (`myledip`) on top of the v1 base.

---

## Layer Structure

```
meta-vd100_v2/
├── conf/
│   ├── distro/
│   │   └── vd100.conf                           ← Custom distro definition
│   ├── machine/
│   │   └── versal-generic-xcve2302.conf          ← Machine config (SDT path, DTB)
│   └── layer.conf                               ← Layer registration
├── recipes-apps/
│   ├── helloworld/                              ← PS + PL LED C++ demo app
│   ├── addstaticfile/                           ← Static file deployment
│   └── mdiorw/                                  ← MDIO register read/write tool
├── recipes-bsp/                                 ← U-Boot / firmware patches
├── recipes-core/
│   ├── base-files/                              ← Hostname, /etc/issue
│   ├── images/
│   │   └── vd100-image.bb                       ← IMAGE_INSTALL
│   ├── shadow/                                  ← Password policy
│   └── systemd/
│       ├── systemd_%.bbappend                   ← Enable timesyncd (NTP)
│       └── systemd-bootconf-edf_1.00.bbappend   ← Fix EFI entry path
├── recipes-devtools/                            ← Development tool recipes
├── recipes-kernel/
│   └── linux-xlnx_%.bbappend                   ← Kernel config fragments
└── recipes-modules/
    └── myledip/
        ├── files/
        │   ├── myledip.c                        ← AXI-lite PL LED kernel module
        │   └── Makefile
        └── myledip.bb                           ← Kernel module recipe
```

---

## conf/distro/vd100.conf

```bitbake
require conf/distro/amd-edf.conf   # NOT poky.conf — preserves full AMD EDF stack

DISTRO          = "vd100"
DISTRO_NAME     = "VD100 Linux"
DISTRO_VERSION  = "1.0"
DISTRO_CODENAME = "versal"

# RPM required — 43 Xilinx recipes produce invalid deb package names
PACKAGE_CLASSES = "package_rpm"

# Must use = not ?= — amd-edf.conf sets ?= first, subsequent ?= is silently ignored
EXTRA_USERS_PARAMS = "\
    useradd -m -G sudo adilson; \
    usermod -p '\$6\$...' adilson; \
"
inherit extrausers

TARGET_VENDOR = "-vd100"   # Cross-compiler tuple: aarch64-vd100-linux
```

> **Rule:** Always `require amd-edf.conf`, never `poky.conf`. Replacing it strips the Versal PLM, TF-A, and PDI handling from the image.

---

## conf/machine/versal-generic-xcve2302.conf

```bitbake
require conf/machine/versal-generic-xcve2302.conf

# Point to Vivado-exported SDT for the ledblink project
SDT_URI = "file:///work/ledblink/software/vd100_sdt"

# Linux domain DTS — board patches applied here directly
CONFIG_DTFILE ?= "${CONFIG_DTFILE_DIR}/cortexa72-linux.dts"
```

---

## DTS Patches — cortexa72-linux.dts

All board-specific patches applied directly to `cortexa72-linux.dts`. No `system-user.dtsi` — patches are co-located with the SDT source for visibility.

### sdhci1 — TXS02612 level shifter

```dts
&sdhci1 {
    no-1-8-v;
    disable-wp;
    max-frequency = <25000000>;   /* TXS02612 max ~30 MHz */
};
```

### USB host mode

```dts
&dwc3_0 {
    status = "okay";
    dr_mode = "host";
};
```

### I2C2 — LM75 + EEPROM

```dts
/* i2c2 label = f1000000 (PMC I2C) = Linux i2c-2
   LM75 and EEPROM are on this bus — confirmed by i2cdetect -y 2 */
&i2c2 {
    status = "okay";

    lm75@48 {
        compatible = "nxp,lm75";
        reg = <0x48>;
    };

    eeprom@50 {
        compatible = "atmel,24c04";
        reg = <0x50>;
        pagesize = <16>;
    };
};
```

I2C0 (`ff020000`) and I2C1 (`ff030000`) enabled with `status = "okay"`. No child devices.

### MyLEDIP PL peripheral

```dts
/* Use &axi — NOT &amba. amba is an alias for /axi but &amba
   causes subtly incorrect node placement in some DTS parsers.
   Node address MUST be lowercase hex — uppercase causes a U-Boot
   address parse loop that prevents boot. */
&axi {
    MyLEDIP_0: MyLEDIP@a4000000 {
        compatible = "xlnx,MyLEDIP-1.0";
        reg = <0x0 0xA4000000 0x0 0x10000>;
        clocks = <&versal_clk 65>;
        clock-names = "s00_axi_aclk";
    };
};
```

> **`&axi` not `&amba`** — both compile but `&axi` is the correct Versal node alias.  
> **Lowercase node address** — `@a4000000` not `@A4000000`. Uppercase causes a U-Boot address parsing loop that hangs boot before reaching the kernel.  
> **Remove `&fpga firmware-name`** if the PDI is not deployed to `/lib/firmware` — leaving it present with a missing file blocks AXI peripheral initialisation.

### Verify after rebuild

```bash
dtc -I dtb -O dts cortexa72-linux.dtb 2>/dev/null | \
  grep -E "no-1-8-v|disable-wp|max-frequency|dwc3|lm75|24c04|MyLEDIP"
```

All seven tokens must be present.

---

## recipes-core/images/vd100-image.bb

```bitbake
require recipes-core/images/edf-linux-disk-image.bb

IMAGE_INSTALL:append = " \
    devmem2             \
    os-release          \
    helloworld          \
    myledip             \
    libgpiod            \
    libgpiod-tools      \
    libgpiod-dev        \
    libgpiodcxx         \
"
```

| Package | Recipe | Function |
|---|---|---|
| `helloworld` | `recipes-apps/helloworld` | PS + PL LED C++ demo (libgpiod + /dev/myledip) |
| `myledip` | `recipes-modules/myledip` | AXI-lite PL LED kernel module |
| `libgpiod` / `libgpiodcxx` | upstream | userspace GPIO library + C++ bindings |
| `devmem2` | upstream | direct register access for debug |

---

## recipes-modules/myledip

PL LED kernel module. Registers as a Linux platform driver, binds to the `MyLEDIP` AXI-lite slave via the device tree compatible string, and exposes a character device at `/dev/myledip`.

### How it works

```
Linux platform bus
    └─ myledip platform_driver
        └─ probe() — of_match: "xlnx,MyLEDIP-1.0"
            └─ platform_get_resource() → 0xA4000000
                └─ ioremap(0xA4000000, 0x10000)
                    └─ register_chrdev() → /dev/myledip
                        ├─ write('1') → iowrite32(1, base + 0x00) → LED on
                        ├─ write('0') → iowrite32(0, base + 0x00) → LED off
                        └─ read()    → ioread32(base + 0x04)      → status
```

### Recipe (myledip.bb)

```bitbake
SUMMARY = "VD100 PL LED AXI-lite kernel module"
LICENSE  = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0-only;md5=..."

inherit module

SRC_URI = "file://myledip.c \
           file://Makefile"

S = "${WORKDIR}"

RPROVIDES:${PN} += "kernel-module-myledip"
```

`inherit module` handles `KERNEL_SRC`, `depmod`, and cross-compile setup automatically.

> **Kernel 6.x:** `myledip_remove()` must be declared `static void` — not `static int`. Returning `int` from a platform driver `remove` callback is a compile error in kernel 6.x.

### Runtime usage

```bash
# PL LED on
echo 1 | sudo tee /dev/myledip

# PL LED off
echo 0 | sudo tee /dev/myledip

# Read LED state (status register — offset 0x04)
sudo cat /dev/myledip

# Load / unload
sudo modprobe myledip
sudo modprobe -r myledip

# Confirm probe
dmesg | grep myledip
# myledip: probed at 0xa4000000 → /dev/myledip
```

### Deploy over SSH

```bash
bitbake myledip
scp tmp/deploy/rpm/cortexa72/myledip-*.rpm adilson@192.168.0.x:~
ssh adilson@192.168.0.x "sudo rpm -i myledip-*.rpm && sudo modprobe myledip"
```

---

## recipes-apps/helloworld — PS + PL LED Demo

C++ application that controls both LEDs from a single program:

| LED | Path | Method |
|---|---|---|
| PS_LED1 | LPD_MIO25 → gpiochip0 line 25 | libgpiod v2 `do_request()` |
| PL LED | `/dev/myledip` | `open()` + `write("1"/"0")` + `read()` |

### libgpiod v2 rules

- `--chip` flag is mandatory (breaking change from v1)
- `do_request()` takes exclusive ownership of the line — never create a second request to read an output line
- For stateless processes (run-exit), persist driven state to `/var/run/`

### C++ API pattern

```cpp
// PS LED — libgpiod v2
auto chip = gpiod::chip("/dev/gpiochip0");
auto line = chip.get_line(25);
gpiod::line_request req;
req.consumer  = "helloworld";
req.request_type = gpiod::line_request::DIRECTION_OUTPUT;
line.request(req, 1);    // drive high (LED on)
line.set_value(0);       // drive low (LED off)

// PL LED — /dev/myledip
int fd = open("/dev/myledip", O_RDWR);
write(fd, "1", 1);       // LED on
write(fd, "0", 1);       // LED off
char buf[2] = {};
read(fd, buf, 1);        // read status register
close(fd);
```

### Cross-compile with SDK

```bash
source /opt/vd100/sdk/<version>/environment-setup-cortexa72-cortexa53-vd100-linux
$CXX $CFLAGS helloworld.cpp -o helloworld $LDFLAGS -lgpiodcxx -lgpiod
file helloworld   # ELF 64-bit LSB executable, ARM aarch64
```

---

## Yocto Upstream Bug — systemd-bootconf-edf

`meta-amd-edf` deploys `edf-linux.conf` to `loader/` instead of `loader/entries/`, causing systemd-boot to silently ignore the boot entry. Fixed via bbappend:

```bitbake
# recipes-core/systemd/systemd-bootconf-edf_1.00.bbappend
do_install:append() {
    install -d ${D}${sysconfdir}/loader/entries
    if [ -f ${D}${sysconfdir}/loader/edf-linux.conf ]; then
        mv ${D}${sysconfdir}/loader/edf-linux.conf \
           ${D}${sysconfdir}/loader/entries/edf-linux.conf
    fi
}
```

---

## NTP — systemd_%.bbappend

The VD100 has no RTC battery — time resets on every poweroff without NTP. NTP syncs time on every boot from network before user session starts:

```bitbake
PACKAGECONFIG:append = " timesyncd"
```

---

## EFI Staging — Python Prefuncs

`do_image_wic:prepend` shell functions are **skipped when sstate cache is active**. All EFI file staging uses python prefuncs — guaranteed to execute regardless of cache state:

```bitbake
do_image_wic[prefuncs] += "stage_efi_files"

python stage_efi_files() {
    import shutil, os
    rootfs    = d.getVar("IMAGE_ROOTFS")
    deploydir = d.getVar("DEPLOY_DIR_IMAGE")

    # BOOTAA64.EFI sourced from Yocto rootfs systemd-boot build
    efi_src = os.path.join(rootfs,
        "usr/lib/systemd/boot/efi/systemd-bootaa64.efi")
    efi_dst = os.path.join(deploydir, "EFI/BOOT/BOOTAA64.EFI")
    os.makedirs(os.path.dirname(efi_dst), exist_ok=True)
    shutil.copy2(efi_src, efi_dst)
    # loader.conf and edf-linux.conf staged similarly
}
```

---

## After Vivado Block Design Changes

Any CIPS or PL IP change in Vivado requires an SDT regeneration and Yocto rebuild:

```bash
# 1. Re-export XSA from Vivado (ledblink.xsa)
# 2. Regenerate SDT
sdtgen set_hw_design ledblink.xsa
sdtgen generate -dir vd100_sdt

# 3. Regenerate machine config from new SDT
gen-machineconf parse-sdt \
    --hw-description /work/ledblink/software/vd100_sdt \
    --soc-family versal \
    --output ../sources/meta-vd100_v2/conf/machine \
    -c conf -l conf/local.conf \
    --machine-name versal-generic-xcve2302 \
    --gen-pl-overlay full

# 4. Re-apply board patches
./apply_vd100_patches.sh

# 5. Rebuild boot artifacts
bitbake -c cleansstate sdt-artifacts
bitbake xilinx-bootbin
```

### Boot partition update only

After a DTB or boot.bin change, only the EFI partition needs updating — no full reflash:

```bash
sudo mount /dev/mmcblk1p1 /mnt
sudo cp boot.bin system.dtb /mnt/
sudo umount /mnt
```

---

## Build the Layer

See the build environment setup:  
**[../build/README.md](../build/README.md)**

---

*meta-vd100_v2 — Yocto Scarthgap — AMD EDF 25.11 — Vivado 2025.2*
