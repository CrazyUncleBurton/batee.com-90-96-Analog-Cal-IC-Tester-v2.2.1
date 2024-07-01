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
#include <Adafruit_ADS1X15.h>

// Project Specific Pinouts
#define RELAY1_CONTROL G19  // G19 (Resistors R2 and R3) 
#define RELAY2_CONTROL G26  // G26 (Resistors R1 and R5)
#define RELAY3_CONTROL G25  // G25 (Resistors R4 and R6)

// I2C Addresses
#define ADS1115_U5 0x48 // I2C Address for ADS1115 ADC #1
#define ADS1115_U6 0x4a // I2C Address for ADS1115 ADC #2
#define Temperature_Sensor_Address 0x4D   //I2C address of MCP9802 Temperature Sensor


// Instantiations
Adafruit_ADS1115 ads;  /* U5 - Use this for the 16-bit version */
Adafruit_ADS1115 ads2;  /* U6 - Use this for the 16-bit version */


// Functions
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
    float TempC = 1.0 * Temperature * 0.0625;
    TempF = TempC * 1.8 + 32.0;
  } else {
    // Must not have detected a sensor
  }
  Wire.endTransmission();

  return TempF;
  // ADC can be read 860 times/sec.  Temp Sensor can be read every 240mS.  So let's slow things down.
  // Probably it just takes ~240ms to converge, so we ought to be able to read it as fast as we want, it will just take a while for it to converge
}



// Setup Runs Once

void setup() {
  
  // M5.begin Line from SodaSaver:  M5.begin(true,true,false,false,kMBusModeInput); //Init M5Core2- bool LCDEnable = true, bool SDEnable = true, bool SerialEnable = false, bool I2CEnable = false, AXP192 power mode OUTPUT to power ext hardwre
  M5.begin(true, true, true, false, kMBusModeInput); //Init M5Core2(Initialization of external I2C is also included).   LCD Enable, SDEnable, Serial Enable, I2C Enable, kMBusModeInput (kMBusModeInput tells Stack it is powered by external MBus 5V, kMBusModeOutput tells stack it is powered internally by USB or internal battery)
  delay(100);
  M5.Axp.SetBusPowerMode(1); // This allows the Stack to be powered from 5V Bus. CUB Added it when the Stack stopped booting from Bus +5V, but would still boot from USB.

  // Setup GPIO
  pinMode(RELAY1_CONTROL, OUTPUT); // Power to Test Resistors R2 and R3
  pinMode(RELAY2_CONTROL, OUTPUT); // Power to Test Resistors R1 and R5
  pinMode(RELAY3_CONTROL, OUTPUT); // Power to Test Resistors R4 and R6

  // Enable Internal I2C
  Wire.begin(21, 22); //Everything is on the M5Stack Internal I2C Bus
  delay(1000);
  Wire.setClock(400000UL); // 400kHz Internal Bus Speed

  // Begin U5 ADC
  ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  if (!ads.begin(ADS1115_U5)) {
    Serial.println("Failed to initialize U5 ADC.");
    while (1); // Halt and Catch Fire
  }

  // Begin U6 ADC
  ads2.setGain(GAIN_SIXTEEN);  // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  if (!ads.begin(ADS1115_U6)) {
    Serial.println("Failed to initialize U6 ADC.");
    while (1); // Halt and Catch Fire
  }

  // Print the header for a display screen
  M5.Lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  M5.Lcd.setFreeFont(FSB9);      // Select Free Serif 9 point font
  M5.Lcd.setTextDatum(MC_DATUM); // cursor is top left of text - not sure if this works with print
  M5.Lcd.setCursor(0, 30);       // Set initial position - might not be needed withdrawString? 
  M5.Lcd.drawString("batee.com Analog Cal IC Tester", 0, 30);
  M5.Lcd.drawString("Hardware: v2.2.1 Firmware v1.0.0", 0, 63);

  // Enable All Relays
  digitalWrite (RELAY1_CONTROL, HIGH); // Turn On Relay1 / Resistors R2 and R3
  digitalWrite (RELAY2_CONTROL, HIGH); // Turn On Relay2 / Resistors R1 and R5
  digitalWrite (RELAY3_CONTROL, HIGH); // Turn On Relay3 / Resistors R4 and R6

  // Delay for Warm-up
  M5.Lcd.drawString("Waiting for Warmup", 0, 96);
  delay(3000);

}


