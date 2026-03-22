----------------------------------------------------------------------------------
-- Company: 
-- Engineer: Adilson Dias
-- 
-- Create Date: 08.03.2026 19:06:07
-- Design Name: 
-- Module Name: led - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
library UNISIM;
use IEEE.STD_LOGIC_1164.ALL;
use UNISIM.VComponents.all;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.

entity led is
   Port (
   
        --DDR4
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
        
        --reset and clock
        sys_clk_p : in std_logic;
        sys_clk_n : in std_logic;
        rst_n: in std_logic;
        
        -- LED
        led : out std_logic
    
    );
end led;

architecture Behavioral of led is

--signal clk : std_logic;
--signal ledStatus: std_logic := '0'; -- led 1 is on, 0 is off active active

begin
   -- led <= ledStatus;
    
    ps_block: entity work.design_1
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
         sys_clk_clk_n(0) => sys_clk_n,
         sys_clk_clk_p(0) => sys_clk_p
       );
    
--    IBUFDS_inst : IBUFDS
--    port map (
--     O => clk,   -- 1-bit output: Buffer output
--     I => sys_clk_p,   -- 1-bit input: Diff_p buffer input (connect directly to top-level port)
--     IB => sys_clk_n  -- 1-bit input: Diff_n buffer input (connect directly to top-level port)
--    );
    
--    process(clk)
--    begin
--         --if rst_n = '1' then
--         --  led <= ~led; --'1';
--         if rising_edge(clk) and rst_n = '0' then -- rst_n active low
--           ledStatus <= not ledStatus;
--         end if; 
--    end process;

end Behavioral;
