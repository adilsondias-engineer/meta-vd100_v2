# vd100_sdt — System Device Tree

System Device Tree (SDT) generated from the Vivado XSA export using `lopper` and `sdtgen`. The SDT is the hardware description that Yocto uses to configure the Linux kernel, U-Boot, and firmware components for the VD100.

---

## What Is the System Device Tree

The traditional Linux device tree describes hardware for a single operating system. The Versal System Device Tree extends this — it describes the full hardware platform including firmware domains (PLM, PSM, TF-A) alongside the Linux domain. Yocto's SDT-based BSP flow uses this to generate all hardware-specific artifacts automatically.

---

## Prerequisites

- Vivado 2025.2 (for XSA export)
- `sdtgen` — System Device Tree generator

---

## Generate the SDT
```tcl
"VITIS_INSTALL_FOLDER\2025.2\Vitis\bin\sdtgen.bat"
sdtgen% sdtgen set_dt_param -dir ./vd100_sdt
sdtgen% sdtgen set_dt_param -xsa design_1_wrapper.xsa
sdtgen% sdtgen generate_sdt
sdtgen% sdtgen copy_hw_files
sdtgen% 
```

## Output Structure

```
vd100_sdt/
├── system-top.dts          ← top-level DTS — includes all domains
├── cortexa72-linux.dts     ← Linux domain DTS (used by Yocto CONFIG_DTFILE)
├── pcw.dtsi                ← CIPS hardware configuration from Vivado
├── system-user.dtsi        ← user patches applied on top of generated DTS
├── design_1_wrapper.pdi    ← Versal PDI (PL bitstream + PS configuration)
└── ...
```

---

## Key Files

### cortexa72-linux.dts

The Linux domain device tree — 2965 lines describing the full Cortex-A72 hardware view. This is the file Yocto builds `cortexa72-linux.dtb` from via `CONFIG_DTFILE`.

This file is **complete** — it contains full sdhci, USB, I2C, GPIO, Ethernet, and DWC3 nodes. It does not need `system-top.dts` or `pcw.dtsi` to build a working Linux DTB.

### design_1_wrapper.pdi

The Versal PDI containing the PL bitstream and PS initialisation data. Referenced in the Yocto machine config as `PDI_PATH`.

### system-user.dtsi

User patch file applied on top of the generated DTS. Board-specific patches that the Vivado CIPS export does not generate automatically belong here. For the VD100, patches are applied directly in `cortexa72-linux.dts` in `meta-vd100_v2` instead — see the Yocto README.

---

## Integration with Yocto

The `vd100_sdt` directory is referenced in `meta-vd100_v2/conf/machine/versal-generic-xcve2302.conf`:

```bitbake
SDT_URI = "file:///work/ledblink/software/vd100_sdt"
```

Yocto's `sdt-artifacts` recipe fetches this directory and makes the PDI and DTS files available to downstream recipes.

---

## Regenerating After Vivado Changes

Any change to the CIPS configuration in Vivado requires re-exporting the XSA and regenerating the SDT:

```bash
# 1. Re-export XSA from Vivado
# 2. Regenerate SDT
sdtgen generate -dir vd100_sdt
gen-machineconf parse-sdt --hw-description  /work/ledblink/software/vd100_sdt --soc-family versal  --output ../sources/meta-vd100_v2/conf/machine -c conf -l conf/local.conf --machine-name versal-generic-xcve2302 --gen-pl-overlay full
# 3. Force Yocto to pick up the new SDT
bitbake -c cleansstate sdt-artifacts device-tree virtual/dtb
bitbake edf-linux-disk-image
```

---

## Next Step

Proceed to Yocto build:
**[../../yocto/build/README.md](../../yocto/build/README.md)**
