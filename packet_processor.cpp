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
void PacketProcessor::processPacketFromSender(const PacketSerial_ < COBS, 0, 48 >& sender, const uint8_t* buffer, size_t size) {

  if (size == sizeof(packet_t)) {
    uint16_t calculated_crc;

    //Copy to our buffer (probably a better way to share memory than this)
    memcpy(&_buffer, buffer, sizeof(packet_t));

    //Calculate the CRC and compare to received
    calculated_crc = CRC16::CalculateArray((char*)&_buffer, sizeof(packet_t) - 2);

    if (calculated_crc == _buffer.crc) {
      byte wakeup = 0x00;

      //It's a good packet
      if (isPacketForMe() && processPacket()) {

        // TODO - Set flag to indicate we processed packet
        _buffer.command = _buffer.command | B10000000;

        //Calculate new checksum over whole buffer
        _buffer.crc =  CRC16::CalculateArray((char*)&_buffer, sizeof(packet_t) - 2);


        //_hardware.GreenLedOn();

        _hardware->enableSerial0TX();

        // Wake up the connected cell module from sleep
        Serial.write((byte) 0);
        sender.send(&wakeup, 1);
        delay(10);

        //Send the packet (even if it was invalid so controller can count crc errors)
        sender.send((byte * ) & _buffer, sizeof(packet_t));

        _hardware->flushSerial0();
        _hardware->disableSerial0TX();
      }

      //_hardware.GreenLedOff();
    }
  }

  //We need to do something here, the packet received was not correct
  _bad_packets++;
}

//Determines if a received packet of instruction is for this module
//based on broadcast flag, bank id and module address
bool PacketProcessor::isPacketForMe() {
  uint8_t address;


  //Modules can be grouped together in banks - only allow processing of packets in the correct bank
  if (((_buffer.address & 0x30) >> 4) != _module_address) return false;

  //Broadcast for my bank?
  if ((_buffer.address & 0x80) == 0x80) {
    //If we receive a broadcast message whatever address is received is my unique module address
    //so store it for later use
    _module_address = _buffer.address & 0x0F;

    return true;
  }

  //Is this packet addressed directly to me?
  if ((_buffer.address & 0x0F) == _module_address && _module_address != 0xFF) return true;

  return false;
}


// Process the request in the received packet
// command byte
// RRRR CCCC
// X    = 1 bit indicate if packet processed
// R    = 3 bits reserved not used
// C    = 4 bits command (16 possible commands)
bool PacketProcessor::processPacket() {

  switch (_buffer.command) {

    case COMMAND::IDENTITY_SET:
      {
        //Set this modules bank address and store in EEPROM
        //_config->mybank = _buffer.moduledata[_module_address] & 0x3;

        //Save settings
        //Settings::writeConfig((char*)_config, sizeof(ModuleConfig));

        //Indicate we processed this packet
        //_buffer.moduledata[_module_address] = 0xFFFF;
        break;
      }

    case COMMAND::VOLTAGE:
      {
        //Read voltage of VCC
        //Maximum voltage 8191mV
        //_buffer.moduledata[_module_address] = CellVoltage() & 0x1FFF;

        //3 top bits
        //X = In bypass
        //Y = Bypass over temperature
        //Z = Not used

        /*if (BypassOverheatCheck()) {
          _buffer.moduledata[_module_address] = _buffer.moduledata[_module_address] | 0x4000;
          }

          if (WeAreInBypass) {
          _buffer.moduledata[_module_address] = _buffer.moduledata[_module_address] | 0x8000;
          }*/

        break;
      }

    case COMMAND::IDENTIFY:
      {
        //identify module
        //Indicate that we received and did something
        //_buffer.moduledata[_module_address] = 0xFFFF;

        //For the next 10 receied packets - keep the LEDs lit up
        //identifyModule = 10;
        break;
      }

    case COMMAND::TEMPERATURE:
      {
        //Read the last 2 temperature values recorded by the ADC (both internal and external)
        //_buffer.moduledata[_module_address] = TemperatureMeasurement();
        break;
      }

    case COMMAND::BAD_PACKET:
      {
        //Report number of bad packets
        //_buffer.moduledata[_module_address] = _bad_packets;
        break;
      }

    case COMMAND::SETTINGS_READ:
      {
        //Report settings/configuration

        /*FLOATUNION_t myFloat;
          myFloat.number = _config->LoadResistance;
          _buffer.moduledata[0] = myFloat.word[0];
          _buffer.moduledata[1] = myFloat.word[1];

          myFloat.number = _config->Calibration;
          _buffer.moduledata[2] = myFloat.word[0];
          _buffer.moduledata[3] = myFloat.word[1];

          myFloat.number = _config->mVPerADC;
          _buffer.moduledata[4] = myFloat.word[0];
          _buffer.moduledata[5] = myFloat.word[1];

          _buffer.moduledata[6] = _config->BypassOverTempShutdown;
          _buffer.moduledata[7] = _config->BypassThresholdmV;
          _buffer.moduledata[8] = _config->Internal_BCoefficient;
          _buffer.moduledata[9] = _config->External_BCoefficient;*/

        break;
      }

    case COMMAND::SETTINGS_WRITE:
      {
        FLOATUNION_t myFloat;

        /*myFloat.word[0] = _buffer.moduledata[0];
          myFloat.word[1] = _buffer.moduledata[1];
          if (myFloat.number < 0xFFFF) {
          _config->LoadResistance = myFloat.number;
          }

          myFloat.word[0] = _buffer.moduledata[2];
          myFloat.word[1] = _buffer.moduledata[3];

          if (myFloat.number < 0xFFFF) {
          _config->Calibration = myFloat.number;
          }

          myFloat.word[0] = _buffer.moduledata[4];
          myFloat.word[1] = _buffer.moduledata[5];
          if (myFloat.number < 0xFFFF) {
          _config->mVPerADC = myFloat.number;
          }

          if (_buffer.moduledata[6] != 0xFF) {
          _config->BypassOverTempShutdown = _buffer.moduledata[6];
          }

          if (_buffer.moduledata[7] != 0xFFFF) {
          _config->BypassThresholdmV = _buffer.moduledata[7];
          }
          if (_buffer.moduledata[8] != 0xFFFF) {
          _config->Internal_BCoefficient = _buffer.moduledata[8];
          }

          if (_buffer.moduledata[9] != 0xFFFF) {
          _config->External_BCoefficient = _buffer.moduledata[9];
          }*/

        //Save settings
        //Settings::writeConfig((char * ) _config, sizeof(ModuleConfig));

        break;
      }

    default:
      return false;
  }

  return true;
}
