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

#define DIYBMS_DEBUG
#define DIYBMS_OLD_PROTOCOL

// TODO - Enable CPU SPEED ERROR
/*
  #if !(F_CPU == 8000000)
  #error Processor speed should be 8 Mhz internal
  #endif
*/

#include <PacketSerial.h>

#include "diybms_core.h"
#include "diybms_arduino.h"

#ifdef DIYBMS_OLD_PROTOCOL
#include "packet_processor_cmp.h"
#else
#include "packet_processor.h"
#endif

// 48 byte buffer. It store up to two frames
PacketSerial_<COBS, 0, 64> packetSerial;

DiyBMSArduino hardware;
DiyBMS bmsCore(&hardware);

#ifdef DIYBMS_OLD_PROTOCOL
LegacyPacketProcessor packetHandler(&bmsCore, &hardware);
#else
PacketProcessor packetHandler(&bmsCore, &hardware);
#endif

ISR(WDT_vect) {
  //This is the watchdog timer - something went wrong and no activity recieved in a while
  bmsCore.isrWatchdog();
}

ISR(ADC_vect) {
  // NOP
}

#if defined(__AVR_ATmega168PB__) || defined(__AVR_ATmega328PB__)
ISR(USART0_START_vect) {
  bmsCore.isrSerialRX();
}
#endif

ISR (PCINT0_vect)
{
  bmsCore.isrPCI();
}

ISR (PCINT1_vect)
{
  bmsCore.isrPCI();
}

ISR (PCINT2_vect)
{
  bmsCore.isrPCI();
}

void setup() {
  hardware.begin();

  packetSerial.setStream(&Serial);
  packetSerial.setPacketHandler([](const uint8_t* buffer, size_t size) {
    packetHandler.processPacketFromSender(packetSerial, buffer, size);
  });

  if (!bmsCore.begin()) {
    hardware.display(LED_FAULT);
    while (true);
  }


         /*float current = (516 - rawcurr) * 45 / 1023;
          Serial.println(current, 3);*/
}

void loop() {
  hardware.watchdogReset();

  if (bmsCore.update()) {
    Serial.printf(F("We can sleep!\n"));
    Serial.println();

    // Program stops here until woken by watchdog or pin change interrupt
    hardware.sleep();

    bmsCore.onWakeup();
  }

  packetSerial.update();
}
