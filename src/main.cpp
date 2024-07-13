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
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setFreeFont(FSB12);      // Select Free Serif 9 point font
  M5.Lcd.setTextDatum(MC_DATUM); // cursor is top left of text - not sure if this works with print
  M5.Lcd.setCursor(0, 30);       // Set initial position - might not be needed withdrawString? 
  M5.Lcd.drawString("batee.com", 120 , 30);
  M5.Lcd.drawString("Analog Cal IC Tester",120, 63);
  M5.Lcd.drawString("Hardware: v2.2.1", 120, 96);
  M5.Lcd.drawString("Firmware v1.0.0", 120, 129);

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
    float VIN, VTEST; // Calculated values of measured voltages with voltage dividers factored in
    char input_voltage_string[16];
    char test_voltage_string[16];
    char R1_voltage_string[16];
    char R2_voltage_string[16];
    char R3_voltage_string[16];
    char R4_voltage_string[16];
    char R5_voltage_string[16];
    char R6_voltage_string[16];
    char R1_resistance_string[16];
    char R2_resistance_string[16];
    char R3_resistance_string[16];
    char R4_resistance_string[16];
    char R5_resistance_string[16];
    char R6_resistance_string[16];
    float temperature = 74;
    char temperature_string[16];
    float R1_test_resistor = 96.000; // in kOhms as measured from ground to the DUT Socket pin 10
    float R2_test_resistor = 4.017; // in kOhms as measured from ground to the DUT Socket pin 13
    float R3_test_resistor = 2.001; // in kOhms as measured from ground to the DUT Socket pin 11
    float R4_test_resistor = 174.200; // in kOhms as measured from ground to the DUT Socket pin 0
    float R5_test_resistor = 4.518; // in kOhms as measured from ground to the DUT Socket pin 4
    float R6_test_resistor = 3.001; // in kOhms as measured from ground to the DUT Socket pin 7

    // Measure 15V0 Input Voltage
    adc2 = ads.readADC_SingleEnded(2); // U5_AIN2 - 15V0 Power In Voltage
    U5_AIN2 = ads.computeVolts(adc2); // U5_AIN2 - 15V0 Power In Voltage
    // U5_AIN2 is the measured voltage.  We need to multiply by the voltage divider to recover the actual voltage
    VIN = U5_AIN2 * 6.00235386;

    // Measure 5V0 Test Voltage
    adc0 = ads.readADC_SingleEnded(0); // U5_AIN0 - 5V0 Test Voltage
    U5_AIN0 = ads.computeVolts(adc0); // U5_AIN0 - 5V0 Test Voltage
    VTEST = U5_AIN0 * 2.0011928;

    // Measure R1
    adc6 = ads2.readADC_SingleEnded(2); // U6_AIN2 - DUT_R1
    U6_AIN2 = ads2.computeVolts(adc6); // U6_AIN2 - DUT_R1
    DUT_R1 = ((U6_AIN2 * R1_test_resistor ) / (VTEST - U6_AIN2)); // in kOhms

    // Measure R2
    adc4 = ads2.readADC_SingleEnded(0); // U6_AIN0 - DUT_R2
    U6_AIN0 = ads2.computeVolts(adc4); // U6_AIN0 - DUT_R2
    DUT_R2 = ((U6_AIN0 * R2_test_resistor ) / (VTEST - U6_AIN0)); // in kOhms

    // Measure R3
    adc5 = ads2.readADC_SingleEnded(1); // U6_AIN1 - DUT_R3
    U6_AIN1 = ads2.computeVolts(adc5); // U6_AIN1 - DUT_R3
    DUT_R3 = ((U6_AIN1 * R3_test_resistor ) / (VTEST - U6_AIN1)); // in kOhms

    // Measure R4
    adc3 = ads.readADC_SingleEnded(3); // U5_AIN3 - DUT_R4
    U5_AIN3 = ads.computeVolts(adc3); // U5_AIN3 - DUT_R4
    DUT_R4 = ((U5_AIN3 * R4_test_resistor ) / (VTEST - U5_AIN3)); // in kOhms

    // Measure R5
    adc7 = ads2.readADC_SingleEnded(3); // U6_AIN3 - DUT_R5
    U6_AIN3 = ads2.computeVolts(adc7); // U6_AIN3 - DUT_R5
    DUT_R5 = ((U6_AIN3 * R5_test_resistor ) / (VTEST - U6_AIN3)); // in kOhms

    // Measure R6
    adc1 = ads.readADC_SingleEnded(1); // U5_AIN1 - DUT_R6
    U5_AIN1 = ads.computeVolts(adc3); // U5_AIN1 - DUT_R6
    DUT_R6 = ((U5_AIN1 * R6_test_resistor ) / (VTEST - U5_AIN1)); // in kOhms.  
    

    // Report Results

    // Measure Temperature and report
    // For some reason, the first reading is always excessively high.
    // Read the temperature twice to work around this problem.
    temperature = get_temperature();
    if (temperature > 100) 
    {
      temperature = get_temperature();
    }

    // Report Test Conditions and format readings
    sprintf(temperature_string,"%2.1fF",temperature);
    sprintf(input_voltage_string,"%2.3f V",VIN); // If I put the space here I want, the unit reboots right after printing this due to a buffer overflow --CUB
    sprintf(test_voltage_string,"%1.3f V",VTEST); // If I put the space here I want, the unit reboots right after printing this due to a buffer overflow --CUB
    
    M5.Lcd.clear();   // Clear the contents displayed on the screen
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setFreeFont(FSB9);      // Select Free Serif 9 point font
    M5.Lcd.setTextDatum(MC_DATUM); // cursor is top left of text - not sure if this works with print
    M5.Lcd.setCursor(0, 30);
    M5.Lcd.print("Vin: ");
    M5.Lcd.println(input_voltage_string);
     M5.Lcd.print("Vtest: ");
    M5.Lcd.println(test_voltage_string);
    M5.Lcd.print("Temp: ");
    M5.Lcd.println(temperature_string);

    // R1
    // It doesn't matter which model this is.  R1 needs to be set to 96k.  
    // Change the 100k resistor R1 to 96k on the board
    if (abs((DUT_R1 - R1_test_resistor)/R1_test_resistor) < 0.01 ) 
    {
      M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    } else {
      M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    } 
    M5.Lcd.print("R1 = ");
    M5.Lcd.print(DUT_R1);
    M5.Lcd.print("k");
    M5.Lcd.print("  Target = ");
    M5.Lcd.print(R1_test_resistor);
    M5.Lcd.println("k");

    // R2
    if (abs((DUT_R2 - R2_test_resistor)/R2_test_resistor) < 0.01 ) 
    {
      M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    } else {
      M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    }
    M5.Lcd.print("R2 = ");
    M5.Lcd.print(DUT_R2);
    M5.Lcd.print("k");
    M5.Lcd.print("  Target = ");
    M5.Lcd.print(R2_test_resistor);
    M5.Lcd.println("k");

    // R3
    if (abs((DUT_R3 - R3_test_resistor)/R3_test_resistor) < 0.01 ) 
    {
      M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    } else {
      M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    }
    M5.Lcd.print("R3 = ");
    M5.Lcd.print(DUT_R3);
    M5.Lcd.print("k");
    M5.Lcd.print("  Target = ");
    M5.Lcd.print(R3_test_resistor);
    M5.Lcd.println("k");

    // R4

    if ((abs((DUT_R4 - R4_test_resistor)/R4_test_resistor) < 0.01 ) or (abs((DUT_R4 - 124.000)/124.000) < 0.01 ))
    {
      M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    } else {
      M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    }
    M5.Lcd.print("R4 = ");
    M5.Lcd.print(DUT_R4);
    M5.Lcd.print("k");
    M5.Lcd.print("  Target = ");
    M5.Lcd.print(R4_test_resistor);
    M5.Lcd.println("k");

    // R5
    if (abs((DUT_R5 - R5_test_resistor)/R5_test_resistor) < 0.01 ) 
    {
      M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    } else {
      M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    }
    M5.Lcd.print("R5 = ");
    M5.Lcd.print(DUT_R5);
    M5.Lcd.print("k");
    M5.Lcd.print("  Target = ");
    M5.Lcd.print(R5_test_resistor);
    M5.Lcd.println("k");

    // R6
    if (abs((DUT_R6 - R6_test_resistor)/R6_test_resistor) < 0.01 ) 
    {
      M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    } else {
      M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    }
    M5.Lcd.print("R6 = ");
    M5.Lcd.print(DUT_R6);
    M5.Lcd.print("k");
    M5.Lcd.print("  Target = ");
    M5.Lcd.print(R6_test_resistor);
    M5.Lcd.println("k");



    // Wait for screen to clear, then Clear Screen and loop
    delay(500); // We can't loop more than 128 samples per second (maybe 32 x 4 channels per ADC = 31.25mS)
    // M5.Lcd.clear();   // Clear the contents displayed on the screen
    // M5.Lcd.setCursor(0, 30);

  }
}
