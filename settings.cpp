#include "settings.h"


Settings::Settings(uint16_t start_address) {
  Settings(start_address, EEPROM.length());
}

Settings::Settings(uint16_t start_address, size_t max_length) {
  // EEPROM Start Address
  _start_address = start_address;

  // Max bytes to write into EEPROM
  if (max_length > EEPROM.length()) {
    max_length = EEPROM.length();
  }
  _max_length = max_length;
}

bool Settings::writeConfig(byte * settings, size_t length) {
  uint16_t address;
  uint16_t checksum;

  if (length > _max_length) {
    return false;
  }

  address = _start_address;
  for (size_t i = 0; i < length; i++) {
    EEPROM.update( address, settings[i]);
    address++;
  }

  //Generate and save the checksum for the setting data block
  checksum = CRC16::CalculateArray(settings, length);
  EEPROM.put(_start_address + length, checksum);

  return true;
}

bool Settings::readConfig(byte * settings, size_t length) {
  uint16_t address;
  uint16_t checksum, existingChecksum;

  if (length > _max_length) {
    return false;
  }
  
  for (size_t i = 0 , address = _start_address; i < length; i++, address++) {
    settings[i] = EEPROM.read(address);
  }
  EEPROM.get(_start_address + length, existingChecksum);

  // Calculate the checksum
  checksum = CRC16::CalculateArray(settings, length);

  return checksum == existingChecksum;
}
