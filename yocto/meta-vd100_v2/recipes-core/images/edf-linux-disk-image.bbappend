# Stage BOOTAA64.EFI from rootfs into deploy dir before WIC builds the image
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# Ensure SRC_URI files are fetched before WIC runs
#do_image_wic[depends] += "virtual/kernel:do_deploy"

python stage_efi_files() {
    import shutil, os

    rootfs = d.getVar('IMAGE_ROOTFS')
    deploy = d.getVar('DEPLOY_DIR_IMAGE')

    # Stage BOOTAA64.EFI
    src = os.path.join(rootfs, 'usr/lib/systemd/boot/efi/systemd-bootaa64.efi')
    dst_dir = os.path.join(deploy, 'EFI/BOOT')
    os.makedirs(dst_dir, exist_ok=True)
    shutil.copy2(src, os.path.join(dst_dir, 'BOOTAA64.EFI'))
    bb.note("Staged BOOTAA64.EFI")

    # Stage loader.conf
    loader_dir = os.path.join(deploy, 'loader')
    entries_dir = os.path.join(deploy, 'loader/entries')
    os.makedirs(entries_dir, exist_ok=True)

    with open(os.path.join(loader_dir, 'loader.conf'), 'w') as f:
        f.write('default edf-linux.conf\ntimeout 5\n')

    with open(os.path.join(entries_dir, 'edf-linux.conf'), 'w') as f:
        f.write('title   VD100 Linux\n')
        f.write('efi     /Image\n')
        f.write('devicetree /system.dtb\n')
        f.write('options console=ttyPS0,115200 console=ttyAMA0 earlycon root=/dev/mmcblk1p3 rw rootwait uio_pdrv_genirq.of_id=generic-uio\n')

    bb.note("Staged loader.conf and edf-linux.conf")
}

do_image_wic[prefuncs] += "stage_efi_files"

IMAGE_EFI_BOOT_FILES:append = " \
    EFI/BOOT/BOOTAA64.EFI;EFI/BOOT/BOOTAA64.EFI \
    loader/loader.conf;loader/loader.conf \
    loader/entries/edf-linux.conf;loader/entries/edf-linux.conf \
    boot.bin;boot.bin \
"
