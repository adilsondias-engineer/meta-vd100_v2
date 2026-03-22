# Versal AI Edge VD100 — AXI-lite PL LED Kernel Driver

Custom Yocto Linux on the Alinx VD100 (XCVE2302-SFVA784-1LP-E-S) — custom AXI-lite IP in Vivado, Linux kernel driver, and C++ userspace demo controlling both the PS and PL LEDs from a single application.

> No VCK190. No MATLAB. No Simulink. Bare-metal VHDL, a UART, and a debugger's mindset.

---

## The Platform

```
Vivado 2025.2
    └─ CIPS (M_AXI_FPD enabled) + MyLEDIP AXI-lite IP → XSA export
        └─ lopper / sdtgen → System Device Tree (vd100_sdt)
            └─ Yocto Scarthgap + meta-vd100_v2
                └─ VD100 Linux 1.0
                    ├─ PLM → U-Boot → systemd-boot → Linux
                    ├─ myledip.ko → /dev/myledip (AXI-lite @ 0xA4000000)
                    ├─ helloworld → PS LED (libgpiod) + PL LED (/dev/myledip)
                    └─ Ethernet, USB, I2C, GPIO, Sysmon
```

---

## Repository Structure

| Path | Contents |
|---|---|
| `ledblink/hardware/` | Vivado 2025.2 project — CIPS + MyLEDIP block design, XSA export |
| `ledblink/ip_repo/MyLEDIP_1_0/` | Custom AXI-lite slave IP — 2-register LED control interface |
| `ledblink/software/vd100_sdt/` | System Device Tree generated from XSA via lopper/sdtgen |
| `yocto/meta-vd100_v2/` | Custom Yocto layer — distro, BSP, DTS patches, recipes |
| `yocto/build/` | Build environment — `local.conf`, `bblayers.conf` |
| `images/` | Boot screenshots and hardware photos |

---

## Board

| Item | Detail |
|---|---|
| **Device** | XCVE2302-SFVA784-1LP-E-S |
| **Family** | Versal AI Edge Series |
| **PS** | Dual-core Cortex-A72 @ 959 MHz |
| **DDR** | 2 GiB |
| **eMMC** | 29.6 GiB |
| **Cost** | ~AU$1,285 — accessible alternative to the AU$15,000+ VCK190 |

---

## Confirmed Working

| Feature | Status |
|---|---|
| SD card (via TXS02612 level shifter) | Y |
| Ethernet GEM | Y |
| USB XHCI (host mode) | Y |
| I2C2 — LM75 board temperature sensor (`i2c-2`, `f1000000`) | Y |
| I2C2 — EEPROM Atmel 24C04 | Y |
| Versal sysmon (die temperature + voltages) | Y |
| PS_LED1 — GPIO LPD_MIO25 via libgpiod v2 | Y |
| PL LED — AXI-lite kernel module (`/dev/myledip`) | Y |
| PS + PL LED C++ demo (`helloworld`) | Y |

---

## Documentation

- **[ledblink/hardware/README.md](ledblink/hardware/README.md)** — Block design, M_AXI_FPD path, MyLEDIP register map, pin constraints, VHDL state machine fixes
- **[ledblink/software/vd100_sdt/README.md](ledblink/software/vd100_sdt/README.md)** — System Device Tree generation from XSA
- **[yocto/build/README.md](yocto/build/README.md)** — Build environment setup, `local.conf`, `bblayers.conf`
- **[yocto/meta-vd100_v2/README.md](yocto/meta-vd100_v2/README.md)** — Complete layer reference: DTS patches, myledip kernel module, helloworld C++ API, SDK workflow

Start with the hardware README and follow the chain.

---

## Development Environment

- **Vivado / Vitis:** 2025.2 (ML Enterprise License)
- **Yocto:** Scarthgap (5.0), meta-xilinx `rel-v2025.2`
- **Build host:** Ubuntu 22.04
- **Target kernel:** `6.12.40-xilinx`

---

## Related

This project builds on [versal-ai-edge-vd100-linux](https://github.com/adilsondias-engineer/versal-ai-edge-vd100-linux) — the base PS bring-up. 

---

*XCVE2302 — Versal AI Edge — VD100 Linux 1.0 — AXI-lite — Vivado/Vitis 2025.2*
