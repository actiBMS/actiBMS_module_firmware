/*
  ____  ____  _  _  ____  __  __  ___
  (  _ \(_  _)( \/ )(  _ \(  \/  )/ __)
  )(_) )_)(_  \  /  ) _ < )    ( \__ \
  (____/(____) (__) (____/(_/\/\_)(___/

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

/*
  HARDWARE ABSTRACTION CODE
*/

#ifndef _DIYBMS_HAL_H
#define _DIYBMS_HAL_H

#include <Arduino.h>

enum LED_STATUS : uint8_t
{
  LED_PROVISIONED = 0x01,
  LED_IDENTIFY = 0x02,
  LED_BALANCING = 0x04,
  LED_BYPASSING = 0x08,
  LED_AWAKED = 0x10,
  LED_OVER_TEMP = 0x20,
  LED_UNDER_TEMP = 0x40,
  LED_FAULT = 0x80
};

enum ADC_CAHNNEL : uint8_t {
  ADC_CELL_VOLTAGE,
  ADC_INTERNAL_TEMP,
  ADC_EXTERNAL_TEMP
};

/*
  This class wraps the hardware pins of DIYBMS away from the core logic/code
  if you are porting to another chipset, clone this class and modify it.
*/
class BMSHal {
  public:

    virtual void begin();

    virtual void dumpLoadOn();
    virtual void dumpLoadOff();

    virtual void activeBalanceOn();
    virtual void activeBalanceOff();

    virtual void referenceVoltageOn();
    virtual void referenceVoltageOff();

    virtual void enableSerialTX();
    virtual void disableSerialTX();
    virtual void flushSerial();

    virtual void watchdogOn();
    virtual void watchdogOff();
    virtual void watchdogReset();    
    virtual void watchdogReboot();

    virtual uint16_t ADCBegin(uint8_t channel, bool more);

    virtual void pwmBegin();
    virtual void pwmEnd();
    virtual void pwmSet(uint16_t value);

    virtual void display(uint8_t state);
    virtual void sleep();
};

#endif
