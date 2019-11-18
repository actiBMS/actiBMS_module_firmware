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
  PB0 = PIN 12 - NOT USED - TP 4           / ARDUINO PIN 8
  PB1 = PIN 13 - DUMP LOAD ENABLE          / ARDUINO PIN 9 / OC1A
  PB2 = PIN 14 - STATUS LED                / ARDUINO PIN 10
  PB3 = PIN 15 - NOT USED - MOSI           / ARDUINO PIN 11
  PB4 = PIN 16 - NOT USED - MISO           / ARDUINO PIN 12
  PB5 = PIN 17 - NOT USED - SCK            / ARDUINO PIN 13
  PB6 = PIN 7  - NOT USED                  / ARDUINO PIN 20
  PB7 = PIN 8  - NOT USED                  / ARDUINO PIN 21

  PC0 = PIN 23 - ACTIVE BALANCE STATUS     / ARDUINO PIN 14/A0
  PC1 = PIN 24 - 2.048V REFERENCE ENABLE   / ARDUINO PIN 15/A1
  PC2 = PIN 25 - REMOTE TEMP sensor        / ARDUINO PIN 16/A2
  PC3 = PIN 26 - VOLTAGE divider           / ARDUINO PIN 17/A3
  PC4 = PIN 27 - ONBOARD TEMP sensor       / ARDUINO PIN 18/A4
  PC5 = PIN 28 - ACTIVE BALANCE ENABLE     / ARDUINO PIN 19/A5
  PC6 = PIN 29 - RESET                     / ARDUINO PIN 22

  PD0 = PIN 30 - SERIAL RECEIVE (RXD0)     / ARDUINO PIN 8
  PD1 = PIN 31 - SERIAL TRANSMIT (TXD0)    / ARDUINO PIN 9
  PD2 = PIN 32 - NOT USED                  / ARDUINO PIN 10
  PD3 = PIN 1  - NOT USED                  / ARDUINO PIN 11
  PD4 = PIN 2  - NOT USED                  / ARDUINO PIN 12
  PD5 = PIN 9  - NOT USED - TP 1           / ARDUINO PIN 13
  PD6 = PIN 10 - NOT USED - TP 2           / ARDUINO PIN 20
  PD7 = PIN 11 - NOT USED - TP 3           / ARDUINO PIN 21

  ATmega168p data sheet
  http://ww1.microchip.com/downloads/en/DeviceDoc/ATmega48_88_168_megaAVR-Data-Sheet-40002074.pdf
*/


/*
  enum STATUS_COLOR : uint8_t {
  LED_GREEN = 0x1,
  LED_RED = 0x1,
  LED_BLUE = 0x1,
  };
*/

#ifndef _DIYBMS_ATMEGA_H // include guard
#define _DIYBMS_ATMEGA_H

// TODO - Show error is not targeting ATMEEGA168P
/*#if !(defined(__AVR_ATmega328P__))
  #error Written for ATMEEGA168P/PB chip
  #endif
*/

#define GPIO_PORTA 0
#define GPIO_PORTB 1
#define GPIO_PORTC 2
#define GPIO_PORTD 3

#define LED_BRIGHTNESS 4
#define LED_MAX_BRIGHTNESS 128

#include "diybms_hal.h"

#include <Adafruit_NeoPixel.h>
#include <TimerOne.h>

class DiyBMSArduino : public BMSHal {
  public:
    DiyBMSArduino() {
    }

    void begin();
    
    void dumpLoadOn();
    void dumpLoadOff();

    void activeBalanceOn();
    void activeBalanceOff();

    void referenceVoltageOn();
    void referenceVoltageOff();

    void enableSerialTX();
    void disableSerialTX();
    void flushSerial();

    void watchdogOn();
    void watchdogOff();
    void watchdogReset();
    void watchdogReboot();

    uint16_t ADCBegin(uint8_t channel, bool more);

    void pwmBegin();
    void pwmEnd();
    void pwmSet(uint16_t value);

    void display(uint8_t state);
    void sleep();

  private:
    Adafruit_NeoPixel* pixels;
    uint32_t last_color;

// SERIAL START OF FRAME is available only on PB series
#if defined(__AVR_ATmega168PB__) || defined(__AVR_ATmega328PB__)    
    void enableSerialFrameDetection();
#endif

    // PIN CHANGE INTERRUPT
    void enablePCI();
    void disablePCI();
};

#endif
