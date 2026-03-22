# ledblink — Vivado Hardware Project

Vivado 2025.2 project for the Alinx VD100 (XCVE2302-SFVA784-1LP-E-S). Configures the Versal CIPS block with M_AXI_FPD enabled, implements the `MyLEDIP` AXI-lite slave custom IP, and exports the XSA for System Device Tree generation. The `MyLEDIP` peripheral is controlled from Linux via the `myledip` kernel module.

---

## Project Structure

```
ledblink/
└── hardware/
    ├── ledblink.hw/              ← Vivado hardware handoff files
    ├── ledblink.gen/             ← generated IP output products
    ├── ledblink.ip_user_files/   ← IP user files
    ├── ledblink.runs/            ← implementation runs
    ├── ledblink.sim/             ← simulation files
    ├── ledblink.srcs/            ← source files (VHDL, constraints)
    ├── ledblink.cache/           ← Vivado cache
    └── ledblink.tmp/             ← temporary build files
```

---

## Prerequisites

- Vivado 2025.2 (ML Enterprise License)
- Alinx VD100 board (XCVE2302-SFVA784-1LP-E-S)
- USB-UART cable for JTAG/serial console

---

## Block Design Topology

```
versal_cips_0                   axi_smc              MyLEDIP_0
┌──────────────────────┐       ┌──────────┐        ┌────────────────┐
│                      │       │          │        │                │
│  M_AXI_FPD ─────────┼──────▶│ S00_AXI  │        │  s00_axi       │
│                      │       │   M00_AXI┼───────▶│                │
│  (clk/rst) ─────────┼──────▶│ aclk     │        │  s00_axi_aclk  │
│                      │       │ aresetn  │        │  s00_axi_aresetn│
└──────────────────────┘       └──────────┘        │                │
         │                                         │  pl_led ───────┼──▶ led (E20)
         │         axi_noc_0                       └────────────────┘
         │        ┌──────────────────────┐
         ├───────▶│ S00_AXI .. S05_AXI   │
         │        │  CH0_DDR4_0 ─────────┼──▶ DDR4
         │        └──────────────────────┘
         │
         ▼
   rst_versal_cips_0_100M
   ilconstant_0 (dcm_locked tie-off)
```

**Critical rule:** M_AXI_FPD → SmartConnect → MyLEDIP for all PL peripherals. **Never route PL peripherals through the NoC** — address space conflicts, PMC EAM errors, and kernel panics result.

---

## M_AXI_FPD — The Right Path for PL Peripherals

M_AXI_FPD is the correct AXI master port for accessing custom PL peripherals from the Versal PS. It must be **explicitly enabled** in the CIPS IP — it is disabled by default.

### Enable in CIPS

```
CIPS IP → PS PMC → PS-PL Interfaces → M AXI FPD → Enable ✓
```

### Valid address apertures

| Aperture base | Size | Use |
|---|---|---|
| `0xA400_0000` | 448 MB | Y PL peripherals — use this range |
| `0x4_0000_0000` | 8 GB | Extended PL space |
| `0x400_0000_0000` | 1 TB | Large PL space |

`MyLEDIP` is mapped at **`0xA4000000`** — within the first aperture.

### Why NOT the NoC for PL peripherals

The AXI NoC (`axi_noc_0`) is the path to DDR4. Routing PL peripheral accesses through the NoC causes:
- Address space conflicts with DDR4 regions
- PMC EAM (Error and Alarm Management) errors at boot
- Kernel panics during peripheral probe

The SmartConnect (`axi_smc`) is the correct interconnect between M_AXI_FPD and PL AXI-lite slaves.

---

## MyLEDIP Custom IP

`MyLEDIP` is a custom AXI-lite slave IP created from the Vivado AXI4-Lite peripheral template. It exposes a 2-register interface to control and read back the PL LED state.

### Register Map

| Offset | Access | Width | Description |
|---|---|---|---|
| `0x00` | R/W | 32-bit | **Control** — bit 0: `1` = LED on, `0` = LED off |
| `0x04` | R | 32-bit | **Status** — mirrors control register (`slv_reg1 <= slv_reg0`) |

Base address: **`0xA4000000`**  
Size: `0x10000` (64 KB — minimum AXI-lite aperture)

### Address Editor (Vivado)

```
Master          Base Address    Range
M_AXI_FPD      0xA400_0000    64K
```

---

## Pin Constraints

