#ifndef _DIYBMS_PACKET_H // include guard
#define _DIYBMS_PACKET_H

#include <PacketSerial.h>

#include "diybms_hal.h"
#include "diybms_core.h"
#include "crc16.h"

#define DATA_LEN_MAX 16

struct packet_t {
  uint16_t address;
  uint16_t sequence;
  uint8_t command;
  uint8_t stats;
  uint8_t data[DATA_LEN_MAX];
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
  PING = 0x00,
  IDENTIFY = 0x01,
  VOLTAGE = 0x02,
  TEMPERATURE = 0x03,
  BAD_PACKET = 0x04,
  SETTINGS_READ = 0x06,
  SETTINGS_WRITE = 0x07,
  IDENTITY_SET = 0x08
};

enum CONFIG : uint8_t
{
  ADDR_BANK,
  ADDR_CELL,
  BYPASS_THR_TEMP,
  BYPASS_THR_VOLT,
  BYPASS_RES,
  VOLT_OFFSET,
  VOLT_REFERENCE,
  TINT_BETA,
  TEXT_BETA
};

class PacketProcessor {
  public:
    PacketProcessor(DiyBMS * bms_core, BMSHal * hardware) {
      _hardware = hardware;
      _bms_core = bms_core;

      _module_bank = _bms_core->getAddrBank();
      _module_address = _bms_core->getAddrCell();
    }

    ~PacketProcessor() {}

    void processPacketFromSender(const PacketSerial_ < COBS, 0, 48 >& sender, const uint8_t* buffer, size_t size);

  private:
    BMSHal * _hardware;
    DiyBMS *_bms_core;

    volatile uint8_t _module_address = 0xFF;
    volatile uint8_t _module_bank = 0xFF;
    volatile uint16_t _bad_packets = 0;

    struct packet_t _buffer;

    bool isPacketForMe();
    bool processPacket();
};

#endif
