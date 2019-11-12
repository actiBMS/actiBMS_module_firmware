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

* Non-Commercial — You may not use the material for commercial purposes.
* Attribution — You must give appropriate credit, provide a link to the license, and indicate if changes were made.
  You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use.
* ShareAlike — If you remix, transform, or build upon the material, you must distribute your
  contributions under the same license as the original.
* No additional restrictions — You may not apply legal terms or technological measures
  that legally restrict others from doing anything the license permits.
*/

//#define DIYBMS_DEBUG

#if !(F_CPU == 8000000)
#error Processor speed should be 8 Mhz internal
#endif

#include <PacketSerial.h>

#include "diybms_core.h"
#include "diybms_attiny841.h"
#include "packet_processor.h"

// 48 byte buffer. It store up to two frames
PacketSerial_<COBS, 0, 48> packetSerial;

//Default values which get overwritten by EEPROM on power up
DiyBMSATTiny841 hardware;
DiyBMS bmsCore(&hardware);
PacketProcessor packetHandler(&bmsCore, &hardware);

ISR(WDT_vect) {
  //This is the watchdog timer - something went wrong and no activity recieved in a while
  bmsCore.incrementWatchdogCounter();
}

ISR(ADC_vect) {
  // NOP
}

ISR(USART0_START_vect) {
  // NOP
}

void setup() {
  hardware.begin();

  packetSerial.setStream(&Serial);
  packetSerial.setPacketHandler([](const uint8_t* buffer, size_t size) {
       packetHandler.processPacketFromSender(packetSerial, buffer, size);
  });

  #ifdef DIYBMS_DEBUG
  if (myPID.err()) {
    Serial1.println("There is a configuration error!");
    for (;;) {}
  }
  #endif
}

void loop() {
  bmsCore.update();
  packetSerial.update();
}
