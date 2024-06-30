

/*

  batee.com 90-96 Analog Cal IC Tester v2.2.1 Firmware

  by Bryan A. "CrazyUncleBurton" Thompson

  Last Updated 06/30/2024

*/


// Includes
#include <Arduino.h>
#include <M5Core2.h> // Board Support File for M5Stack Core2
#include <Wire.h>  // For external ADC and temperature sensors on I2C bus
#include "Free_Fonts.h"  // Include the header file attached to this sketch


// Project Specific Pinouts

#define RELAY1_CONTROL G19       // G19 
#define RELAY1_CONTROL G25       // G25 
#define RELAY1_CONTROL G26       // G26


// for MCP9802 Temperature Sensor
#define Temperature_Sensor_Address 0x4D   //I2C address of MCP9802


float get_temperature()
{
  int TempByte1, TempByte2;
  float TempF=0;

  // MCP9802 Read temperature 
  Wire.beginTransmission(Temperature_Sensor_Address); // I see this on the capture.
  Wire.write((byte)0x00); // Address of temperature register - I see this on the capture
  Wire.endTransmission();

  // https://github.com/Koepel/How-to-use-the-Arduino-Wire-library/wiki/Common-mistakes
  Wire.requestFrom(Temperature_Sensor_Address, 2); // Now read two bytes of temperature data

  if (Wire.available()) {
    TempByte1 = Wire.read(); // MSB  Sign/64C/32C/16C/8C/4C/2C/1C
    TempByte2 = Wire.read(); // LSB  0.5C, 0.25C, 0.125C, 0.0625C, 0, 0, 0, 0
    unsigned int Temperature = ((TempByte1 << 8) | TempByte2);
    Temperature = Temperature >> 4;
    float TempC = 1.0*Temperature*0.0625;
    TempF = TempC*1.8+32.0;
  } else {
    // Must not have detected a sensor
  }
  Wire.endTransmission();

  return TempF;
  // ADC can be read 860 times/sec.  Temp Sensor can be read every 240mS.  So let's slow things down.
  // Probably it just takes ~240ms to converge, so we ought to be able to read it as fast as we want, it will just take a while for it to converge
}

// for ADS1115 ADC
#define ADS1115_U5 0x48
#define ADS1115_U6 0x4a
