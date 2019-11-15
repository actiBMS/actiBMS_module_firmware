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

  HARDWARE ABSTRACTION CODE FOR ATTINY841 & DIYBMS v5!

  PIN MAPPINGS
  Diagram
  https://github.com/SpenceKonde/ATTinyCore/blob/master/avr/extras/ATtiny_x41.md

  PA1 = PIN 12 - SERIAL TRANSMIT (TXD0)
  PA2 = PIN 11 - SERIAL RECEIVE (RXD0)

  PA3 = PIN 10 - DUMP LOAD ENABLE        / ARDUINO PIN 7/A3 / TOCC2
  PA4 = PIN 9  - ACTIVE BALANCE ENABLE   / ARDUINO PIN 6/A4
  PA5 = PIN 8  - ACTIVE BALANCE STATUS   / ARDUINO PIN 5/A4
  PA6 = PIN 7  - STATUS LED              / ARDUINO PIN 4/A6
  PA7 = PIN 6  - 2.048V REFERENCE ENABLE / ARDUINO PIN 3/A7

  PB0 = PIN 2  - ON BOARD TEMP sensor    / ADC11 / ARDUINO PIN 0/A11
  PB1 = PIN 3  - VOLTAGE reading         / ADC10 / ARDUINO PIN 1/A10
  PB2 = PIN 5  - REMOTE TEMP sensor      / ADC8  / ARDUINO PIN 2/A8

  ATTiny841 data sheet
  http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-8495-8-bit-AVR-Microcontrollers-ATtiny441-ATtiny841_Datasheet.pdf

*/


/*
  enum STATUS_COLOR : uint8_t {
  LED_GREEN = 0x1,
  LED_RED = 0x1,
  LED_BLUE = 0x1,
  };
*/

#ifndef _DIYBMS_ATTINY841_H // include guard
#define _DIYBMS_ATTINY841_H

//Show error is not targeting ATTINY841

#if !(defined(__AVR_ATtiny841__))
#error Written for ATTINY841 chip
#endif


#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>


#include <tinyNeoPixel.h>


#include "diybms_hal.h"

#ifdef DIYBMS_DEBUG

#define DEBUG_PRINT(str) \
  Serial1.print(__LINE__); \
  Serial1.print(' '); \
  Serial1.println(str);
#else
#define DEBUG_PRINT(str);
#endif

class DiyBMSATTiny841 : public BMSHal {
  public:
    DiyBMSATTiny841() {
    }

    void begin();

    void greenLedOn();
    void greenLedOff();

    void dumpLoadOn();
    void dumpLoadOff();

    void activeBalanceOn();
    void activeBalanceOff();

    void referenceVoltageOn();
    void referenceVoltageOff();

    /*
        void enableSerial0();
        void disableSerial0();
    */

    void flushSerial();

    void enableSerialTX();
    void disableSerialTX();

    void watchdogOn();
    void watchdogReset();
    void watchdogOff();

    uint16_t ADCBegin(uint8_t channel, bool more);
    //uint16_t ADCRead();

    void timer2Begin();
    void timer2End();
    void timer2Set(uint16_t value);

    void sleep();

    void enableSFD();

  private:
    tinyNeoPixel* pixels;

    /*
      void enableSerial1();
      void disableSerial1();
    */

    // PIN CHANGE INTERRUPT
    void enablePCI();
    void disablePCI();
};

#endif
