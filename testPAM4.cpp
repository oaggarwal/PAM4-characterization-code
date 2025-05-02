#include <iostream>
#include <chrono>
#include <thread>
#include <getopt.h>
#include <string.h>
#include <fstream>
#include <string>
#include <iomanip>
#include <sys/stat.h>
#include <map>
#include <bitset>
#include <vector>

#include <FT232H.h>
#include <HYT271.h>
#include <ComIOException.h>
#include <SPIFTDICom.h>
#include <LMK03806INO.h>
#include <METAROCKINO.h>
#include <TextSerialCom.h>
#include <SerialCom.h>
#include "Logger.h"
#include <bitset>

#include "PowerSupplyChannel.h"
#include "EquipConf.h"

std::string equipConfigFile = "equip_testbench.json";



float F_VCO = 2695.0;//VCO frequency, measured with scope
int nDivide = 6; // dT = 2.2263
int nDivide_in = 6;
int nDivide_40MHz = 66;
std::string outFileName = "out.csv";

int tsleep_write = 10;


void initializePAM4Transmitter(std::shared_ptr<TextSerialCom> com) { //configure register stuff

  std::cout<<"======== Configure METAROCK chip:"<<std::endl; //(this needs to be reconfigured based on PAM4, rn its AFE)
  std::shared_ptr<METAROCKINO> metarock(new METAROCKINO(com, 1000.0/(F_VCO/nDivide)));
  
  metarock->writeGPIO("RST", 1); // Set RST_B LOW BEFORE WriteConfig
  metarock->writeGPIO("CFGCLK", 0); // Set CFGCLK LOW BEFORE WriteConfig
  metarock->writeGPIO("CFGIN", 1); // Set CFGIN HIGH BEFORE WriteConfig
  metarock->writeGPIO("RST", 0); // Set RST_B LOW BEFORE WriteConfig
  metarock->writeGPIO("RST", 1); //Set RST HIGH


    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  
    std::cout << "Initializing PAM4 Transmitter..." << std::endl;

    // Reset PAM4 transmitter
  
    // Define configuration (example)
    uint32_t configValue = 0b0000000000000000000000000000; // Default configuration

    configValue |= (0 << 0);   // PAM4 = 0
    configValue |= (1 << 1);   // invert_precursor
    configValue |= (0 << 2);   // invert_cursor
    configValue |= (1 << 3);   // invert_postcursor
    configValue |= (1 << 4);   // disable_precursor
    configValue |= (0 << 5);   // disable_cursors
    configValue |= (1 << 6);   // disable_postcursor
    configValue |= (0 << 7);   // prbs
    configValue |= (0 << 8);   // disable_tx_term
    configValue |= (1 << 9);   // disable_tx_noterm 
    configValue |= (1 << 10);  // disable forward clk

    configValue |= (1 << 11);  // test
    configValue |= (0 << 12);  // test
    configValue |= (1 << 13);  // test
    configValue |= (0 << 14);  // test
    configValue |= (0 << 15);  // test
    configValue |= (0 << 16);  // test
    configValue |= (0 << 17);  // test
    configValue |= (0 << 18);  // test

    configValue |= (1 << 19);  // test
    configValue |= (1 << 20);  // test
    configValue |= (0 << 21);  // test
    configValue |= (0 << 22);  // test
    configValue |= (0 << 23);  // test
    configValue |= (0 << 24);  // test
    configValue |= (0 << 25);  // test
    configValue |= (0 << 26);  // test


    configValue |= (0 << 27);  // spare

    // configValue = 0b1111111111111111111111111111;
    // configValue = 0b0000000000000000000000000000;

    // uint16_t testSequence = 0b0001000100010001; // Example test sequence
    // configValue |= (static_cast<uint32_t>(testSequence) << 11); // Shift test sequence to bits 11-26
  
      std::cout << "PAM4 Transmitter Begin" << std::endl;
      // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      std::cout<<configValue<<std::endl;
      metarock->writeConfig(configValue);


    std::cout << "PAM4 Transmitter Initialization Complete." << std::endl;
    
}   



