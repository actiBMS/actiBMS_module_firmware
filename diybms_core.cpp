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

void DiyBMS::update() {

  //This loop runs around 3 times per second when the module is in bypass
  _hardware->watchdogReset();

  /*if (PP.identifyModule > 0) {
    hardware.GreenLedOn();
    PP.identifyModule--;

    if (PP.identifyModule == 0) {
      hardware.GreenLedOff();
    }
    }*/

  /*if (!PP.WeAreInBypass && bypassHasJustFinished == 0) {
    //We don't sleep if we are in bypass mode or just after completing bypass
    hardware.EnableStartFrameDetection();

    //Program stops here until woken by watchdog or pin change interrupt
    hardware.Sleep();
    }*/

  //We are awake....

  /*if (wdt_triggered) {
    //Flash green LED twice after a watchdog wake up
    hardware.double_tap_green_led();
    }*/

  //We always take a voltage and temperature reading on every loop cycle to check if we need to go into bypass
  //this is also triggered by the watchdog should comms fail or the module is running standalone
  //Probably over kill to do it this frequently
  /*hardware.ReferenceVoltageOn();

    //allow 2.048V to stabalize
    delay(10);

    PP.TakeAnAnalogueReading(ADC_CELL_VOLTAGE);
    //Internal temperature
    PP.TakeAnAnalogueReading(ADC_INTERNAL_TEMP);
    //External temperature
    PP.TakeAnAnalogueReading(ADC_EXTERNAL_TEMP);

    hardware.ReferenceVoltageOff();

    if (PP.BypassCheck()) {
    //Our cell voltage is OVER the setpoint limit, start draining cell using load bypass resistor

    if (!PP.WeAreInBypass) {
      //We have just entered the bypass code

      //The TIMER2 can vary between 0 and 10,000
      myPID.setOutputRange(0, 10000);

      //Start timer2 with zero value
      hardware.StartTimer2();

      PP.WeAreInBypass = true;

      //This controls how many cycles of loop() we make before re-checking the situation
      bypassCountDown = 200;
    }
    }

    if (bypassCountDown > 0) {
    //Compare the real temperature against max setpoint
    //We want the PID to keep at this temperature
    //int setpoint = ;
    //int feedback =
    uint16_t output = myPID.step(myConfig.BypassOverTempShutdown, PP.InternalTemperature());

    hardware.SetTimer2Value(output);

    bypassCountDown--;

    if (bypassCountDown == 0) {
      //Switch everything off for this cycle

      PP.WeAreInBypass = false;

      //myPID.clear();
      hardware.StopTimer2();

      //switch off
      hardware.DumpLoadOff();

      //On the next iteration of loop, don't sleep so we are forced to take another
      //cell voltage reading without the bypass being enabled, and we can then
      //evaludate if we need to stay in bypass mode, we do this a few times
      //as the cell has a tendancy to float back up in voltage once load resistor is removed
      bypassHasJustFinished = 200;
    }
    }

    /*if (wdt_triggered) {
    //We got here because the watchdog (after 8 seconds) went off - we didnt receive a packet of data
    wdt_triggered = false;
    } else {
    //Loop here processing any packets then go back to sleep

    //NOTE this loop size is dependant on the size of the packet buffer (34 bytes)
    //     too small a loop will prevent anything being processed as we go back to Sleep
    //     before packet is received correctly
    for (size_t i = 0; i < 15; i++) {
      //Allow data to be received in buffer
      delay(10);

      // Call update to receive, decode and process incoming packets.
      myPacketSerial.update();
    }
    }
  */
  /*
    if (bypassHasJustFinished > 0) {
      bypassHasJustFinished--;
    }*/
}

/*
   //Read cell voltage and return millivolt reading (16 bit unsigned)
  uint16_t PacketProcessor::CellVoltage() {
  //TODO: Get rid of the need for float variables?
  float v = ((float) raw_adc_voltage * _config->mVPerADC) * _config->Calibration;

  return (uint16_t) v;
  }

  //Returns the last RAW ADC value 0-1023
  uint16_t PacketProcessor::RawADCValue() {
  return raw_adc_voltage;
  }
*/





/*uint16_t PacketProcessor::TemperatureMeasurement() {
  return (TemperatureToByte(Steinhart::ThermistorToCelcius(_config->Internal_BCoefficient, onboard_temperature)) << 8) +
         TemperatureToByte(Steinhart::ThermistorToCelcius(_config->External_BCoefficient, external_temperature));
  }*/


/*


  //Start an ADC reading via Interrupt
  void PacketProcessor::TakeAnAnalogueReading(uint8_t mode) {
  adcmode = mode;

  switch (adcmode) {
    case ADC_CELL_VOLTAGE:
      {
        _hardware-> SelectCellVoltageChannel();
        break;
      }
    case ADC_INTERNAL_TEMP:
      {
        _hardware-> SelectInternalTemperatureChannel();
        break;
      }
    case ADC_EXTERNAL_TEMP:
      {
        _hardware-> SelectExternalTemperatureChannel();
        break;
      }
    default:
      //Avoid taking a reading if we get to here
      return;
  }

  _hardware-> BeginADCReading();
  }

*/
/*

   //Records an ADC reading after the interrupt has finished
  void PacketProcessor::ADCReading(uint16_t value) {
  switch (adcmode) {
   case ADC_CELL_VOLTAGE:
     {
       //UpdateRingBuffer(value);
       raw_adc_voltage = value;
       break;
     }
   case ADC_INTERNAL_TEMP:
     {
       onboard_temperature = value;
       break;
     }
   case ADC_EXTERNAL_TEMP:
     {
       external_temperature = value;
       break;
     }
  }
  }

*/



