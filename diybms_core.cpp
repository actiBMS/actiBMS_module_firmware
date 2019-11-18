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

#include "diybms_core.h"

bool DiyBMS::begin() {
  // Check if setup routine needs to be run
  if (!loadConfig()) {
    loadDefaultConfig();
    storeConfig();
  }

  //The TIMER2 can vary between 0 and 10,000
  _pid->setOutputRange(0, 10000);

  if (_pid->err()) {
    _status |= STAT_FAULT;

    return false;
  }

  return true;
}

//This should runs around 3 times per second when the module is in bypass mode
bool DiyBMS::update() {
  int cell_temperature;

  int cell_threshold_over_temperature = ((int)(_config.cell_threshold_over_temperature) * 10);
  int cell_threshold_under_temperature = ((int)(_config.cell_threshold_under_temperature) * 10);

  int bypass_temperature_setpoint = ((int)(_config.bypass_temperature_setpoint) * 10);
  int bypass_threshold_over_temperature = ((int)(_config.bypass_threshold_over_temperature) * 10);

  // Clear STATUS::STAT_AWAKED flag;
  _status &= ~STATUS::STAT_AWAKED;

  // We always take a voltage and temperature reading on every loop cycle to check if we need to go into bypass
  // this is also triggered by the watchdog should comms fail or the module is running standalone
  // Probably over kill to do it this frequently

  // Read Cell voltage
  adcAcquireData(ADC_CELL_VOLTAGE, true);

  // Onboard temperature
  adcAcquireData(ADC_INTERNAL_TEMP, true);

  // External temperature, last measurement!
  adcAcquireData(ADC_EXTERNAL_TEMP, false);

  _cell_voltage = 3800;
  _onboard_temperature = 267;
  _external_temperature = 397;

  if (_external_temperature == -2732) {
    cell_temperature = _onboard_temperature;
  } else {
    cell_temperature = _external_temperature;
  }

  Serial.printf(F("Cell Voltage: %dmV\n"), _cell_voltage);
  Serial.printf(F("Cell Temperature: %d°C\n"), cell_temperature);
  Serial.printf(F("Cell Voltage Bypass: %dmV\n"), _config.bypass_threshold_voltage);
  Serial.printf(F("Cell Temperature Threshold Over: %d°C\n"), cell_threshold_over_temperature);
  Serial.printf(F("Cell Temperature Threshold Under: %d°C\n"), cell_threshold_under_temperature);

  Serial.printf(F("Board Temperature: %d°C\n"), _onboard_temperature);
  Serial.printf(F("Board Temperature Setpoint: %d°C\n"), bypass_temperature_setpoint);
  Serial.printf(F("Board Temperature Threshold Over: %d°C\n"), bypass_threshold_over_temperature);

  Serial.println();

  if (cell_temperature > cell_threshold_over_temperature) {
    _status |= STATUS::STAT_OVER_TEMP;
  } else {
    _status &= ~STATUS::STAT_OVER_TEMP;
  }

  if (cell_temperature < cell_threshold_under_temperature ) {
    _status |= STATUS::STAT_UNDER_TEMP;
  } else {
    _status &= ~STATUS::STAT_UNDER_TEMP;
  }

  if (_bypass_count_down > 0) {
    // Compare the real temperature against max setpoint - We want the PID to keep it stable
    uint16_t output = _pid->step(bypass_temperature_setpoint, _onboard_temperature);
    _hardware->pwmSet(output);

    Serial.printf(F("Board Bypass Duty: %d%\n"), output / 100);
    Serial.println();

    _bypass_count_down--;

    // Switch everything off for this cycle
    if (_bypass_count_down == 0 || _onboard_temperature > bypass_threshold_over_temperature) {

      _status &= ~STATUS::STAT_BYPASSING;

      if (_onboard_temperature > bypass_threshold_over_temperature) {
        _status |= STATUS::STAT_OVER_TEMP;
      } else {
        _status &= ~STATUS::STAT_OVER_TEMP;
      }

      _hardware->pwmEnd();

      // Switch Load off
      _hardware->dumpLoadOff();

      // On the next iteration of loop, don't sleep so we are forced to take another
      // cell voltage reading without the bypass being enabled, and we can then
      // evaludate if we need to stay in bypass mode, we do this a few times
      // as the cell has a tendancy to float back up in voltage once load resistor is removed
      _bypass_count_finished = _config.bypass_cooldown_count;
    }
  }

  if (_bypass_count_finished > 0) {
    _bypass_count_finished--;
  }

  // Our cell voltage is OVER the setpoint limit, start draining cell using load bypass resistor
  if (_cell_voltage > _config.bypass_threshold_voltage) {

    //We have just entered the bypass code
    if (!_bypass_count_down) {

      Serial.printf(F("Board Bypass STARTED!\n"));
      Serial.println();

      _status |= STATUS::STAT_BYPASSING;

      // Start timer2 with zero value
      _hardware->pwmBegin();

      //This controls how many cycles of loop() we make before re-checking the situation
      _bypass_count_down = _config.bypass_duration_count;
      _bypass_count_finished = 0;
    }
  }

  Serial.printf(F("_bypass_count_down: %d\n"), _bypass_count_down);
  Serial.printf(F("_bypass_count_finished: %d\n"), _bypass_count_finished);

  // Notify Hardware of the current state
  _hardware->display(_status);

  if (_serial_isr) {
    _serial_isr = false;

    // Wait for the frame to arrive - This depends on frame size!
    // Our Frame is 24 bytes + 2 Byte COBS Frame delimiter for a total 26 byte per frame
    delay(150);

    return false;
  }

  // We don't sleep if we are in bypass mode or just after completing bypass
  return !_bypass_count_down && !_bypass_count_finished;
}

