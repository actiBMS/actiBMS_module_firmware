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
  PD2 = PIN 32 -                           / ARDUINO PIN 10
  PD3 = PIN 1  - NOT USED                  / ARDUINO PIN 11
  PD4 = PIN 2  - NOT USED                  / ARDUINO PIN 12
  PD5 = PIN 9  - NOT USED - TP 1           / ARDUINO PIN 13
  PD6 = PIN 10 - NOT USED - TP 2           / ARDUINO PIN 20
  PD7 = PIN 11 - NOT USED - TP 3           / ARDUINO PIN 21

  ATmega168p data sheet
  http://ww1.microchip.com/downloads/en/DeviceDoc/ATmega48_88_168_megaAVR-Data-Sheet-40002074.pdf
*/

#include "diybms_arduino.h"

#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

#include <Arduino.h>

void DiyBMSArduino::begin() {
  //Must be first line of code
  watchdogOff();

  // Setup Power Domains
  power_spi_disable();
  power_twi_disable();
  power_adc_disable();
  power_timer1_disable();
  power_timer2_disable();


  //Set up data handler
  Serial.begin(9600, SERIAL_8N1);

#ifndef DIYBMS_DEBUG
  //disableSerialTX();
#endif

#if defined(__AVR_ATmega168PB__) || defined(__AVR_ATmega328PB__)
  enableSerialFrameDetection();
#endif

  pixels = new Adafruit_NeoPixel(1, 10, NEO_GRB + NEO_KHZ800);  

  // MUCR - Enable Pull-Ups
  MCUCR &= ~(1 << PUD);

  // DDRB – Port B Data Direction Register
  // When DDxn is set, the pin Pxn is configured as an output.
  // When DDxn is cleared, the pin is configured as an input
  portMode(GPIO_PORTB, (1 << DDB1) | (1 << DDB2));

  //DDRC – Port C Data Direction Register
  portMode(GPIO_PORTC, (1 << DDC1) | (1 << DDC5));

  //DDRD – Port D Data Direction Register
  portMode(GPIO_PORTD, 0);

  // Disable pullups
  portWrite(GPIO_PORTB, 0x00);
  portWrite(GPIO_PORTC, 0x00);
  portWrite(GPIO_PORTD, 0x00);

  // Enable pullup on PC0 - ACTIVE BALANCE STATUS
  portWrite(GPIO_PORTC, (1 << DDC0));

  enablePCI();

  //Set pins to initial state
  dumpLoadOff();
  referenceVoltageOff();

  // Status LED off
  pixels->begin();
  pixels->setBrightness(LED_BRIGHTNESS);
  
  pixels->clear();
  pixels->show();

  watchdogOn();
}

// Watchdog
void DiyBMSArduino::watchdogOn() {
  // Disable interrupts
  cli();

  // Reset watchdog
  wdt_reset();

  //Setup a watchdog timer for 8 seconds
  MCUSR = 0;

  //Enable watchdog (to reset)
  WDTCSR |= (1 << WDE);

  // Enable BITs on WDTCSR
  WDTCSR |= (1 << WDCE) | (1 << WDE);

  // We ONLY INTERRUPT the chip after 8 seconds of sleeping (not reboot!)
  WDTCSR = (1 << WDP3) | (WDTO_8S & 0x27);
  WDTCSR |= (1 << WDIE);

  //Enable global interrupts
  sei();

  wdt_reset();
}

void DiyBMSArduino::watchdogOff() {
  wdt_disable();
}

void DiyBMSArduino::watchdogReset() {
  wdt_reset();
}

void DiyBMSArduino::watchdogReboot() {
  WDTCSR |= (1 << WDCE) | (1 << WDE);
}

void DiyBMSArduino::pwmBegin() {
  // Turn on power domain
  power_timer1_enable();
  delay(10);

  Timer1.initialize(8); // 8 us = 125khz
  Timer1.pwm(9, 0);
}

void DiyBMSArduino::pwmEnd() {
  Timer1.disablePwm(9);
  Timer1.stop();

  power_timer1_disable();
}

