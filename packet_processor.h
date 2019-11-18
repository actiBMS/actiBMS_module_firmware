#ifndef _DIYBMS_PACKET_H // include guard
#define _DIYBMS_PACKET_H

#include <PacketSerial.h>

#include "diybms_hal.h"
#include "diybms_core.h"
#include "crc16.h"

#define DATA_LEN_MAX 12

#define ADDRESS_BROADCAST 0x8000
#define ADDRESS_MODULE 0x00FF
#define ADDRESS_BANK 0x0F00

#define SEQUENCE_REPLY 0x80

struct packet_t {
  uint16_t address;
  uint16_t sequence;
  uint8_t command;
  uint8_t stats;
  uint16_t data[DATA_LEN_MAX];
  uint16_t crc;
}  __attribute__((packed));

enum COMMAND : uint8_t
{
  CMD_PING = 0x00,
  CMD_IDENTIFY = 0x01,
  CMD_VOLTAGE = 0x02,
  CMD_TEMPERATURE = 0x03,
  CMD_BAD_PACKET = 0x04,
  CMD_SETTINGS_READ = 0x06,
  CMD_SETTINGS_WRITE = 0x07,
  CMD_IDENTITY_SET = 0x08
};

class PacketProcessor {
  public:
    PacketProcessor(DiyBMS * bms_core, BMSHal * hardware) {
      _hardware = hardware;
      _bms_core = bms_core;

      _module_bank = _bms_core->_config.bank_id;
      _module_address = _bms_core->_config.cell_id;
    }

    ~PacketProcessor() {}

    void processPacketFromSender(const PacketSerial_ < COBS, 0, 48 >& sender, const uint8_t* buffer, size_t size);

  private:
    BMSHal* _hardware;
    DiyBMS* _bms_core;

    struct packet_t _buffer;

    volatile uint8_t _module_address = 0xFF;
    volatile uint8_t _module_bank = 0xFF;
    volatile uint16_t _bad_packets = 0;

  private:
    bool isPacketForMe();
    bool processPacket();

    bool processConfigLoad();
    bool processConfigSave();
};

#endif
