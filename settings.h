/*

  EEPROM Settings storage with checksum
  (c)2019 Stuart Pittaway

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

#ifndef Settings_H // include guard
#define Settings_H

#include <Arduino.h>
#include "crc16.h"
#include <EEPROM.h>

class Settings {
  public:
    Settings(uint16_t start_address);
    Settings(uint16_t start_address, size_t max_length);

    bool writeConfig(byte* settings, size_t length);
    bool readConfig(byte* settings, size_t length);

  private:
    size_t _max_length;    
    uint16_t _start_address;
};
#endif
