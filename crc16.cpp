#include "crc16.h"

// Calculate XMODEM 16 crc code on data array
uint16_t CRC16::CalculateArray(uint8_t data[], uint16_t length)
{
  //Calculates XMODEM CRC16
  uint16_t polynomial = 0x1021;
  uint16_t xorIn = 0x0000;
  uint16_t xorOut = 0x0000;
  uint16_t msbMask = 0x8000;
  uint16_t mask = 0xffff;

  uint16_t crc = xorIn;

  int j;
  uint8_t c;
  uint16_t bit;

  if (length == 0) return crc;

  for (uint16_t i = 0; i < length; i++)
  {
    c = data[i];

    j = 0x80;

    while (j > 0)
    {
      bit = (uint16_t)(crc & msbMask);
      crc <<= 1;

      if ((c & j) != 0)
      {
        bit = (uint16_t)(bit ^ msbMask);
      }

      if (bit != 0)
      {
        crc ^= polynomial;
      }

      j >>= 1;
    }
  }

  return crc;
}
