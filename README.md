# batee.com 90-96 Analog Cal IC Tester

Bryan A. "CrazyUncleBurton" Thompson

 Last Updated 6/30/2024

Github Repository:  batee.com-90-96-Analog-Cal-IC-Tester-v2.2.1
<https://github.com/CrazyUncleBurton/batee.com-90-96-Analog-Cal-IC-Tester-v2.2.1>

## Overview

This is the production repository for the 90-96 Analog Cal IC Tester v2.2.1.  It is the firmware for the production Analog Cal IC tester.  

## M5Stack Core2 AWS I/O

G19 - RELAY1_CONTROL
G21 - Internal I2C BUS SDA
G22 - Internal I2C BUS SCL
G25 - RELAY3_CONTROL
G26 - RELAY2_CONTROL

## I2C Addresses

U5 ADS1115 ADC:  0x48
U6 ADS1115 ADC:  0x4A
U10 MCP9802-5T Temp Sender:  0x4D

## ADC Inputs

U5_AIN0 - +5V_MEAS_U5_AIN0
U5_AIN1 - DUT_R6_MEAS_U5_AIN1
U5_AIN2 - +PWRIN_MEAS_U5_AIN2
U5_AIN3 - DUT_R4_MEAS_U5_AIN3
U6_AIN0 - DUT_R2_MEAS_U6_AIN0
U6_AIN1 - DUT_R3_MEAS_U6_AIN1
U6_AIN2 - DUT_R1_MEAS_U6_AIN2
U6_AIN3 - DUT_R5_MEAS_U6_AIN3

## Power

We use a standard MeanWell GS90 15V, 6A, 90W Power Supply with a 5.5m x 2.5mm connector.  The same one we use on our bench.  There is an input protection diode (D1) which protects everything against reverse voltage input.

### VBUS CPU Voltage

This is provided from a switching power supply formed by a TPS5420DR IC U2.

### 5V0 Test Voltage

This is created by a RECOM R78E5.0-1.0 Module U1.

## Tests

1) Measure voltage at +PWRIN_MEAS_U5_AIN2.  Report if it's not around 15V.
2) Measure voltage at +5V_MEAS_U5_ANI0.  This measurement is needed to convert the other measurements to resistance.
3) Measure voltage at DUT_R2_MEAS_U6_AIN0.
4) Measure voltage at DUT_R3_MEAS_U6_AIN1.
5) Measure voltage at DUT_R5_MEAS_U6_AIN3.
6) Measure voltage at DUT_R4_MEAS_U4_AIN3. Determine if this is a 6K or an 8K product based on this measurement.
7) Measure voltage at DUT_R6_MEAS_U5_AIN1.  
8) Measure voltage at DUT_R1_MEAS_U6_AIN2. Loop and prompt operator until R4 is in range based on what type of IC we determined this to be.  