void DiyBMS::onWakeup() {
  Serial.printf(F("We are awake....\n"));

  //We are awake....

  if (!(_watchdog_isr || _pci_isr)) {
    _serial_isr = true;
  }

  // Process current state
  if (_watchdog_isr || _serial_isr || _pci_isr) {

    if (_watchdog_isr) {
      Serial.printf(F("_watchdog_isr\n"));
    }

    if (_serial_isr) {
      Serial.printf(F("_serial_isr\n"));
    }

    if (_pci_isr) {
      Serial.printf(F("_pci_isr\n"));
    }

    Serial.flush();

    _status |= STATUS::STAT_AWAKED;

    // Notify State to hardware
    _hardware->display(_status);

    _watchdog_isr = false;
  }
}

uint8_t DiyBMS::getCellStatus() {
  return _status;
}

uint16_t DiyBMS::getCellVoltage() {
  return _cell_voltage;
}

int DiyBMS::getOnboardTemperature() {
  return _onboard_temperature;
}

int DiyBMS::getExternalTemperature() {
  return _external_temperature;
}

uint16_t DiyBMS::adcRawRequest(uint8_t channel, bool more) {
  _hardware->ADCBegin(channel, more);
}

void DiyBMS::setIdentify(bool value) {
  if (value) {
    _status |= STAT_IDENTIFY;
  } else {
    _status &= ~STAT_IDENTIFY;
  }

  _hardware->display(_status);
}

bool DiyBMS::getIdentify() {
  return (_status & STAT_IDENTIFY) > 0;
}

// Private STUFF
void DiyBMS::adcAcquireData(uint8_t channel, bool more) {
  uint16_t raw_data = adcRawRequest(channel, more);

  switch (channel) {
    case ADC_CAHNNEL::ADC_CELL_VOLTAGE:
      {
        _cell_voltage = (raw_data * _config.volt_reference) + _config.volt_offset;
        break;
      }
    case ADC_CAHNNEL::ADC_INTERNAL_TEMP:
      {
        _onboard_temperature = round(Steinhart::rawToCelcius(_config.t_internal_beta, raw_data) * 10);
        break;
      }
    case ADC_CAHNNEL::ADC_EXTERNAL_TEMP:
      {
        // Temperatures are stored in deciCelsius
        _external_temperature = round(Steinhart::rawToCelcius(_config.t_external_beta, raw_data) * 10);
        break;
      }

    default:
      //Avoid taking a reading if we get to here
      return;
  }
}

// Configuration Handler
bool DiyBMS::restoreFactoryConfig() {
  loadDefaultConfig();
  return storeConfig();
}

bool DiyBMS::storeConfig() {

  _hardware->display(_status | STAT_PROVISIONED);

  return _settings->writeConfig((byte *) &_config, sizeof(config_t));
}

bool DiyBMS::loadConfig() {
  return _settings->readConfig((byte *) &_config, sizeof(config_t));
}

// Private:
void DiyBMS::loadDefaultConfig() {
  // Communication Addresses
  _config.bank_id = 0xFF;
  _config.cell_id = 0xFF;

  // Stop if cell temperature over 70 degrees C
  _config.cell_threshold_over_temperature = 70;

  // Stop if cell temperature below 2 degrees C
  _config.cell_threshold_under_temperature = 2;

  // Self regulate bypass at 70 degrees C
  _config.bypass_threshold_over_temperature = 80;

  // Self regulate bypass at 70 degrees C
  _config.bypass_temperature_setpoint = 70;

  // Start bypass at 4100 mV
  _config.bypass_threshold_voltage = 4100;

  // Resistor network 2.6667 Ohm
  _config.bypass_resistance = 2667;

  // Wait 200 iterations before re-checking the situation
  _config.bypass_duration_count = 200;

  // Wait 200 iterations before next bypass mode
  _config.bypass_cooldown_count = 200;

  // About 2210mV seems about right
  _config.volt_offset = 2210;

  // 2mV per ADC resolution
  _config.volt_reference = 2; //2048mV/1024;

  // Resistance @ 25℃ = 47k, B Constant 4150, 0.20mA max current
  //Using https://www.thinksrs.com/downloads/programs/therm%20calc/ntccalibrator/ntccalculator.html

  //4150 = B constant (25-50℃)
  _config.t_internal_beta = 4150;
  //4150 = B constant (25-50℃)
  _config.t_external_beta = 4150;
}
