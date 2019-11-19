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

#include "packet_processor_cmp.h"

//Run when a new packet is received over serial
void LegacyPacketProcessor::processPacketFromSender(const PacketSerial_ < COBS, 0, 64 >& sender, const uint8_t* buffer, size_t size) {

  if (size == sizeof(packet_t)) {
    uint16_t calculated_crc;

    //Copy to our buffer (probably a better way to share memory than this)
    memcpy(&_buffer, buffer, sizeof(packet_t));

    //Calculate the CRC and compare to received
    calculated_crc = CRC16::CalculateArray((uint8_t*)&_buffer, sizeof(packet_t) - 2);

    if (calculated_crc == _buffer.crc) {

      //It's a good packet
      if (isPacketForMe() && processPacket()) {

        // Set flag to indicate we processed packet
        _buffer.command |= SEQUENCE_REPLY;

        // Calculate new checksum over whole buffer
        _buffer.crc = CRC16::CalculateArray((uint8_t*)&_buffer, sizeof(packet_t) - 2);

        _hardware->enableSerialTX();

        //Send the packet (even if it was invalid so controller can count crc errors)
        sender.send((byte * ) & _buffer, sizeof(packet_t));

        _hardware->flushSerial();
        _hardware->disableSerialTX();
      }

      return;
    }
  }

  //We need to do something here, the packet received was not correct
  _bad_packets++;
}

//Determines if a received packet of instruction is for this module
//based on broadcast flag, bank id and module address
bool LegacyPacketProcessor::isPacketForMe() {
  uint8_t address, bank;

  address = _buffer.address & ADDRESS_MODULE;
  bank = _buffer.address & ADDRESS_BANK >> 4;

  // Modules can be grouped together in banks - only allow processing of packets in the correct bank
  if (bank != _module_bank) {
    return false;
  }

  // Broadcast for my bank?
  if (_buffer.address & ADDRESS_BROADCAST) {
    // If we receive a broadcast message whatever address is received is my
    // unique module address so store it for later use
    _module_address = address;

    // Update Configuration & Store it
    _bms_core->_config.cell_id = address;
    _bms_core->storeConfig();

    // Ensure the next module has a higher address
    incrementPacketAddress();

    return true;
  }

  //Is this packet addressed directly to me?
  if (address == _module_address && _module_address != 0xFF) {
    return true;
  }

  return false;
}


