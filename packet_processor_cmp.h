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

#ifndef _DIYBMS_PACKET_COMPAT_H
#define _DIYBMS_PACKET_COMPAT_H

#include <PacketSerial.h>

#include "diybms_hal.h"
#include "diybms_core.h"
#include "crc16.h"

#define DATA_LEN_MAX 16

#define ADDRESS_BROADCAST 0x80
#define ADDRESS_MODULE 0x0F
#define ADDRESS_BANK 0x70

#define SEQUENCE_REPLY 0x80

struct packet_t {
  uint8_t address;
  uint8_t command;
  uint16_t sequence;
  uint16_t data[DATA_LEN_MAX];
  uint16_t crc;
}  __attribute__((packed));

typedef union
{
  float number;
  uint8_t bytes[4];
  uint16_t word[2];
} FLOATUNION_t;

enum COMMAND : uint8_t
{
  CMD_BANK_IDENTITY_SET = 0x00,
  CMD_VOLTAGE_STATUS = 0x01,
  CMD_IDENTIFY = 0x02,
  CMD_TEMPERATURE = 0x03,
  CMD_BAD_PACKET = 0x04,
  CMD_SETTINGS_READ = 0x05,
  CMD_SETTINGS_WRITE = 0x06
};

class LegacyPacketProcessor {
  public:
    LegacyPacketProcessor(DiyBMS * bms_core, BMSHal * hardware) {
      _hardware = hardware;
      _bms_core = bms_core;

      _module_bank = _bms_core->_config.bank_id;
      _module_address = _bms_core->_config.cell_id;
    }

    ~LegacyPacketProcessor() {}

    void processPacketFromSender(const PacketSerial_ < COBS, 0, 64 >& sender, const uint8_t* buffer, size_t size);

  private:
    BMSHal* _hardware;
    DiyBMS* _bms_core;

    struct packet_t _buffer;

    volatile uint8_t _module_address = 0xFF;
    volatile uint8_t _module_bank = 0xFF;
    volatile uint16_t _bad_packets = 0;

  private:
    uint8_t _identify_module;

    bool isPacketForMe();
    bool processPacket();

    bool processConfigLoad();
    bool processConfigSave();

    void incrementPacketAddress();
    uint8_t temperatureToByte(float celcius);
};

#endif
