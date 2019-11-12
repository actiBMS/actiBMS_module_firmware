/*
  ____  ____  _  _  ____  __  __  ___    _  _  __
  (  _ \(_  _)( \/ )(  _ \(  \/  )/ __)  ( \/ )/. |
  )(_) )_)(_  \  /  ) _ < )    ( \__ \   \  /(_  _)
  (____/(____) (__) (____/(_/\/\_)(___/    \/   (_)

  DIYBMS V4.0
  CELL MODULE FOR ATTINY841

  (c)2019 Stuart Pittaway

  COMPILE THIS CODE USING PLATFORM.IO

  LICENSE
  Attribution-NonCommercial-ShareAlike 2.0 UK: England & Wales (CC BY-NC-SA 2.0 UK)
  https://creativecommons.org/licenses/by-nc-sa/2.0/uk/

  Non-Commercial — You may not use the material for commercial purposes.
  Attribution — You must give appropriate credit, provide a link to the license, and indicate if changes were made.
  You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use.
  ShareAlike — If you remix, transform, or build upon the material, you must distribute your
  contributions under the same license as the original.
  No additional restrictions — You may not apply legal terms or technological measures
  that legally restrict others from doing anything the license permits.
*/

#ifndef _DIYBMS_CORE_H
#define _DIYBMS_CORE_H

#include <tinyNeoPixel.h>
#include <FastPID.h>

#include "diybms_hal.h"

#include "steinhart.h"
#include "settings.h"
#include "crc16.h"

//This is where the data begins in EEPROM
#define EEPROM_CONFIG_ADDRESS 0x10

/*
   Configuration

   All the settings that we store into the EEPROM
*/

struct config_t {

  // Communication Addresses
  uint8_t bank_id;
  uint8_t cell_id;

  // Bypass Over Temperature Threshold
  uint8_t bypass_threshold_temperature;

  // Bypass Over Temperature Threshold
  uint16_t bypass_threshold_voltage;

  // Resistance of bypass load
  float bypass_resistance;

  // Voltage Calibration
  float volt_offset;

  // Reference voltage (millivolt) normally 2.00mV
  float volt_reference;

  // Internal Thermistor settings
  uint16_t t_internal_beta;

  // External Thermistor settings
  uint16_t t_external_beta;

} __attribute__((packed));

class DiyBMS {
  public:
    DiyBMS(BMSHal *hardware) {
      _hardware = hardware;

      _settings = new Settings(EEPROM_CONFIG_ADDRESS);

      // Check if setup routine needs to be run
      if (!load()) {
        loadDefault();
        store();
      }

      pixels = tinyNeoPixel(0, A6, NEO_GRB + NEO_KHZ800);

      //3Hz rate - number of times we call this code in Loop
      pid = FastPID (150.0, 2.5, 5, 3, 16, false);
    }

    void update();

    uint16_t incrementWatchdogCounter() {
      return ++watchdog_counter;
    }

    uint16_t cellVoltage();
    float chipTemperature();
    float onboardTemperature();
    float externalTemperature();

    void adcRequest(uint8_t value);
    uint16_t adcRaw();

    // Configuration Handler
    uint8_t setAddrBank(uint8_t value, bool store);
    uint8_t getAddrBank();

    uint8_t setAddrCell(uint8_t value, bool store);
    uint8_t getAddrCell();

    uint8_t setBypassTemp(uint8_t value, bool store);
    uint8_t getBypassTemp();
    
    uint16_t setBypassVoltage( uint16_t value, bool store);
    uint16_t getBypassVoltage();
    
    uint16_t setTempIntBeta( uint16_t value, bool store);
    uint16_t getTempIntBeta();

    uint16_t setTempExtBeta( uint16_t value, bool store);
    uint16_t getTempExtBeta();
    
    float setBypassResistance(float value, bool store);
    float getBypassResistance();
    
    float setVoltOffset(float value, bool store);
    float getVoltOffset();
    
    float setVoltReference(float value, bool store);
    float getVoltReference();

    bool restoreFactory();
    bool store();
    bool load();

  private:
    Settings * _settings;
    BMSHal *_hardware;
    volatile struct config_t _config;

    tinyNeoPixel pixels;
    FastPID pid;    

    volatile bool _bypass_active;
    volatile bool _identify_active;

    volatile float cell_voltage;
    volatile float chip_temperature;
    volatile float onboard_temperature;
    volatile float external_temperature;

    volatile uint16_t watchdog_counter = 0;
    volatile uint16_t bypass_count_down = 0;
    volatile uint16_t bypass_count_finished = 0;

    bool checkBypass();
    bool checkBypassOverheat();
    void loadDefault();

};

#endif