```xdc
# PL LED — E20
set_property PACKAGE_PIN E20 [get_ports led]
set_property IOSTANDARD  LVCMOS15 [get_ports led]

# PL KEY1 — reset input — F21
set_property PACKAGE_PIN F21 [get_ports rst_n]
set_property IOSTANDARD  LVCMOS15 [get_ports rst_n]
```

---

## MyLEDIP VHDL — AXI-lite State Machine

The write path uses a two-state FSM (`Waddr` → `Wdata`). Two fixes are required beyond the Vivado template default.

### Fix 1 — wready explicit deassertion

`axi_wready` must be explicitly deasserted in **both** the `Waddr` and `Wdata` states. The Vivado template does not deassert in `Waddr`, causing spurious write acceptance on the next cycle:

```vhdl
when Waddr =>
    axi_wready <= '0';  -- explicit deassertion required
    if s00_axi_awvalid = '1' and axi_awready = '1' then
        state_write <= Wdata;
        axi_wready  <= '1';
    end if;

when Wdata =>
    if s00_axi_wvalid = '1' then
        state_write <= Waddr;
        axi_bvalid  <= '1';
        axi_awready <= '1';
        axi_wready  <= '0';  -- must deassert here too
    end if;
```

### Fix 2 — status register inside clocked process

`slv_reg1` (status mirror of `slv_reg0`) must be assigned **inside** the clocked write process. A concurrent signal assignment outside the process creates a multiple-driver elaboration error:

```vhdl
-- WRONG — concurrent assignment outside process causes multiple-driver error
slv_reg1 <= slv_reg0;

-- CORRECT — inside clocked write process, after case statement
process (s00_axi_aclk)
begin
    if rising_edge(s00_axi_aclk) then

        -- AXI write logic
        case state_write is
            when Waddr => ...
            when Wdata => ...
        end case;

        -- Mirror control register to status register
        slv_reg1 <= slv_reg0;

    end if;
end process;
```

---

## CIPS Configuration

### SD Card (sdhci1)

| Setting | Value | Notes |
|---|---|---|
| SD interface | SD 3.0 | Vivado CIPS config |
| MIO bank | Bank 501 | 1.8V |
| Physical interface | via TXS02612 | level shifter to 3.3V |

> The TXS02612 level shifter limits SD clock to ~30 MHz. The Linux DTB patch `max-frequency = <25000000>` is required — see [yocto/meta-vd100_v2/README.md](../../yocto/meta-vd100_v2/README.md).

### Ethernet

| Setting | Value |
|---|---|
| Interface | RGMII |
| MIO | Bank 501 |
| PHY | Auto-discovered by generic PHY driver — no `phy-handle` required in DTS |

### USB

| Setting | Value |
|---|---|
| Controller | DWC3 at `fe200000` |
| Mode | Host |

### I2C

| Bus label | Address | Linux device | Connected devices |
|---|---|---|---|
| `i2c0` | `ff020000` (LPD) | `i2c-0` | — |
| `i2c1` | `ff030000` (LPD) | `i2c-1` | — |
| `i2c2` | `f1000000` (PMC) | `i2c-2` | LM75 @ 0x48, EEPROM @ 0x50 |

> **DTS label ≠ Linux number.** The DTS label `i2c2` maps to physical address `f1000000` (PMC I2C), which Linux enumerates as `i2c-2`. Confirmed by `i2cdetect -y 2` showing `UU` at 0x50–0x57 and lm75 probing as `2-0048`. The `ff030000` bus (LPD) has no connected devices on the VD100.

### GPIO

| Controller | Address | Lines | Connected |
|---|---|---|---|
| LPD GPIO | `ff0b0000` | 58 lines | PS_LED1 on MIO25 |
| PMC GPIO | `f1020000` | — | — |

---

## XSA Export

After implementation, export the XSA for System Device Tree generation:

```
Vivado menu: File → Export → Export Hardware (XSA)
    ☑ Include bitstream
    Output: ledblink.xsa
```

---

## Boot Mode Switches

The Alinx VD100 uses active-low boot mode switches — physical switch positions are inverted from MODE bit values.

| MODE | Boot source | Physical switches | Notes |
|---|---|---|---|
| `1110` | SD1 LS (level-shifter) | OFF OFF OFF ON | Y Correct for VD100 |
| `0101` | SD1 SD 2.0 | ON OFF ON OFF | Also works with DTB patch |
| `0000` | JTAG | ON ON ON ON | Development/debug |

---

## Next Step

Export the XSA and proceed to System Device Tree generation:  
**[../software/vd100_sdt/README.md](../software/vd100_sdt/README.md)**
