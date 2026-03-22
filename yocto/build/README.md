# yocto/build — Build Environment

Yocto build environment for VD100 Linux 1.0. Contains the build configuration (`local.conf`, `bblayers.conf`) and workspace.

---

## Prerequisites

### Host System

- Ubuntu 22.04 LTS (recommended)
- 100+ GB free disk space for build artifacts
- 16+ GB RAM recommended

### Required packages

```bash
sudo apt-get install -y \
    gawk wget git diffstat unzip texinfo gcc build-essential \
    chrpath socat cpio python3 python3-pip python3-pexpect \
    xz-utils debianutils iputils-ping python3-git python3-jinja2 \
    libegl1-mesa libsdl1.2-dev pylint xterm python3-subunit \
    mesa-common-dev zstd liblz4-tool file locales libacl1
```

---

## Repository Setup

Clone the required layers alongside this repository:

```bash
mkdir -p /work/yocto/sources
cd /work/yocto/sources

# Poky (Yocto base)
git clone https://git.yoctoproject.org/poky -b scarthgap

# meta-xilinx (AMD/Xilinx BSP layers)
git clone https://github.com/Xilinx/meta-xilinx -b rel-v2025.2

# meta-amd-edf (AMD Embedded Development Framework)
git clone https://github.com/Xilinx/meta-amd-edf -b rel-v2025.2

# meta-openembedded
git clone https://github.com/openembedded/meta-openembedded -b scarthgap

# This repository's meta-vd100_v2 layer is at:
# /work/yocto/sources/meta-vd100_v2  (symlink or copy from repo)
```

---

## Build Environment Initialisation

```bash
cd /work/yocto
source sources/poky/oe-init-build-env build
```

---

## bblayers.conf

```bitbake
BBLAYERS ?= " \
    /work/yocto/sources/poky/meta \
    /work/yocto/sources/poky/meta-poky \
    /work/yocto/sources/meta-openembedded/meta-oe \
    /work/yocto/sources/meta-openembedded/meta-python \
    /work/yocto/sources/meta-openembedded/meta-networking \
    /work/yocto/sources/meta-openembedded/meta-filesystems \
    /work/yocto/sources/meta-xilinx/meta-xilinx-core \
    /work/yocto/sources/meta-xilinx/meta-xilinx-standalone \
    /work/yocto/sources/meta-xilinx/meta-xilinx-bsp \
    /work/yocto/sources/meta-amd-edf \
    /work/yocto/sources/meta-vd100_v2 \
"
```

---

## local.conf — Key Settings

```bitbake
# Target machine
MACHINE = "versal-generic-xcve2302"

# Custom VD100 distro
DISTRO = "vd100"

# Package format — RPM required (deb has 43 broken Xilinx package names)
PACKAGE_CLASSES = "package_rpm"

# Parallel build settings — adjust to host CPU count
BB_NUMBER_THREADS = "16"
PARALLEL_MAKE = "-j 16"

# Download and sstate cache directories
DL_DIR = "/work/yocto/downloads"
SSTATE_DIR = "/work/yocto/sstate-cache"

# Accept AMD/Xilinx EULA
LICENSE_FLAGS_ACCEPTED = "xilinx"
```

---

## Build the Image

```bash
# Full image build
bitbake edf-linux-disk-image

# Build SDK for cross-compilation
bitbake -c populate_sdk edf-linux-disk-image
```

Build time: 2–4 hours on first build (downloads + compiles everything). Subsequent builds are incremental.

---

## Build Outputs

```
tmp/deploy/images/versal-generic-xcve2302/
├── edf-linux-disk-image-*.wic          ← SD card image (flash with dd)
├── boot.bin                            ← PLM + TF-A + U-Boot
├── Image                               ← Linux kernel
├── cortexa72-linux.dtb                 ← Linux domain DTB
└── system.dtb → cortexa72-linux.dtb    ← symlink used by boot config
```

---

## Flash SD Card

```bash
# Find your SD card device
lsblk

# Flash (replace sdX with actual device — DESTRUCTIVE)
sudo dd if=tmp/deploy/images/versal-generic-xcve2302/edf-linux-disk-image-*.wic \
    of=/dev/sdX bs=4M status=progress conv=fsync

# Verify 3 partitions were created
sudo fdisk -l /dev/sdX
```

Expected partition layout:
- `/dev/sdX1` — FAT32 1GB — EFI boot partition
- `/dev/sdX2` — FAT32 1GB — storage (empty)
- `/dev/sdX3` — ext4 6GB — root filesystem

---

## Update Boot Partition Only (DTB/config changes)

After DTB-only changes, only the EFI partition needs updating — no full reflash required:

```bash
# Mount EFI partition
sudo mount /dev/sdX1 /mnt

# Copy updated DTB
sudo cp tmp/deploy/images/versal-generic-xcve2302/cortexa72-linux.dtb \
    /mnt/devicetree/

# system.dtb symlink should already point correctly
ls -la /mnt/system.dtb

sudo umount /mnt
```

---

## Install SDK

```bash
# Install SDK to /opt/vd100/
./tmp/deploy/sdk/vd100-glibc-x86_64-edf-linux-disk-image-cortexa72-cortexa53-versal-generic-xcve2302-toolchain-*.sh

# Source SDK environment for cross-compilation
source /opt/vd100/sdk/<version>/environment-setup-cortexa72-cortexa53-vd100-linux
```

---

## Common Bitbake Commands

```bash
# Clean and rebuild specific recipe
bitbake -c cleansstate <recipe>
bitbake <recipe>

# Clean DTB chain after DTS changes
bitbake -c cleansstate sdt-artifacts device-tree virtual/dtb
bitbake edf-linux-disk-image

# Show recipe variables (debug)
bitbake -e <recipe> | grep "^VARIABLE_NAME"

# List available tasks
bitbake -c listtasks <recipe>

# Force rebuild without sstate
bitbake --no-setscene edf-linux-disk-image
```

---

## Troubleshooting

### do_image_wic fails — EFI files missing

Shell `do_image_wic:prepend` is skipped by sstate cache. The `meta-vd100_v2` layer uses `python prefuncs` (`do_image_wic[prefuncs]`) to stage EFI files — this is guaranteed to run. If files are still missing, run:

```bash
bitbake -c cleansstate edf-linux-disk-image
bitbake edf-linux-disk-image
```

### systemd-bootconf-edf fails — wrong deploy path

Upstream `meta-amd-edf` bug: `edf-linux.conf` deployed to `loader/` instead of `loader/entries/`. Fixed in `meta-vd100_v2/recipes-core/systemd/systemd-bootconf-edf_1.00.bbappend`.

### EXTRA_USERS_PARAMS not applied

Use `=` not `?=` in `vd100.conf`. The `amd-edf.conf` sets `EXTRA_USERS_PARAMS` with `?=` first — a subsequent `?=` in `vd100.conf` is silently ignored.

---

## Next Step

See the complete Yocto layer reference:
**[../meta-vd100_v2/README.md](../meta-vd100_v2/README.md)**
