-- Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
-- Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
-- -------------------------------------------------------------------------------
-- This file contains confidential and proprietary information
-- of AMD and is protected under U.S. and international copyright
-- and other intellectual property laws.
--
-- DISCLAIMER
-- This disclaimer is not a license and does not grant any
-- rights to the materials distributed herewith. Except as
-- otherwise provided in a valid license issued to you by
-- AMD, and to the maximum extent permitted by applicable
-- law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
-- WITH ALL FAULTS, AND AMD HEREBY DISCLAIMS ALL WARRANTIES
-- AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
-- BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
-- INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
-- (2) AMD shall not be liable (whether in contract or tort,
-- including negligence, or under any other theory of
-- liability) for any loss or damage of any kind or nature
-- related to, arising under or in connection with these
-- materials, including for any direct, or any indirect,
-- special, incidental, or consequential loss or damage
-- (including loss of data, profits, goodwill, or any type of
-- loss or damage suffered as a result of any action brought
-- by a third party) even if such damage or loss was
-- reasonably foreseeable or AMD had been advised of the
-- possibility of the same.
--
-- CRITICAL APPLICATIONS
-- AMD products are not designed or intended to be fail-
-- safe, or for use in any application requiring fail-safe
-- performance, such as life-support or safety devices or
-- systems, Class III medical devices, nuclear facilities,
-- applications related to the deployment of airbags, or any
-- other applications that could lead to death, personal
-- injury, or severe property or environmental damage
-- (individually and collectively, "Critical
-- Applications"). Customer assumes the sole risk and
-- liability of any use of AMD products in Critical
-- Applications, subject only to applicable laws and
-- regulations governing limitations on product liability.
--
-- THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
-- PART OF THIS FILE AT ALL TIMES.
--
-- DO NOT MODIFY THIS FILE.

-- MODULE VLNV: amd.com:blockdesign:design_1:1.0

-- The following code must appear in the VHDL architecture header.

-- COMP_TAG     ------ Begin cut for COMPONENT Declaration ------
COMPONENT design_1
  PORT (
    DDR4_dq : INOUT STD_LOGIC_VECTOR(63 DOWNTO 0);
    DDR4_dqs_t : INOUT STD_LOGIC_VECTOR(7 DOWNTO 0);
    DDR4_dqs_c : INOUT STD_LOGIC_VECTOR(7 DOWNTO 0);
    DDR4_adr : OUT STD_LOGIC_VECTOR(16 DOWNTO 0);
    DDR4_ba : OUT STD_LOGIC_VECTOR(1 DOWNTO 0);
    DDR4_bg : OUT STD_LOGIC_VECTOR(0 DOWNTO 0);
    DDR4_act_n : OUT STD_LOGIC_VECTOR(0 DOWNTO 0);
    DDR4_reset_n : OUT STD_LOGIC_VECTOR(0 DOWNTO 0);
    DDR4_ck_t : OUT STD_LOGIC_VECTOR(0 DOWNTO 0);
    DDR4_ck_c : OUT STD_LOGIC_VECTOR(0 DOWNTO 0);
    DDR4_cke : OUT STD_LOGIC_VECTOR(0 DOWNTO 0);
    DDR4_cs_n : OUT STD_LOGIC_VECTOR(0 DOWNTO 0);
    DDR4_dm_n : INOUT STD_LOGIC_VECTOR(7 DOWNTO 0);
    DDR4_odt : OUT STD_LOGIC_VECTOR(0 DOWNTO 0);
    sys_clk_clk_p : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    sys_clk_clk_n : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    led : OUT STD_LOGIC;
    rst_n : IN STD_LOGIC
  );
END COMPONENT;
-- COMP_TAG_END ------  End cut for COMPONENT Declaration  ------

-- The following code must appear in the VHDL architecture
-- body. Substitute your own instance name and net names.

-- INST_TAG     ------ Begin cut for INSTANTIATION Template ------
your_instance_name : design_1
  PORT MAP (
    DDR4_dq => DDR4_dq,
    DDR4_dqs_t => DDR4_dqs_t,
    DDR4_dqs_c => DDR4_dqs_c,
    DDR4_adr => DDR4_adr,
    DDR4_ba => DDR4_ba,
    DDR4_bg => DDR4_bg,
    DDR4_act_n => DDR4_act_n,
    DDR4_reset_n => DDR4_reset_n,
    DDR4_ck_t => DDR4_ck_t,
    DDR4_ck_c => DDR4_ck_c,
    DDR4_cke => DDR4_cke,
    DDR4_cs_n => DDR4_cs_n,
    DDR4_dm_n => DDR4_dm_n,
    DDR4_odt => DDR4_odt,
    sys_clk_clk_p => sys_clk_clk_p,
    sys_clk_clk_n => sys_clk_clk_n,
    led => led,
    rst_n => rst_n
  );
-- INST_TAG_END ------  End cut for INSTANTIATION Template  ------

-- You must compile the wrapper file design_1.vhd when simulating
-- the module, design_1. When compiling the wrapper file, be sure to
-- reference the VHDL simulation library.
