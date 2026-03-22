# MyLEDIP v1.0 — AXI-lite PL LED Controller

Custom AXI4-Lite slave IP for the Alinx VD100 (XCVE2302-SFVA784-1LP-E-S).  
Controls a PL-connected LED via memory-mapped registers from the Versal PS.  
Driven from Linux via the `myledip` kernel module through `/dev/myledip`.

---

## IP Repository Structure

```
MyLEDIP_1_0/
├── component.xml              ← IP-XACT descriptor (Vivado IP catalog metadata)
├── xgui/
│   └── MyLEDIP_v1_0.tcl       ← Vivado IP GUI customisation
└── hdl/
    └── vhdl/
        ├── MyLEDIP_v1_0.vhd           ← Top-level wrapper
        └── MyLEDIP_v1_0_S00_AXI.vhd  ← AXI-lite slave logic + register bank
```

---

## Interface

### Ports

| Port | Direction | Width | Description |
|---|---|---|---|
| `s00_axi_aclk` | in | 1 | AXI clock |
| `s00_axi_aresetn` | in | 1 | AXI active-low reset |
| `s00_axi` | slave | — | AXI4-Lite slave bus (addr, data, strobe, valid, ready, resp) |
| `pl_led` | out | 1 | PL LED output — connect to board pin E20 |

### AXI4-Lite Parameters

| Parameter | Value |
|---|---|
| Data width | 32-bit |
| Address width | 4-bit (covers 4 × 32-bit registers) |
| Protocol | AXI4-Lite |
| Number of registers | 2 (slv_reg0, slv_reg1) |

---

## Register Map

Base address: **`0xA4000000`** (set in Vivado Address Editor, M_AXI_FPD aperture)

| Offset | Name | Access | Description |
|---|---|---|---|
| `0x00` | Control (`slv_reg0`) | R/W | Bit 0: `1` = LED on, `0` = LED off. Bits 31:1 reserved. |
| `0x04` | Status (`slv_reg1`) | R | Mirrors control register. Confirms the written value was latched. |

```
0xA4000000  ┌─────────────────────────────────┐
            │ slv_reg0 [31:1] reserved         │
            │ slv_reg0 [0]    LED control       │ ← write 1: ON, write 0: OFF
0xA4000004  ├─────────────────────────────────┤
            │ slv_reg1 [31:0] status (mirror)  │ ← read to confirm
            └─────────────────────────────────┘
```

LED output is driven directly from bit 0 of `slv_reg0`:

```vhdl
pl_led <= slv_reg0(0);
```

---

## VHDL Architecture

The AXI-lite slave uses a two-state write FSM and a standard single-state read path, derived from the Vivado AXI4-Lite peripheral template. Two fixes are applied beyond the template default.

### Write FSM

```
        ┌─────────────────────────────────────────┐
        │              RESET                       │
        └─────────────────┬───────────────────────┘
                          │
                          ▼
              ┌──────────────────────┐
         ┌───▶│   Waddr              │◀───┐
         │    │   axi_wready <= '0'  │    │
         │    │   axi_awready <= '0' │    │
         │    └──────────┬───────────┘    │
         │               │ awvalid='1'    │
         │               │ awready='1'    │  wvalid='1'
         │               ▼               │  (write complete)
         │    ┌──────────────────────┐    │
         │    │   Wdata              │────┘
         │    │   axi_wready <= '1'  │
         │    │   write register     │
         │    │   assert bvalid      │
         └────┘
```

### Fix 1 — wready explicit deassertion in Waddr state

The Vivado template omits the `axi_wready <= '0'` assignment in the `Waddr` state. Without it, `wready` holds its previous value and can cause spurious write acceptance on the next transaction:

