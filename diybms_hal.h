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

    virtual void flushSerial();

    virtual void enableSerialTX();
    virtual void disableSerialTX();

    virtual void watchdogOn();
    virtual void watchdogReset();
    virtual void watchdogOff();

    virtual uint16_t ADCBegin(uint8_t channel, bool more);
    //virtual uint16_t ADCRead();

    virtual void timer2Begin();
    virtual void timer2End();
    virtual void timer2Set(uint16_t value);

    virtual void sleep();
};

#endif
