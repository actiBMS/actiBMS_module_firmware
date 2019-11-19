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

#include "packet_processor.h"


//Run when a new packet is received over serial
void PacketProcessor::processPacketFromSender(const PacketSerial_ < COBS, 0, 64 >& sender, const uint8_t* buffer, size_t size) {

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
        _buffer.sequence |= SEQUENCE_REPLY;

        // TODO - Set STATUS byte

        //Calculate new checksum over whole buffer
        _buffer.crc =  CRC16::CalculateArray((uint8_t*)&_buffer, sizeof(packet_t) - 2);

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
bool PacketProcessor::isPacketForMe() {
  uint8_t address, bank;

  address = _buffer.address & ADDRESS_BANK;
  bank = _buffer.address & ADDRESS_BANK >> 8;

  // Is a reply from someone else, ignore it!
  if (_buffer.sequence & SEQUENCE_REPLY)
    return false;

  if (_buffer.address & ADDRESS_BROADCAST) {
    // Modules can be grouped together in banks
    // only allow processing of packets in the correct bank or bank is not set
    return (bank == 0 || bank == _module_bank);
  }

  //Is this packet addressed directly to me?
  if (address == _module_address)
    return true;

  return false;
}


// Process the request in the received packet
bool PacketProcessor::processPacket() {

  switch (_buffer.command) {

    case COMMAND::CMD_PING:
      {
        // Nothing to do!
        break;
      }

    case COMMAND::CMD_IDENTIFY:
      {
        // Identify module - Turn on LED?
        _bms_core->setIdentify(_buffer.data[0]);
        break;
      }

    case COMMAND::CMD_IDENTITY_SET:
      {
        // Update Packet Processor Address
        _module_bank = _buffer.data[0] >> 8;
        _module_address = _buffer.data[0] & 0xFF;

        // Update Configuration & Store it

        _buffer.data[0] = _bms_core->_config.bank_id;
        _buffer.data[1] = _bms_core->_config.cell_id;

        _bms_core->_config.bank_id = _module_bank;
        _bms_core->_config.cell_id = _module_address;

        _bms_core->storeConfig();

        break;
      }

    case COMMAND::CMD_VOLTAGE:
      {
        //Read voltage of VCC
        _buffer.data[0] = _bms_core->getCellVoltage();

        break;
      }

    case COMMAND::CMD_TEMPERATURE:
      {
        _buffer.data[0] = _bms_core->getOnboardTemperature();
        _buffer.data[1] = _bms_core->getExternalTemperature();

        break;
      }

    case COMMAND::CMD_BAD_PACKET:
      {
        //Report number of bad packets
        _buffer.data[0] = _bad_packets;

        break;
      }

    case COMMAND::CMD_SETTINGS_READ:
      {
        //Report settings/configuration

        volatile struct config_t* conf = &(_bms_core->_config);

        // Over Temperature Threshold
        _buffer.data[0] = conf->cell_threshold_over_temperature;

        // Under Temperature Threshold
        _buffer.data[1] = conf->cell_threshold_under_temperature;

        // Bypass Over Temperature Threshold
        _buffer.data[2] = conf->bypass_threshold_over_temperature;

        // // Bypass Temperature Setpoint
        _buffer.data[3] = conf->bypass_temperature_setpoint;

        // Bypass Voltage Threshold (millivolt)
        _buffer.data[4] = conf->bypass_threshold_voltage;

        // Resistance of bypass load (milliohm) normally
        _buffer.data[5] = conf->bypass_resistance;

        // Voltage Calibration (millivolt)
        _buffer.data[6] = conf->volt_offset;

        // Reference voltage (millivolt) normally 2mV
        _buffer.data[7] = conf->volt_reference;

        // Internal Thermistor Beta Coeff
        _buffer.data[8] = conf->t_internal_beta;

        // External Thermistor Beta Coeff
        _buffer.data[9] = conf->t_external_beta;

        // # of iterations in bypass mode before another check
        _buffer.data[10] = _bms_core->_config.bypass_duration_count;

        // # of iterations to wait for before next bypass
        _buffer.data[11] = _bms_core->_config.bypass_cooldown_count;

        break;
      }

    case COMMAND::CMD_SETTINGS_WRITE:
      {

        volatile struct config_t* conf = &(_bms_core->_config);

        // Cell Over Temperature Threshold
        conf->cell_threshold_over_temperature = _buffer.data[0];

        // Cell Under Temperature Threshold
        conf->cell_threshold_under_temperature = _buffer.data[1];

        // Bypass Over Temperature Threshold
        conf->bypass_threshold_over_temperature = _buffer.data[2];

        // Bypass Temperature Setpoint
        conf->bypass_temperature_setpoint = _buffer.data[3];

        // Bypass Voltage Threshold (millivolt)
        conf->bypass_threshold_voltage = _buffer.data[4];

        // Resistance of bypass load (milliohm) normally
        conf->bypass_resistance = _buffer.data[5];

        // Voltage Calibration (millivolt)
        conf->volt_offset = _buffer.data[6];

        // Reference voltage (millivolt) normally 2mV
        conf->volt_reference = _buffer.data[7];

        // Internal Thermistor Beta Coeff
        conf->t_internal_beta = _buffer.data[8];

        // External Thermistor Beta Coeff
        conf->t_external_beta = _buffer.data[9];

        // # of iterations in bypass mode before another check
        _bms_core->_config.bypass_duration_count = _buffer.data[10];

        // # of iterations to wait for before next bypass
        _bms_core->_config.bypass_cooldown_count = _buffer.data[11];

        //Save settings
        _bms_core->storeConfig();

        break;
      }

    default:
      return false;
  }

  return true;
}
