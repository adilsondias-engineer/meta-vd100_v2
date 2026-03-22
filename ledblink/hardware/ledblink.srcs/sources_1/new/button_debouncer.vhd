----------------------------------------------------------------------------------
-- Company: 
-- Engineer: Adilson Dias
-- 
-- Create Date: 09.03.2026 15:01:31
-- Design Name: 
-- Module Name: button_debouncer - Behavioral
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
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity button_debouncer is
   Generic(
        DEBBOUNCE_TIME : integer := 2_000_000 --default values for hardware
    );
    Port ( clk : in STD_LOGIC;
           btn : in STD_LOGIC;
           led_out : out STD_LOGIC);
end button_debouncer;

architecture Behavioral of button_debouncer is

begin

    -- using 2-3 flip-flops to cross clock domains
    


end Behavioral;
