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

#define WDT_REBOOT_CNT 3

#include <FastPID.h>

#include "diybms_hal.h"

#include "steinhart.h"
#include "settings.h"
#include "crc16.h"

//This is where the data begins in EEPROM
#define EEPROM_CONFIG_ADDRESS 0x10

#define THERMISTOR_TEMPERATURE 25
#define THERMISTOR_NOMINAL 47000
#define RESISTOR_SERIES 47000

/*
   Statuses

   All the statuses that the cell module can have
*/

enum STATUS : uint8_t
{
  STAT_PROVISIONED = 0x01,
  STAT_IDENTIFY = 0x02,
  STAT_BALANCING = 0x04,
  STAT_BYPASSING = 0x08,
  STAT_AWAKED = 0x10,
  STAT_OVER_TEMP = 0x20,
  STAT_UNDER_TEMP = 0x40,
  STAT_FAULT = 0x80
};

/*
   Configuration STRUCT
   All the settings that we store into the EEPROM
*/
struct config_t {

  // Communication Addresses
  uint8_t bank_id;
  uint8_t cell_id;

  // Cell Over Temperature Threshold
  uint8_t cell_threshold_over_temperature;

  // Cell Under Temperature Threshold
  uint8_t cell_threshold_under_temperature;

  // Enable Passive Balancing
  uint8_t enable_bypass;

  // Bypass Over Temperature Threshold
  uint8_t bypass_threshold_over_temperature;

  // Bypass Temperature Setpoint
  uint8_t bypass_temperature_setpoint;

  // Bypass Voltage Threshold (millivolt)
  uint16_t bypass_threshold_voltage;

  // Resistance of bypass load (milliohm) normally
  uint16_t bypass_resistance;

  // # of iterations in bypass mode before another check
  uint16_t bypass_duration_count;

  // # of iterations to wait for before next bypass
  uint16_t bypass_cooldown_count;

  // Enable Active Balancing
  uint8_t enable_balancing;

  // Enable Active Balancing
  uint8_t enable_balancing_onbypass;

  // Active Balancing Voltage Threshold (millivolt)
  uint16_t balancing_threshold_voltage;

  // Voltage Calibration (millivolt)
  uint16_t volt_offset;

  // Reference voltage (millivolt) normally 2mV
  uint16_t volt_reference;

  // Internal Thermistor Beta Coeff
  uint16_t t_internal_beta;

  // External Thermistor Beta Coeff
  uint16_t t_external_beta;

} __attribute__((packed));

class DiyBMS {
  public:
    volatile struct config_t _config;

    DiyBMS(BMSHal *hardware) {
      _hardware = hardware;
      _settings = new Settings(EEPROM_CONFIG_ADDRESS);

      //3Hz rate - number of times we call this code in Loop
      _pid = new FastPID (150.0, 2.5, 5, 3, 16, false);
    }

    bool begin();
    bool update();
    void onWakeup();

    uint16_t isrWatchdog() {
      _watchdog_isr = true;

      if (WDT_REBOOT_CNT > 0 && _watchdog_counter >= WDT_REBOOT_CNT) {
        _hardware->watchdogReboot();
      }

      _hardware->watchdogReset();

      return ++_watchdog_counter;
    }

    uint16_t isrSerialRX() {
      _serial_isr = true;
    }

    uint16_t isrPCI() {
      _pci_isr = true;
    }

    uint8_t getCellStatus();

    // Return Cell Voltage in mV
    uint16_t getCellVoltage();

    // Return Temperatures in deciCelsius
    int getChipTemperature();
    int getOnboardTemperature();
    int getExternalTemperature();

    // Request and return a RAW ADC data
    uint16_t adcRawRequest(uint8_t channel, bool more);

    // Enable or Disable identify STATUS
    void setIdentify(bool value);
    bool getIdentify();

    // Configuration Handler
    bool restoreFactoryConfig();
    bool storeConfig();
    bool loadConfig();

  private:
    Settings* _settings;
    BMSHal* _hardware;
    FastPID* _pid;

    volatile uint8_t _status;

    volatile uint16_t _cell_voltage;

    volatile int _chip_temperature;
    volatile int _onboard_temperature;
    volatile int _external_temperature;

    volatile bool _watchdog_isr = false;
    volatile bool _serial_isr = false;
    volatile bool _pci_isr = false;

    volatile uint16_t _watchdog_counter = 0;

    volatile uint16_t _bypass_count_down = 0;
    volatile uint16_t _bypass_count_finished = 0;

    bool checkBypass();
    bool checkBypassOverheat();

    void adcAcquireData(uint8_t channel, bool more);
    void loadDefaultConfig();
};

#endif
