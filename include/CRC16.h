#ifndef CRC16_H
#define CRC16_H

/*
**   CRC is a CRC16 value calculated over the preceding characters in the data message 
**   (from “/” to “!” using the polynomial: x16+x15+x2+1). 
**   CRC16 uses no XOR in, no XOR out and is computed with least significant bit first. 
**   The value is represented as 4 hexadecimal characters (MSB first).
*/

//#if defined(ARDUINO) && ARDUINO >= 100
//  #include "Arduino.h"
//#else
//  #include "WProgram.h"
//#endif

unsigned int CRC16(unsigned int crc, unsigned char *buf, int len)
{
  for (int pos = 0; pos < len; pos++)
  {
    //Serial.print((char)buf[pos]);
    crc ^= (unsigned int)buf[pos];    // XOR byte into least sig. byte of crc

    for (int i = 8; i != 0; i--)      // Loop over each bit
    {
      if ((crc & 0x0001) != 0)        // If the LSB is set
      {
        crc >>= 1;                    // Shift right and XOR 0xA001
        crc ^= 0xA001;
      }
      else                            // Else LSB is not set
        crc >>= 1;                    // Just shift right
    }
  }

  return crc;
}
#endif