/*
  // Returns an integer byte indicating the internal thermistor temperature in degrees C
  // uses basic B Coefficient Steinhart calculaton to give rough approximation in temperature
  /*int8_t PacketProcessor::InternalTemperature() {
  return round(Steinhart::ThermistorToCelcius(_config->  Internal_BCoefficient, onboard_temperature));
  }*/

//Returns TRUE if the internal thermistor is hotter than the required setting
bool DiyBMS::checkBypassOverheat() {
  return (onboard_temperature > _config.bypass_threshold_temperature);
}

//Returns TRUE if the cell voltage is greater than the required setting
bool DiyBMS::checkBypass() {
  return (cell_voltage > _config.bypass_threshold_voltage);
}

// Configuration Handler

uint8_t DiyBMS::setAddrBank(uint8_t value, bool store) {
  uint8_t old_value = _config.bank_id;
  _config.bank_id = value;

  return old_value;
}

uint8_t DiyBMS::getAddrBank() {
  return _config.bank_id;
}

uint8_t DiyBMS::setAddrCell(uint8_t value, bool store) {
  uint8_t old_value = _config.cell_id;
  _config.cell_id = value;

  return old_value;
}

uint8_t DiyBMS::getAddrCell() {
  return _config.cell_id;
}

uint8_t DiyBMS::setBypassTemp(uint8_t value, bool store) {
  uint8_t old_value = _config.bypass_threshold_temperature;
  _config.bypass_threshold_temperature = value;

  return old_value;
}

uint8_t DiyBMS::getBypassTemp() {
  return _config.bypass_threshold_temperature;
}

uint16_t DiyBMS::setBypassVoltage( uint16_t value, bool store) {
  uint16_t old_value = _config.bypass_threshold_voltage;
  _config.bypass_threshold_voltage = value;

  return old_value;
}

uint16_t DiyBMS::getBypassVoltage() {
  return _config.bypass_threshold_voltage;
}

uint16_t DiyBMS::setTempIntBeta( uint16_t value, bool store) {
  uint16_t old_value = _config.t_internal_beta;
  _config.t_internal_beta = value;

  return old_value;
}

uint16_t DiyBMS::getTempIntBeta() {
  return _config.t_internal_beta;
}

uint16_t DiyBMS::setTempExtBeta( uint16_t value, bool store) {
  uint16_t old_value = _config.t_external_beta;
  _config.t_external_beta = value;

  return old_value;
}

uint16_t DiyBMS::getTempExtBeta() {
  return _config.t_external_beta;
}

float DiyBMS::setBypassResistance(float value, bool store) {
  float old_value = _config.bypass_resistance;
  _config.bypass_resistance = value;

  return old_value;
}

float DiyBMS::getBypassResistance() {
  return _config.bypass_resistance;
}

float DiyBMS::setVoltOffset(float value, bool store) {
  float old_value = _config.volt_offset;
  _config.volt_offset = value;

  return old_value;
}

float DiyBMS::getVoltOffset() {
  return _config.volt_offset;
}

float DiyBMS::setVoltReference(float value, bool store) {
  float old_value = _config.volt_reference;
  _config.volt_reference = value;

  return old_value;
}

float DiyBMS::getVoltReference() {
  return _config.volt_reference;
}

bool DiyBMS::restoreFactory() {
  loadDefault();
  return store();
}

bool DiyBMS::store() {
  return _settings->writeConfig((byte *) &_config, sizeof(config_t));
}

bool DiyBMS::load() {
  return _settings->readConfig((byte *) &_config, sizeof(config_t));
}

// Private:
void DiyBMS::loadDefault() {
  // Communication Addresses
  _config.bank_id = 0xFF;
  _config.cell_id = 0xFF;

  // Bypass
  // Stop running bypass if temperature over 70 degrees C
  _config.bypass_threshold_temperature = 70;

  //Start bypass at 4.1 volt
  _config.bypass_threshold_voltage = 4100;

  // Resistor network
  _config.bypass_resistance =  2.6667;

  //About 2.2100 seems about right
  _config.volt_offset = 2.21000;

  //2mV per ADC resolution
  _config.volt_reference = 2.0; //2048.0/1024.0;

  // Resistance @ 25℃ = 47k, B Constant 4150, 0.20mA max current
  //Using https://www.thinksrs.com/downloads/programs/therm%20calc/ntccalibrator/ntccalculator.html

  //4150 = B constant (25-50℃)
  _config.t_internal_beta = 4150;
  //4150 = B constant (25-50℃)
  _config.t_external_beta = 4150;
}
