--Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
--Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
----------------------------------------------------------------------------------
--Tool Version: Vivado v.2025.2 (win64) Build 6299465 Fri Nov 14 19:35:11 GMT 2025
--Date        : Sun Mar 22 14:01:11 2026
--Host        : Mercury running 64-bit major release  (build 9200)
--Command     : generate_target design_1_wrapper.bd
--Design      : design_1_wrapper
--Purpose     : IP block netlist
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity design_1_wrapper is
  port (
    DDR4_act_n : out STD_LOGIC_VECTOR ( 0 to 0 );
    DDR4_adr : out STD_LOGIC_VECTOR ( 16 downto 0 );
    DDR4_ba : out STD_LOGIC_VECTOR ( 1 downto 0 );
    DDR4_bg : out STD_LOGIC_VECTOR ( 0 to 0 );
    DDR4_ck_c : out STD_LOGIC_VECTOR ( 0 to 0 );
    DDR4_ck_t : out STD_LOGIC_VECTOR ( 0 to 0 );
    DDR4_cke : out STD_LOGIC_VECTOR ( 0 to 0 );
    DDR4_cs_n : out STD_LOGIC_VECTOR ( 0 to 0 );
    DDR4_dm_n : inout STD_LOGIC_VECTOR ( 7 downto 0 );
    DDR4_dq : inout STD_LOGIC_VECTOR ( 63 downto 0 );
    DDR4_dqs_c : inout STD_LOGIC_VECTOR ( 7 downto 0 );
    DDR4_dqs_t : inout STD_LOGIC_VECTOR ( 7 downto 0 );
    DDR4_odt : out STD_LOGIC_VECTOR ( 0 to 0 );
    DDR4_reset_n : out STD_LOGIC_VECTOR ( 0 to 0 );
    led : out STD_LOGIC;
    rst_n : in STD_LOGIC;
    sys_clk_clk_n : in STD_LOGIC_VECTOR ( 0 to 0 );
    sys_clk_clk_p : in STD_LOGIC_VECTOR ( 0 to 0 )
  );
end design_1_wrapper;

architecture STRUCTURE of design_1_wrapper is
  component design_1 is
  port (
    DDR4_dq : inout STD_LOGIC_VECTOR ( 63 downto 0 );
    DDR4_dqs_t : inout STD_LOGIC_VECTOR ( 7 downto 0 );
    DDR4_dqs_c : inout STD_LOGIC_VECTOR ( 7 downto 0 );
    DDR4_adr : out STD_LOGIC_VECTOR ( 16 downto 0 );
    DDR4_ba : out STD_LOGIC_VECTOR ( 1 downto 0 );
    DDR4_bg : out STD_LOGIC_VECTOR ( 0 to 0 );
    DDR4_act_n : out STD_LOGIC_VECTOR ( 0 to 0 );
    DDR4_reset_n : out STD_LOGIC_VECTOR ( 0 to 0 );
    DDR4_ck_t : out STD_LOGIC_VECTOR ( 0 to 0 );
    DDR4_ck_c : out STD_LOGIC_VECTOR ( 0 to 0 );
    DDR4_cke : out STD_LOGIC_VECTOR ( 0 to 0 );
    DDR4_cs_n : out STD_LOGIC_VECTOR ( 0 to 0 );
    DDR4_dm_n : inout STD_LOGIC_VECTOR ( 7 downto 0 );
    DDR4_odt : out STD_LOGIC_VECTOR ( 0 to 0 );
    sys_clk_clk_p : in STD_LOGIC_VECTOR ( 0 to 0 );
    sys_clk_clk_n : in STD_LOGIC_VECTOR ( 0 to 0 );
    led : out STD_LOGIC;
    rst_n : in STD_LOGIC
  );
  end component design_1;
begin
design_1_i: component design_1
     port map (
      DDR4_act_n(0) => DDR4_act_n(0),
      DDR4_adr(16 downto 0) => DDR4_adr(16 downto 0),
      DDR4_ba(1 downto 0) => DDR4_ba(1 downto 0),
      DDR4_bg(0) => DDR4_bg(0),
      DDR4_ck_c(0) => DDR4_ck_c(0),
      DDR4_ck_t(0) => DDR4_ck_t(0),
      DDR4_cke(0) => DDR4_cke(0),
      DDR4_cs_n(0) => DDR4_cs_n(0),
      DDR4_dm_n(7 downto 0) => DDR4_dm_n(7 downto 0),
      DDR4_dq(63 downto 0) => DDR4_dq(63 downto 0),
      DDR4_dqs_c(7 downto 0) => DDR4_dqs_c(7 downto 0),
      DDR4_dqs_t(7 downto 0) => DDR4_dqs_t(7 downto 0),
      DDR4_odt(0) => DDR4_odt(0),
      DDR4_reset_n(0) => DDR4_reset_n(0),
      led => led,
      rst_n => rst_n,
      sys_clk_clk_n(0) => sys_clk_clk_n(0),
      sys_clk_clk_p(0) => sys_clk_clk_p(0)
    );
end STRUCTURE;
