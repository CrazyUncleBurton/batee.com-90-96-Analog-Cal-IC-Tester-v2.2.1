/*

  batee.com 90-96 Analog Cal IC Tester v2.2.1 Firmware

  by Bryan A. "CrazyUncleBurton" Thompson

  Last Updated 07/12/2024

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
  Wire.setClock(100000UL); // 400kHz Internal Bus Speed

  // Begin U5 ADC
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  if (!ads.begin(ADS1115_U5)) {
    Serial.println("Failed to initialize U5 ADC.");
    while (1); // Halt and Catch Fire
  }

  // Begin U6 ADC
  // ads2.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  ads2.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads2.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads2.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads2.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads2.setGain(GAIN_SIXTEEN);  // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  if (!ads2.begin(ADS1115_U6)) {
    Serial.println("Failed to initialize U6 ADC.");
    while (1); // Halt and Catch Fire
  }

  // Print the header for a display screen
  M5.Lcd.clear();
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setFreeFont(FSB12);      // Select Free Serif 9 point font
  M5.Lcd.setTextDatum(MC_DATUM); // cursor is top left of text - not sure if this works with print
  M5.Lcd.setCursor(0, 30);       // Set initial position - might not be needed withdrawString? 
  M5.Lcd.println("                  batee.com");
  M5.Lcd.println("         Analog Cal IC Tester");
  M5.Lcd.println("            Hardware: v2.2.1");
  M5.Lcd.println("            Firmware v1.0.0");
  M5.Lcd.println("         by CrazyUncleBurton");
  M5.Lcd.println(" ");
  M5.Lcd.println("        Waiting for Warmup...");

  // Enable All Relays
  digitalWrite (RELAY1_CONTROL, HIGH); // Turn On Relay1 / Resistors R2 and R3
  digitalWrite (RELAY2_CONTROL, HIGH); // Turn On Relay2 / Resistors R1 and R5
  digitalWrite (RELAY3_CONTROL, HIGH); // Turn On Relay3 / Resistors R4 and R6

  // Delay for Warm-up
  delay(1000);

}


void loop() {
  while (1)
  {

    int16_t adc0, adc1, adc2, adc3, adc4, adc5, adc6, adc7, adc8; // Actual count measured by the ADC
    float U5_AIN0, U5_AIN1, U5_AIN2, U5_AIN3, U6_AIN0, U6_AIN1, U6_AIN2, U6_AIN3; // Actual voltage measured by the ADC
    float VIN, VTEST; // Calculated values of measured voltages with voltage dividers factored in
    float R1_calculated, R2_calculated, R3_calculated, R4_calculated, R5_calculated, R6_calculated;  // Calculated value of the resistors under test
    char environment_string[128];
    float TEMP = 74;
    char TEMP_string[16];
    char R1_resistance_string[16];
    char R2_resistance_string[16];
    char R3_resistance_string[16];
    char R4_resistance_string[16];
    char R5_resistance_string[16];
    char R6_resistance_string[16];
    float R1_test_resistor = 96.000; // in kOhms as measured from ground to the DUT Socket pin 10
    float R2_test_resistor = 4.017; // in kOhms as measured from ground to the DUT Socket pin 13
    float R3_test_resistor = 2.001; // in kOhms as measured from ground to the DUT Socket pin 11
    float R4_test_resistor = 174.200; // in kOhms as measured from ground to the DUT Socket pin 0
    float R5_test_resistor = 4.518; // in kOhms as measured from ground to the DUT Socket pin 4
    float R6_test_resistor = 3.001; // in kOhms as measured from ground to the DUT Socket pin 7
    float R1_desired = 96.00; // in kOhms, this is the value we install on the ICs
    float R2_desired = 4.02; // in kOhms, this is the value we install on the ICs
    float R3_desired = 2.00; // in kOhms, this is the value we install on the ICs
    float R4_desired_1 = 174.00; // in kOhms, this is the value we install on the ICs
    float R4_desired_2 = 124.00; // in kOhms, this is the value we install on the ICs
    float R5_desired = 4.53; // in kOhms, this is the value we install on the ICs
    float R6_desired = 3.00; // in kOhms, this is the value we install on the ICs
    float VIN_divider = 6.00235386; // VIN = VIN_divider * U5_AIN2 - actual measured value
    float VTEST_divider = 2.0011928; // VTEST = VTEST_divider * U5_AIN0 - actual measured value
    int model = 0; // Which model did we detect?  6=6k, 8=8k, 0 = not close to either
    float model_value;

    // Measure 15V0 Input Voltage
    adc2 = ads.readADC_SingleEnded(2); // U5_AIN2 - 15V0 Power In Voltage
    U5_AIN2 = ads.computeVolts(adc2); // U5_AIN2 - 15V0 Power In Voltage
    // U5_AIN2 is the measured voltage.  We need to multiply by the voltage divider to recover the actual voltage
    VIN = U5_AIN2 * VIN_divider;

    // Measure 5V0 Test Voltage
    adc0 = ads.readADC_SingleEnded(0); // U5_AIN0 - 5V0 Test Voltage
    U5_AIN0 = ads.computeVolts(adc0); // U5_AIN0 - 5V0 Test Voltage
    VTEST = U5_AIN0 * VTEST_divider;

    // Measure TEMP
    // For some reason, the first reading is always excessively high.
    // Read the TEMP twice to work around this problem.
    TEMP = get_temperature();
    if (TEMP > 100) 
    {
      TEMP = get_temperature();
    }

    // Measure R1
    adc6 = ads2.readADC_SingleEnded(2); // U6_AIN2 - DUT_R1
    U6_AIN2 = ads2.computeVolts(adc6); // U6_AIN2 - DUT_R1
    R1_calculated = ((U6_AIN2 * R1_test_resistor ) / (VTEST - U6_AIN2)); // in kOhms

    // Measure R2
    adc4 = ads2.readADC_SingleEnded(0); // U6_AIN0 - DUT_R2
    U6_AIN0 = ads2.computeVolts(adc4); // U6_AIN0 - DUT_R2
    R2_calculated = ((U6_AIN0 * R2_test_resistor ) / (VTEST - U6_AIN0)); // in kOhms

    // Measure R3
    adc5 = ads2.readADC_SingleEnded(1); // U6_AIN1 - DUT_R3
    U6_AIN1 = ads2.computeVolts(adc5); // U6_AIN1 - DUT_R3
    R3_calculated = ((U6_AIN1 * R3_test_resistor ) / (VTEST - U6_AIN1)); // in kOhms

    // Measure R4
    adc3 = ads.readADC_SingleEnded(3); // U5_AIN3 - DUT_R4
    U5_AIN3 = ads.computeVolts(adc3); // U5_AIN3 - DUT_R4
    R4_calculated = ((U5_AIN3 * R4_test_resistor ) / (VTEST - U5_AIN3)); // in kOhms

    // Measure R5
    adc7 = ads2.readADC_SingleEnded(3); // U6_AIN3 - DUT_R5
    U6_AIN3 = ads2.computeVolts(adc7); // U6_AIN3 - DUT_R5
    R5_calculated = ((U6_AIN3 * R5_test_resistor ) / (VTEST - U6_AIN3)); // in kOhms

    // Measure R6
    adc1 = ads.readADC_SingleEnded(1); // U5_AIN1 - DUT_R6
    U5_AIN1 = ads.computeVolts(adc3); // U5_AIN1 - DUT_R6
    R6_calculated = ((U5_AIN1 * R6_test_resistor ) / (VTEST - U5_AIN1)); // in kOhms.  
    

    // Report Results
    M5.Lcd.clear();   // Clear the contents displayed on the screen
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setFreeFont(FS12);      // Select Free Serif 9 point font
    M5.Lcd.setTextDatum(MC_DATUM); // cursor is top left of text - not sure if this works with print
    M5.Lcd.setCursor(0,30);

    // format environment output and print
    sprintf(environment_string,"  Vtest: %1.3fV   Temp: %2.1fF", VTEST, TEMP);
    M5.Lcd.println(environment_string);

    // R1
    // It doesn't matter which model this is.  R1 needs to be set to 96k.  
    // Change the 100k resistor R1 to 96k on the board
    if (abs((R1_calculated - R1_desired)/R1_desired) < 0.01 ) 
    {
      M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    } else {
      M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    } 
    sprintf(R1_resistance_string,"R1=%3.3fk Target =%3.3fk",R1_calculated, R1_desired);
    M5.Lcd.println(R1_resistance_string);

    // R2
    if (abs((R2_calculated - R2_desired)/R2_desired) < 0.01 ) 
    {
      M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    } else {
      M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    }
    sprintf(R2_resistance_string,"R2=%1.3fk Target =%1.3fk",R2_calculated, R2_desired);
    M5.Lcd.println(R2_resistance_string);

    // R3
    if (abs((R3_calculated - R3_desired)/R3_desired) < 0.01 ) 
    {
      M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    } else {
      M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    }
    sprintf(R3_resistance_string,"R3=%1.3fk Target =%1.3fk",R3_calculated, R3_desired);
    M5.Lcd.println(R3_resistance_string);

    // R4
    M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    if (abs((R4_calculated - R4_desired_1)/R4_desired_1) < 0.01 ) 
    {
      // 6K
      model=6;
      model_value = R4_desired_1;
      M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    }

    if (abs((R4_calculated - R4_desired_2)/R4_desired_2) < 0.01 ) 
    {
      // 8K
      model=8;
      model_value = R4_desired_2;
      M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    } 
    sprintf(R4_resistance_string,"R4=%3.3fk Target =%3.1fk",R4_calculated, model_value);
    M5.Lcd.println(R4_resistance_string);

    // R5
    if (abs((R5_calculated - R5_test_resistor)/R5_test_resistor) < 0.01 ) 
    {
      M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    } else {
      M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    }
    sprintf(R5_resistance_string,"R5=%1.3fk Target =%1.3fk",R5_calculated, R5_test_resistor);
    M5.Lcd.println(R5_resistance_string);

    // R6
    if (abs((R6_calculated - R6_test_resistor)/R6_test_resistor) < 0.01 ) 
    {
      M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    } else {
      M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    }
    sprintf(R6_resistance_string,"R6=%1.3fk Target =%1.3fk",R6_calculated, R6_test_resistor);
    M5.Lcd.println(R6_resistance_string);

    // Wait for screen to clear, then Clear Screen and loop
    delay(500); // We can't loop more than 128 samples per second (maybe 32 x 4 channels per ADC = 31.25mS)

  }
}