void DiyBMSArduino::pwmSet(uint16_t value) {
  Timer1.setPwmDuty(9, value);
}

void DiyBMSArduino::disableSerialTX() {
  UCSR0B &= ~_BV(TXEN0); //disable transmitter (saves 6mA)
}

void DiyBMSArduino::enableSerialTX() {
  UCSR0B |= (1 << TXEN0); // enable transmitter

  // Wake up the connected cell module from sleep
  Serial.write((byte) 0);
  delay(10);
}

void DiyBMSArduino::flushSerial() {
  Serial.flush();
}

#if defined(__AVR_ATmega168PB__) || defined(__AVR_ATmega328PB__)
void DiyBMSArduino::enableSerialFrameDetection() {
  cli();

  // Enable Start Frame Detection
  UCSR0D = (1 << RXSIE0) | (1 << SFDE0);

  sei();
}
#endif

// Switch bypass on. Connected to Pin 13, PB1
void DiyBMSArduino::dumpLoadOn() {
  PORTB |= (1 << PORTB1);
}

// Switch bypass off. Connected to Pin 13, PB1
void DiyBMSArduino::dumpLoadOff() {
  PORTB &= ~(1 << PORTB1);
}

// Switch active balancer on. Connected to Pin 23, PC0
void DiyBMSArduino::activeBalanceOn() {
  PORTC |= (1 << PORTC5);
}

// Switch active balancer off. Connected to Pin 23, PC0
void DiyBMSArduino::activeBalanceOff() {
  PORTC &= ~(1 << PORTC5);
}

// Switch 2.048V regulator on. Connected to Pin 24, PC1
void DiyBMSArduino::referenceVoltageOn() {
  PORTC |= (1 << PORTC1);
}

// Switch 2.048V regulator off. Connected to Pin 24, PC1
void DiyBMSArduino::referenceVoltageOff() {
  PORTC &= ~(1 << PORTC1);
}

// Pin Change IRQ
// TODO - Fire pin change interrupt on RXD0 changing state
void DiyBMSArduino::enablePCI() {
  cli();

  // INT0 is not used, it might be used to detect SOF on 196P/328P
  //EICRA |= (1 << ISC01);
  //EICRA &= ~(1 << ISC00);
  //EIMSK |= (1 << INT0);

  // Pin Change Interrupt Enable all ports & clear flags
  PCICR |= (1 << PCIE2) | (1 << PCIE1) | (1 << PCIE0);
  PCIFR |= (1 << PCIF2) | (1 << PCIF1) | (1 << PCIF0);

  //PCMSK0 – Pin Change Mask
  //PCMSK0 |= (1 << PCINT5) | (1 << PCINT4) | (1 << PCINT3) | (1 << PCINT0);
  PCMSK1 |= (1 << PCINT8);
  //PCMSK2 |= (1 << PCINT23) | (1 << PCINT22) | (1 << PCINT21);

  sei();
}

void DiyBMSArduino::disablePCI() {
  // Pin Change Interrupt Disable all ports
  PCICR &= ~((1 << PCIE2) | (1 << PCIE1) | (1 << PCIE0));
}