```vhdl
when Waddr =>
    axi_wready <= '0';  -- required — not in Vivado template default
    if s00_axi_awvalid = '1' and axi_awready = '1' then
        state_write <= Wdata;
        axi_wready  <= '1';
    end if;

when Wdata =>
    if s00_axi_wvalid = '1' then
        state_write <= Waddr;
        axi_bvalid  <= '1';
        axi_awready <= '1';
        axi_wready  <= '0';  -- deassert after accepting write data
    end if;
```

### Fix 2 — status register inside clocked process

`slv_reg1` mirrors `slv_reg0` as a readback status register. It must be assigned **inside** the clocked write process. A concurrent signal assignment outside the process creates a multiple-driver elaboration error in Vivado:

```vhdl
-- WRONG — concurrent assignment outside process
slv_reg1 <= slv_reg0;   -- multiple-driver error

-- CORRECT — inside clocked write process, after case statement
process (s00_axi_aclk)
begin
    if rising_edge(s00_axi_aclk) then

        case state_write is
            when Waddr => ...
            when Wdata => ...
        end case;

        slv_reg1 <= slv_reg0;  -- one clock latency — acceptable for status

    end if;
end process;
```

---

## Add to Vivado IP Catalog

```
Vivado → Settings → IP → Repository
    → Add Repository → select path to MyLEDIP_1_0 parent folder
    → IP status: MyLEDIP v1.0 appears in catalog
```

Or in the block design:

```
Right-click canvas → Add IP → search "MyLEDIP"
```

---

## Block Design Integration

In the `ledblink` block design:

```
versal_cips_0
    M_AXI_FPD ──▶ axi_smc (SmartConnect)
                       M00_AXI ──▶ MyLEDIP_0
                                       s00_axi
                                       s00_axi_aclk
                                       s00_axi_aresetn
                                       pl_led ──▶ led (port → E20)
```

**M_AXI_FPD must be explicitly enabled in CIPS** (disabled by default):
```
CIPS IP → PS PMC → PS-PL Interfaces → M AXI FPD → Enable ✓
```

**Never connect PL peripheral slaves to the NoC** — route all PL peripheral traffic through M_AXI_FPD → SmartConnect.

### Address Editor

```
Master        Slave             Base Address    Range
M_AXI_FPD    MyLEDIP_0/S00_AXI  0xA400_0000    64K
```

---

## Pin Constraints

```xdc
# PL LED output
set_property PACKAGE_PIN E20      [get_ports pl_led]
set_property IOSTANDARD  LVCMOS15 [get_ports pl_led]
```

---

## Linux Device Tree Binding

The `myledip` kernel module binds via the compatible string exported into the System Device Tree:

```dts
&axi {
    MyLEDIP_0: MyLEDIP@a4000000 {
        compatible = "xlnx,MyLEDIP-1.0";
        reg = <0x0 0xA4000000 0x0 0x10000>;
        clocks = <&versal_clk 65>;
        clock-names = "s00_axi_aclk";
    };
};
```

> Node address must be **lowercase hex** (`@a4000000` not `@A4000000`).  
> Uppercase causes a U-Boot address parse loop that hangs boot.

---

## Linux Usage

```bash
# Via kernel module /dev/myledip
echo 1 | sudo tee /dev/myledip    # LED on
echo 0 | sudo tee /dev/myledip    # LED off
sudo cat /dev/myledip             # read status register (offset 0x04)

# Direct register access (devmem2 — debug only)
sudo devmem2 0xA4000000 w 0x1    # write control: LED on
sudo devmem2 0xA4000000 w 0x0    # write control: LED off
sudo devmem2 0xA4000004          # read status register
```

---

## Related

| Component | Repository |
|---|---|
| Kernel module (`myledip.ko`) | `yocto/meta-vd100_v2/recipes-modules/myledip/` |
| Vivado project (`ledblink`) | `ledblink/hardware/` |
| Yocto layer | `yocto/meta-vd100_v2/` |
| Full documentation | [ledblink/hardware/README.md](../../ledblink/hardware/README.md) |

---

*MyLEDIP v1.0 — AXI4-Lite — Versal XCVE2302 — Vivado 2025.2*
