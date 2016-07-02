#include "Si7021Support.h"
#include "UniGlobals.h"

Si7021::Si7021()
{
}

void Si7021::begin()
{
  Wire.begin();
}
const HumidityAnswer& Si7021::read()
{
  dt.Humidity = NO_TEMPERATURE_DATA;
  dt.HumidityDecimal = 0;
  dt.Temperature = NO_TEMPERATURE_DATA;
  dt.TemperatureDecimal = 0;
  
  uint16_t humidity = 0;
  uint16_t temp = 0;
  bool crcOk = false;
  static byte buffer[] = {0, 0, 0};
 
  
  Wire.beginTransmission(Si7021Address);
  SI7021_WRITE(Si7021_E5);
  Wire.endTransmission();
  
  Wire.requestFrom(Si7021Address, 3);
  
  
  if(Wire.available() >= 3)
  {
  
    
    buffer[0] = SI7021_READ();
    buffer[1] = SI7021_READ();
    buffer[2] = SI7021_READ();
    
    humidity =  (buffer[0]<<8) | buffer[1];

    uint8_t calcCRC = 0;  

    for (uint8_t i = 0; i < 2; i++)
    {
      calcCRC ^= buffer[i];
  
        for (uint8_t j = 8; j > 0; j--)
        {
           if (calcCRC & 0x80)
              calcCRC = (calcCRC << 1) ^ 0x131;
          else
              calcCRC = (calcCRC << 1);
        } // for

    } // for

    crcOk = (calcCRC == buffer[2]);

  } // if
  
  Wire.beginTransmission(Si7021Address);
  SI7021_WRITE(Si7021_E0);
  Wire.endTransmission();
  
  Wire.requestFrom(Si7021Address, 2);
  
  if(Wire.available() >= 2)
  {
    buffer[0] = SI7021_READ(); 
    buffer[1] = SI7021_READ();
    
    temp =  (buffer[0]<<8) | buffer[1];
  }
  
  if(temp != 0 && humidity != 0)
  {    
    int iTmp = ((125.0*humidity)/65536 - 6)*100;
    
    dt.Humidity = iTmp/100;
    dt.HumidityDecimal = iTmp%100;
    
    iTmp = (175.72*temp/65536 - 46.85)*100;
    
    dt.Temperature = iTmp/100;
    dt.TemperatureDecimal = iTmp%100;
    
  }
  
  return dt;
}