// Process the request in the received packet
bool LegacyPacketProcessor::processPacket() {

  switch (_buffer.command) {

    case COMMAND::CMD_BANK_IDENTITY_SET:
      {
        // Update Packet Processor Address
        _module_bank = _buffer.data[_module_address] & 0x3;

        // Update Configuration & Store it
        _buffer.data[_module_address] = 0xFFFF;

        _bms_core->_config.bank_id = _module_bank;

        _bms_core->storeConfig();

        break;
      }

    case COMMAND::CMD_VOLTAGE_STATUS:
      {
        uint8_t status = _bms_core->getCellStatus();

        //Read voltage of VCC
        _buffer.data[_module_address] = _bms_core->getCellVoltage() & 0x1FFF;

        if (status & STAT_OVER_TEMP) {
          _buffer.data[_module_address] |=  0x4000;
        }

        if (status & STAT_BYPASSING) {
          _buffer.data[_module_address] |= 0x8000;
        }

        break;
      }

    case COMMAND::CMD_IDENTIFY:
      {
        // Identify module - Turn on LED?
        _bms_core->setIdentify(true);
        _buffer.data[_module_address] = 0xFFFF;

        // For the next 10 receied packets - keep the LEDs lit up
        _identify_module = 10;

        break;
      }


    case COMMAND::CMD_TEMPERATURE:
      {
        _buffer.data[_module_address] = temperatureToByte(_bms_core->getOnboardTemperature() / 10) << 8
                                        | temperatureToByte(_bms_core->getExternalTemperature() / 10);

        break;
      }

    case COMMAND::CMD_BAD_PACKET:
      {
        //Report number of bad packets
        _buffer.data[_module_address] = _bad_packets;

        break;
      }

    case COMMAND::CMD_SETTINGS_READ:
      {
        // Report settings/configuration

        volatile struct config_t* conf = &(_bms_core->_config);
        FLOATUNION_t myFloat;

        // Resistance of bypass load (milliohm) normally
        myFloat.number = ((float) conf->bypass_resistance) / 1000.0F;
        _buffer.data[0] = myFloat.word[0];
        _buffer.data[1] = myFloat.word[1];


        // Voltage Calibration (millivolt)
        myFloat.number = ((float) conf->volt_offset) / 1000.0F;
        _buffer.data[2] = myFloat.word[0];
        _buffer.data[3] = myFloat.word[1];

        // Reference voltage (millivolt) normally 2mV
        myFloat.number = (float) conf->volt_reference;
        _buffer.data[4] = myFloat.word[0];
        _buffer.data[5] = myFloat.word[1];

        // Bypass Temperature Setpoint
        _buffer.data[6] = conf->bypass_temperature_setpoint;

        // Bypass Voltage Threshold (millivolt)
        _buffer.data[7] = conf->bypass_threshold_voltage;

        // Internal Thermistor Beta Coeff
        _buffer.data[8] = conf->t_internal_beta;

        // External Thermistor Beta Coeff
        _buffer.data[9] = conf->t_external_beta;

        break;
      }

    case COMMAND::CMD_SETTINGS_WRITE:
      {

        // Update settings/configuration

        volatile struct config_t* conf = &(_bms_core->_config);
        FLOATUNION_t myFloat;

        // Resistance of bypass load (milliohm) normally
        myFloat.word[0] = _buffer.data[0];
        myFloat.word[1] = _buffer.data[1];
        if (myFloat.number < 0xFFFF) {
          conf->bypass_resistance = round(myFloat.number * 1000.0F);
        }

        // Voltage Calibration (millivolt)
        myFloat.word[0] = _buffer.data[2];
        myFloat.word[1] = _buffer.data[3];

        if (myFloat.number < 0xFFFF) {
          conf->volt_offset = round(myFloat.number * 1000.0F);
        }

        // Reference voltage (millivolt) normally 2mV
        myFloat.word[0] = _buffer.data[4];
        myFloat.word[1] = _buffer.data[5];
        if (myFloat.number < 0xFFFF) {
          conf->volt_reference = myFloat.number;
        }

        // Bypass Temperature Setpoint
        if (_buffer.data[6] != 0xFF) {
          conf->bypass_temperature_setpoint = _buffer.data[6];
        }

        // Bypass Voltage Threshold (millivolt)
        if (_buffer.data[7] != 0xFFFF) {
          conf->bypass_threshold_voltage = _buffer.data[7];
        }

        // Internal Thermistor Beta Coeff
        if (_buffer.data[8] != 0xFFFF) {
          conf->t_internal_beta = _buffer.data[8];
        }

        // External Thermistor Beta Coeff
        if (_buffer.data[9] != 0xFFFF) {
          conf->t_external_beta = _buffer.data[9];
        }

        //Save settings
        _bms_core->storeConfig();

        break;
      }

    default:
      return false;
  }

  return true;
}

// Increases the incoming packets address before sending to the next module
void LegacyPacketProcessor::incrementPacketAddress() {
  _buffer.address = (_buffer.address & 0xF0) + ((_buffer.address & 0x0F) + 1);
}

// This function reduces the scale of temperatures from float types to a single byte (unsigned)
// We have an artifical floor at 40oC, anything below +40 is considered negative (below freezing)
// Gives range of -40 to +216 degrees C
uint8_t LegacyPacketProcessor::temperatureToByte(float celcius) {
  celcius += 40;

  //Set the limits
  if (celcius < 0) celcius = 0;
  if (celcius > 255) celcius = 255;

  return (uint8_t) celcius;
}