void configureClock(std::shared_ptr<LMK03806INO> clock) {
  std::cout << "======== Configure LMK03806 CLOCK for CLKout2 ========" << std::endl;

  // Read all registers initially
  for (int reg = 0; reg < 31; reg++) {
      clock->read(reg);
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }

  // Unlock registers for write
  uint32_t R31 = 0b11111;
  clock->write(R31);
  std::this_thread::sleep_for(std::chrono::milliseconds(tsleep_write));

  // Step 1: Reset R0
  uint32_t R0 = (0 << 18) | (1 << 17) | 0; // Reset the device
  clock->write(R0);
  std::this_thread::sleep_for(std::chrono::milliseconds(tsleep_write));

  // Step 2: Configure R1 for CLKout2 and CLKout3 (clock group 1)
  uint32_t R1 = (0 << 18) | (nDivide << 5) | 1; // Set divider for CLKout2 and CLKout3
  clock->write(R1);
  std::this_thread::sleep_for(std::chrono::milliseconds(tsleep_write));
  clock->write(R1); // Write twice if divider > 25
  std::this_thread::sleep_for(std::chrono::milliseconds(tsleep_write));

  // Step 3: Configure R6 for CLKout2 output type (LVDS, LVPECL, etc.)
  uint32_t R6 = (0x01 << 24) | 6; // Set CLKout2 to LVDS (0x01)
  clock->write(R6);
  std::this_thread::sleep_for(std::chrono::milliseconds(tsleep_write));

  // Step 4: Configure PLL settings (required for CLKout2 to function)
  uint32_t R24 = (0 << 28) | (0 << 24) | (0 << 20) | (0 << 16) | 24; // Default loop filter settings
  clock->write(R24);
  std::this_thread::sleep_for(std::chrono::milliseconds(tsleep_write));

  uint32_t R26 = (1 << 31) | (1 << 29) | (3 << 26) | (0b111010 << 20) | (8192 << 5) | 26; // PLL settings
  clock->write(R26);
  std::this_thread::sleep_for(std::chrono::milliseconds(tsleep_write));

  uint32_t R28 = (2 << 20) | 28; // PLL R divider
  clock->write(R28);
  std::this_thread::sleep_for(std::chrono::milliseconds(tsleep_write));

  uint32_t R29 = (0 << 24) | (1 << 22) | (25 << 5) | 29; // PLL N calibration
  clock->write(R29);
  std::this_thread::sleep_for(std::chrono::milliseconds(tsleep_write));

  uint32_t R30 = (5 << 24) | (25 << 5) | 30; // PLL N divider
  clock->write(R30);
  std::this_thread::sleep_for(std::chrono::milliseconds(tsleep_write));
  clock->write(R30); // Write twice to ensure calibration
  std::this_thread::sleep_for(std::chrono::milliseconds(tsleep_write));

  // Lock all registers - R0 to R30 only readable now
  R31 = 0b111111;
  clock->write(R31);
  std::this_thread::sleep_for(std::chrono::milliseconds(tsleep_write));

  // Read back all registers
  for (int reg = 0; reg < 31; reg++) {
      clock->read(reg);
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }

  std::cout << "======== Configure LMK03806 CLOCK for CLKout2 done ========" << std::endl;
}


int main(int argc, char** argv) //clock divider
{  

  
  if(argc > 1)
  {
    nDivide_in = std::stoi(argv[1]);
    if (nDivide_in > 0) nDivide = nDivide_in;
  }

  if(argc > 2)
  {
    outFileName = argv[2];
  }



  std::shared_ptr<TextSerialCom> com(new TextSerialCom("/dev/ttyACM0", SerialCom::BaudRate::Baud115200));
  com->setTermination("\n");
  com->init();
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  

  //clock
  std::shared_ptr<LMK03806INO> clock(new LMK03806INO(com));
  if (nDivide_in > 0) {
    configureClock(clock);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  
  initializePAM4Transmitter(com);


  return 0;
}






