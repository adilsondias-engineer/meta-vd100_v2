#include <string>
#include <stdio.h>
#include <iostream>
#include <ctime>
#include <gpiod.hpp>
#include <filesystem>
#include <cstdlib>
#include <fstream>

using namespace std;

const std::filesystem::path chip_path("/dev/gpiochip0");
const gpiod::line::offset line_offset = 25;
const std::filesystem::path STATE_FILE = "/var/run/led25.state"; //persist state

const std::filesystem::path pl_led_path("/dev/plledip");
const std::filesystem::path PL_LED_STATE_FILE = "/var/run/pl_led.state"; //persist state

int main()
{

   cout << "Hello World!" << endl;
   cout << "Welcome to my little FPGA world - Adilson Dias 2026 " << endl;
   cout << "\n**********My own VD100 custom build!!!!!!!! " << endl;
   time_t timestamp;
   time(&timestamp);

   // Display the date and time represented by the timestamp
   cout << ctime(&timestamp);

    // Read persisted state — default inactive if no file
    bool active = false;
    if (std::filesystem::exists(STATE_FILE)) {
        std::ifstream f(STATE_FILE);
        f >> active;
    }

    bool pl_led_active = false;
    if (std::filesystem::exists(PL_LED_STATE_FILE)) {
        std::ifstream f(PL_LED_STATE_FILE);
        f >> pl_led_active;
    }


    ::std::cout << line_offset << " PS LED=" << " last known state=" << active  << ::std::endl;
    // Toggle
    active = !active;

   auto request = ::gpiod::chip(chip_path)
                          .prepare_request()
                          .set_consumer("get-line-value")
                          .add_line_settings(
                              line_offset,
                              ::gpiod::line_settings().set_direction(
                                  ::gpiod::line::direction::OUTPUT))
                          .do_request();
  ::std::cout << line_offset << " PS LED current=" << (request.get_value(line_offset) == ::gpiod::line::value::ACTIVE ? "Active" : "Inactive") << ::std::endl;

   if (active)
   {
      request.set_value(line_offset, gpiod::line::value::ACTIVE); // on
   }else{
      request.set_value(line_offset, gpiod::line::value::INACTIVE); // off
   }
   // Persist new state
    std::ofstream f(STATE_FILE);
    f << active;

   std::cout << line_offset << " PS LED=" << " current state=" << active  << " :" << (request.get_value(line_offset) == ::gpiod::line::value::ACTIVE ? "Active" : "Inactive") << ::std::endl;


   std::cout << "PL LED=" << " last knwon state=" << pl_led_active  << " :" << (pl_led_active ? "Active" : "Inactive") << ::std::endl;
   //Toggle PL LED
   pl_led_active = !pl_led_active;

   std::ofstream pl_f(pl_led_path);
   pl_f << pl_led_active;
   //write(fd, "1", 1);  // PL LED on
   //write(fd, "0", 1);  // PL LED off
   //close(fd);
   // Persist new state
   std::ofstream pl_f_state(PL_LED_STATE_FILE);
   pl_f_state << pl_led_active;

   std::cout << "PL LED=" << " current state=" << pl_led_active  << " :" << (pl_led_active ? "Active" : "Inactive") << ::std::endl;

   return 0;
}
//$CXX $CFLAGS pl_led_driver.cpp -o pl_led_driver $LDFLAGS -lgpiodcxx -lgpiod