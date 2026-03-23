# Versal AI Edge VD100 — AXI-lite PL LED + PL Reset IRQ + PS_KEY Kernel Driver

Custom Yocto Linux on the Alinx VD100 (XCVE2302-SFVA784-1LP-E-S) — custom AXI-lite IP in Vivado, Linux kernel driver, PL fabric reset detection via hardware interrupt, and interactive LED control from both PS_KEY and userspace.

> No VCK190. No MATLAB. No Simulink. Bare-metal VHDL, a UART, and a debugger's mindset.

---

## The Platform

```
Vivado 2025.2
    └─ CIPS (M_AXI_FPD + pl_ps_irq[0] enabled)
       + MyLEDIP AXI-lite IP
       + Inline Utility Vector Logic (PL_KEY1 invert → pl_ps_irq)
       → XSA export
        └─ lopper / sdtgen → System Device Tree (vd100_sdt)
            └─ Yocto Scarthgap + meta-vd100_v2
                └─ VD100 Linux 1.0
                    ├─ PLM → U-Boot → systemd-boot → Linux
                    ├─ myledip.ko → /dev/myledip (AXI-lite @ 0xA4000000)
                    │   ├─ pl_ps_irq[0] → PL reset detection (GIC SPI 84)
                    │   ├─ PS_KEY IRQ → LED toggle (LPD MIO24)
                    │   ├─ PS_LED mirror (LPD MIO25)
                    │   └─ sysfs pl_led_state
                    ├─ helloworld → PS LED (libgpiod) + PL LED (/dev/myledip)
                    └─ Ethernet, USB, I2C, GPIO, Sysmon
```

---

## Repository Structure

| Path | Contents |
|---|---|
| `ledblink/hardware/` | Vivado 2025.2 project — CIPS + MyLEDIP + Inline Utility Vector Logic block design, XSA export |
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
| Ethernet GEM0 | Y |
| USB XHCI (host mode) | Y |
| I2C2 — LM75 board temperature sensor (`i2c-2`, `f1000000`) | Y |
| I2C2 — EEPROM Atmel 24C04 | Y |
| Versal sysmon (die temperature + voltages) | Y |
| PS_LED1 — LPD MIO25 via kernel driver | Y |
| PS_KEY — LPD MIO24 IRQ → LED toggle via kernel driver | Y |
| PL_LED1 — AXI-lite kernel module (`/dev/myledip`) | Y |
| PL reset detection — pl_ps_irq[0] GIC SPI 84 | Y |
| PL LED auto-restore after reset | Y |
| PS + PL LED C++ demo (`helloworld`) | Y |

---

## PL Reset Detection

PL_KEY1 (F21, active-low) is wired to two independent destinations in the block design:

```
PL_KEY1 (F21, active-low)
    ├── aux_reset_in → Proc_Sys_Reset → s00_axi_aresetn   (hard PL reset)
    └── Inline Utility Vector Logic (NOT gate)
            └── CIPS pl_ps_irq[0] → GIC SPI 84           (PS interrupt)
```

The interrupt path is completely independent of AXI — it fires on the PS even while
the PL fabric is in reset. No AXI register access occurs in the ISR.

```
$ journalctl -k | grep myledip
myledip: PL is restarting — LED was ON
myledip: PL restart complete — LED restored ON
myledip: PL is restarting — LED was OFF     ← previously silent, now always visible
myledip: PL restart complete — LED restored ON
```

---

## LED State — Source of Truth Hierarchy

```
AXI slv_reg0 (hardware)                 ← absolute truth, read on probe
    └─ driver shadow register           ← always coherent with hardware
        └─ /sys/.../pl_led_state        ← reconstructed on probe
            └─ /var/run/pl_led.state    ← reconstructed on boot via udev
```

On PS reboot, `myledip.ko` reads the hardware register directly — no dependency
on any state file. Hardware is always the authority.

**PL LED semantic:** ON = PL fabric running. OFF = PL in reset.

---

## Device Access — Non-root

Devices are accessible to the `pldev` group without `sudo`:

```bash
# /dev/myledip — PL LED control
echo 1 | tee /dev/myledip    # ON
echo 0 | tee /dev/myledip    # OFF
cat /dev/myledip              # read state

# sysfs
cat /sys/class/myledip/myledip0/pl_led_state
echo 1 > /sys/class/myledip/myledip0/pl_led_state
```

Group membership is set in `vd100.conf` via `EXTRA_USERS_PARAMS` — single location
for all user/group management. udev rule `99-myledip.rules` sets `GROUP=pldev MODE=0660`.

---

## Key DTS Patches

All patches applied via `meta-vd100_v2/apply_vd100_patches.sh`:

| Patch | Reason |
|---|---|
| `sdhci1` — SD 2.0, no-1-8-v, max-frequency=25MHz | TXS02612 level shifter compatibility |
| `dwc3_0` — dr_mode=host | USB host mode |
| `i2c0`, `i2c1` disabled | Frees f1000000 as i2c-0 for LM75 + EEPROM |
| `i2c2` — LM75 + EEPROM nodes | PMC_I2C MIO34/35 |
| `MyLEDIP@a4000000` — clocks=66, interrupts=<0 84 4> | PL peripheral + pl_ps_irq |
| `MyLEDIP` — ps-key-gpios, ps-led-gpios | PS_KEY MIO24, PS_LED MIO25 |
| `gem0` — xlnx,has-mdio=<0x1> | SDT export regression — MDIO enable |

> `xlnx,has-mdio` is required — Vivado SDT export drops it on regeneration.
> Without it, U-Boot reports `Could not get PHY for eth0: addr -19` and Ethernet fails.

---

## Documentation

- **[ledblink/hardware/README.md](ledblink/hardware/README.md)** — Block design topology, M_AXI_FPD path, Inline Utility Vector Logic wiring, MyLEDIP register map, pin constraints, VHDL state machine
- **[ledblink/software/vd100_sdt/README.md](ledblink/software/vd100_sdt/README.md)** — System Device Tree generation from XSA
- **[yocto/build/README.md](yocto/build/README.md)** — Build environment setup, `local.conf`, `bblayers.conf`
- **[yocto/meta-vd100_v2/README.md](yocto/meta-vd100_v2/README.md)** — Complete layer reference: DTS patches, myledip driver, PS_KEY/PL_KEY1 interrupt design, permissions, SDK workflow

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