// TODO - Take a look to 16.9 Section!
uint16_t DiyBMSArduino::ADCBegin(uint8_t channel, bool more) {
  uint8_t low ;

  // ADCs take time reset WDT
  watchdogReset();

  referenceVoltageOn();

  // Turn on Power for ADC
  power_adc_enable();

  //allow 2.048V and ADC to stabalize
  delay(10);

  switch (channel) {
    case ADC_CELL_VOLTAGE:
      {
        /* 0011 */
        ADMUX = (1 << MUX1) | (1 << MUX0);
        break;
      }
    case ADC_INTERNAL_TEMP:
      {
        /* 0100 */
        ADMUX = (1 << MUX2);
        break;
      }
    case ADC_EXTERNAL_TEMP:
      {
        /* 0010 */
        ADMUX = (1 << MUX1);
        break;
      }
    default:
      //Avoid taking a reading if we get to here
      return 0;
  }

  //ADMUXB – ADC Multiplexer Selection Register
  //Select external AREF pin (internal reference turned off)
  ADMUX &= ~((1 << REFS1) | (1 << REFS0));

  //ADCSRA – ADC Control and Status Register A
  //Consider ADC sleep conversion mode?
  //prescaler of 64 = 8MHz/64 = 125KHz.
  ADCSRA |= _BV(ADPS2) | _BV(ADPS1); // | _BV(ADPS0);

  //adc_enable();
  //Bit 4 – ADIF: ADC Interrupt Flag
  //Bit 7 – ADEN: ADC Enable
  ADCSRA |= _BV(ADEN) | _BV(ADIF); // enable ADC, turn off any pending interrupt

  // wait for ADC to settle
  // The ADC must be enabled during the settling time.
  // ADC requires a settling time of 1ms before measurements are stable
  delay(2);

  // Disable Interrupts
  cli();

  // Setup sleep during ADC sample
  set_sleep_mode(SLEEP_MODE_ADC);
  sleep_enable();

  // Start the conversion
  ADCSRA |= _BV(ADSC) | _BV(ADIE);

  // Enable Interrupts
  sei();

  // Go to sleep while ADC is running
  sleep_cpu();

  // Snoring can be heard at this point....

  // Now we are awaken - An interrut has occurred
  sleep_disable();

  // Awake, reading should be done, better make sure maybe the timer interrupt fired
  while (bit_is_set(ADCSRA, ADSC)) {}

  if (!more) {
    //adc_disable
    ADCSRA &= (~(1 << ADEN));

    // Turn off Power for ADC
    power_adc_disable();

    // Turn off the Reference
    referenceVoltageOff();
  }

  low = ADCL;
  return (ADCH << 8) | low;
}

void DiyBMSArduino::display(uint8_t state) {
  uint32_t processed = last_color;

  // First quick/temp states
  if (state & LED_AWAKED) {
    pixels->setPixelColor(0, 0, 255, 0);
    pixels->show();
    delay(25);
    pixels->setPixelColor(0, last_color);
    pixels->show();
    delay(1);
    pixels->setPixelColor(0, 0, 255, 0);
    pixels->show();
    delay(25);
  }

  if (state & LED_PROVISIONED) {
    pixels->setPixelColor(0, 0, 255, 0);
    pixels->show();
    delay(50);
  }

  // Low Priority to hi Priority
  if (state & LED_BALANCING) {
    processed = pixels->Color(0, 0, 0, 255);
  }

  if (state & LED_BYPASSING) {
    processed = pixels->Color(0, 255, 255, 0);
  }

  // TODO - Case BALANCING + BYPASSING

  if (state & LED_OVER_TEMP) {
    processed = pixels->Color(0, 255, 100, 0);
  }

  if (state & LED_UNDER_TEMP) {
    processed = pixels->Color(0, 0, 200, 255);
  }

  if (state & LED_FAULT) {
    processed = pixels->Color(0, 255, 0, 0);
  }

  if (state & LED_IDENTIFY) {
    processed = pixels->Color(0, 255, 255, 255);
    pixels->setBrightness(LED_MAX_BRIGHTNESS);
  } else {
    pixels->setBrightness(LED_BRIGHTNESS);
  }

  if (!state) {
    pixels->clear();
  }

  pixels->setPixelColor(0, processed);
  pixels->show();
}

void DiyBMSArduino::sleep() {

  // Clear Serial buffer before sleep
  Serial.flush();

  //For low power applications, before entering sleep, remember to turn off the ADC
  ADCSRA &= (~(1 << ADEN));

  power_spi_disable();
  power_twi_disable();
  power_adc_disable();

  power_timer0_disable();
  power_timer1_disable();
  power_timer2_disable();

  //Keep this alive
  //power_usart0_disable();

  sei();

#if defined(__AVR_ATmega168PB__) || defined(__AVR_ATmega328PB__)
  set_sleep_mode(SLEEP_MODE_PWD_DOWN);
#else
  set_sleep_mode(SLEEP_MODE_IDLE);
#endif

  sleep_enable();
  sleep_cpu();

  // Snoring can be heard at this point....

  sleep_disable();

  power_timer0_enable();
}