void loop() {
  while (1)
  {

    int16_t adc0, adc1, adc2, adc3, adc4, adc5, adc6, adc7, adc8;
    float U5_AIN0, U5_AIN1, U5_AIN2, U5_AIN3, U6_AIN0, U6_AIN1, U6_AIN2, U6_AIN3;
    float DUT_R1, DUT_R2, DUT_R3, DUT_R4, DUT_R5, DUT_R6; // Calculated value of the resistors under test
    char input_voltage_string[10];
    char test_voltage_string[10];
    char R1_voltage_string[10];
    char R2_voltage_string[10];
    char R3_voltage_string[10];
    char R4_voltage_string[10];
    char R5_voltage_string[10];
    char R6_voltage_string[10];


    // Measure Temperature and report
    // For some reason, the first reading is always excessively high.
    // Read the temperature twice to work around this problem.
    char temperature_string[10];
    float temperature = 74;
    temperature = get_temperature();
    if (temperature > 100) 
    {
      temperature = get_temperature();
    }

    // Measure 15V0 Input Voltage
    adc2 = ads.readADC_SingleEnded(2); // U5_AIN2 - 15V0 Power In Voltage
    U5_AIN2 = ads.computeVolts(adc2); // U5_AIN2 - 15V0 Power In Voltage

    // Measure 5V0 Test Voltage
    adc0 = ads.readADC_SingleEnded(0); // U5_AIN0 - 5V0 Test Voltage
    U5_AIN0 = ads.computeVolts(adc0); // U5_AIN0 - 5V0 Test Voltage

    // Report Test Conditions
    sprintf(temperature_string,"%2.1fF\n",temperature);
    sprintf(input_voltage_string,"%2.1fVolts\n",U5_AIN2); // If I put the space here I want, the unit reboots right after printing this --CUB
    sprintf(test_voltage_string,"%2.1fVolts\n",U5_AIN0); // If I put the space here I want, the unit reboots right after printing this --CUB
    M5.Lcd.clear();   // Clear the contents displayed on the screen
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setFreeFont(FSB9);      // Select Free Serif 9 point font
    M5.Lcd.setTextDatum(MC_DATUM); // cursor is top left of text - not sure if this works with print
    M5.Lcd.setCursor(0, 30);
    M5.Lcd.print("Vin: ");
    M5.Lcd.print(input_voltage_string);
    M5.Lcd.print("V  Vtest: ");
    M5.Lcd.print(test_voltage_string);
    M5.Lcd.print("V  Temp: ");
    M5.Lcd.print(temperature_string);
    M5.Lcd.println("degF");

    M5.Lcd.setFreeFont(FSB18);
    M5.Lcd.drawString(input_voltage_string, 160, 88);
    M5.Lcd.drawString(input_voltage_string, 160, 121);
    M5.Lcd.drawString(temperature_string, 160, 154);

    // Measure R4 / Should be 174K or 125K.  Test (top) resistor is 174K
    adc3 = ads.readADC_SingleEnded(3); // U5_AIN3 - DUT_R4
    U5_AIN3 = ads.computeVolts(adc3); // U5_AIN3 - DUT_R4
    DUT_R4 = ((U5_AIN0 * 174000 ) / (U5_AIN0 - U5_AIN3));

    // Measure R1 / 95K -ish  Adjustable - Test resistor is 100K
    adc6 = ads2.readADC_SingleEnded(2); // U6_AIN2 - DUT_R1
    U6_AIN2 = ads2.computeVolts(adc6); // U6_AIN2 - DUT_R1
    DUT_R1 = ((U5_AIN0 * 100000 ) / (U5_AIN0 - U6_AIN2));

    // Determine if this is 6K (174K) or 8K (125K).
    // This will tell us what value to set R1 to 
    // If U6_AIN2 is close to 0.5* U5_AIN0, it's a 6K. 
    // If U6_AIN2 is close to 0.41666667

    // Measure R2 / 4.02K
    adc4 = ads2.readADC_SingleEnded(0); // U6_AIN0 - DUT R2
    U6_AIN0 = ads2.computeVolts(adc4); // U6_AIN0 - DUT R2
    DUT_R2 = ((U5_AIN0 * 4020 ) / (U5_AIN0 - U6_AIN0));

    // Measure R3 / 2K
    adc5 = ads2.readADC_SingleEnded(1); // U6_AIN1 - DUT_R3
    U6_AIN1 = ads2.computeVolts(adc5); // U6_AIN1 - DUT_R3
    DUT_R3 = ((U5_AIN0 * 2000 ) / (U5_AIN0 - U6_AIN1));

    // Measure R5 / 4.53K
    adc7 = ads2.readADC_SingleEnded(3); // U6_AIN3 - DUT_R5
    U6_AIN3 = ads2.computeVolts(adc7); // U6_AIN3 - DUT_R5
    DUT_R5 = ((U5_AIN0 * 4530 ) / (U5_AIN0 - U6_AIN3));

    // Measure R6 / 3K
    adc1 = ads.readADC_SingleEnded(1); // U5_AIN1 - DUT_R6
    U5_AIN1 = ads.computeVolts(adc1); // U5_AIN1 - DUT_R6
    DUT_R6 = ((U5_AIN0 * 100000 ) / (U5_AIN0 - U5_AIN1));

    // Report pass/fail results and CCW/CW results

    

    // Wait for screen to clear, then Clear Screen and loop
    delay(50); // We can't loop more than 128 samples per second (maybe 32 x 4 channels per ADC = 31.25mS)
    M5.Lcd.clear();   // Clear the contents displayed on the screen
    M5.Lcd.setCursor(0, 30);

  }
}
