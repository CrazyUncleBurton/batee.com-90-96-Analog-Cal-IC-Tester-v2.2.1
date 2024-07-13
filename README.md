# batee.com 90-96 Analog Cal IC Tester

Bryan A. "CrazyUncleBurton" Thompson

 Last Updated 7/13/2024

Github Repository:  batee.com-90-96-Analog-Cal-IC-Tester-v2.2.1
<https://github.com/CrazyUncleBurton/batee.com-90-96-Analog-Cal-IC-Tester-v2.2.1>

## Overview

This is the production repository for the 90-96 Analog Cal IC Tester v2.2.1.  It is the firmware for the production Analog Cal IC tester.  

## M5Stack Core2 AWS I/O

* G19 - RELAY1_CONTROL (Resistors R2 and R3)
* G21 - Internal I2C BUS SDA
* G22 - Internal I2C BUS SCL
* G25 - RELAY3_CONTROL (Resistors R4 and R6)
* G26 - RELAY2_CONTROL (Resistors R1 and R5)

## I2C Addresses

In the M5Stack Core2 AWS on the internal I2C Bus:

* AXP192 Power Controller IC:   0x34
* ATECC608B Crypto Chip:        0x35 (in the bottom grey base not present in the tester)
* FT6336U Capacitive Touch IC:  0x38
* BM8563 RTC:                   0x51
* MPU6886 6 axis accel / gyro:  0x68 (in the bottom grey base not present in the tester)

Note the grey bottom base is removed from the Stack and not used in the 90-96 Analog Cal IC Tester.

On the purple 90-96 Analog Cal IC Tester Board on the Internal I2C Bus:

* U5 - ADS1115 ADC:             0x48
* U6 - ADS1115 ADC:             0x4A
* U10 - MCP9802-5T Temp Sender: 0x4D

## ADC Inputs

* U5_AIN0 - +5V_MEAS_U5_AIN0 - Vtest
* U5_AIN1 - DUT_R6_MEAS_U5_AIN1 - Voltage of the R6 divider pair
* U5_AIN2 - +PWRIN_MEAS_U5_AIN2 - Vin
* U5_AIN3 - DUT_R4_MEAS_U5_AIN3 - Voltage of the R4 divider pair
* U6_AIN0 - DUT_R2_MEAS_U6_AIN0 - Voltage of the R2 divider pair
* U6_AIN1 - DUT_R3_MEAS_U6_AIN1 - Voltage of the R3 divider pair
* U6_AIN2 - DUT_R1_MEAS_U6_AIN2 - Voltage of the R1 divider pair
* U6_AIN3 - DUT_R5_MEAS_U6_AIN3 - Voltage of the R5 divider pair

## Power

We use a standard MeanWell GS90 15V, 6A, 90W Power Supply with a 5.5m x 2.5mm connector.  The same one we use on our bench.  There is an input protection diode (D1) which protects everything against reverse voltage input.  We call this (Vin).

### VBUS CPU Voltage

This 5.1V voltage (Vbus) is provided from a switching power supply formed by a TPS5420DR IC U2.  Set the voltage the CPU receives to +5.1V.

### 5V0 Test Voltage

This 5V0 test voltage (VTEST) is created by a RECOM R78E5.0-1.0 Module U1.  

## Resistance Calculations

* Need to measure Test Voltage (Vtest in volts)
* Need to measure DUT output voltage for the resistor divider pair under test (Vmeas in volts)
* Need to know what the actual divider reisistor is (Rtop)
* Rbottom is the resistor under test
* Rbottom = (Vmeas * Rtop) / (Vtest - Vmeas)
