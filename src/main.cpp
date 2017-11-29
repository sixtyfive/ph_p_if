#include "Arduino.h"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RunningAverage.h>

LiquidCrystal_I2C lcd(0x3F, 2,1,0,4,5,6,7,3, POSITIVE);

uint32_t raw_buffer_size = 1000;
RunningAverage raw_samples(raw_buffer_size);

uint32_t voltage_buffer_size = 100;
RunningAverage voltage_samples(voltage_buffer_size);

uint32_t ADC_units = /* 0 to */ 1023; // - two hands full of bits
float Vref = 4.99; // volts
float first_stage_gain = 3.78; // factor (measured between 1st op-amp input and output)
float pH_stepsize_ideal = 59.16; // millivolts; ideal pH probe slope (about 12 ADC units)
float pH_stepsize = pH_stepsize_ideal; // millivolts; calculated from pH 4 and 6 calibration values
float pH7 = 488; // ADC units
// float pH4 = 220; // ADC units; short-wired probe
float pH4 = 20; // ADC units; long-wired, older probe

void calc_stepsize()
{
  pH_stepsize = ((((Vref * (pH7 - pH4)) / ADC_units) * 1000) / first_stage_gain) / (7 - 4);
  Serial.print("pH step size: ");
  Serial.println(pH_stepsize);
}

void setup()
{
  pinMode(A1, INPUT);

  Serial.begin(9600);
  while (!Serial);
  
  lcd.begin(16, 2);
  raw_samples.clear();
  calc_stepsize();
}

void loop()
{
  int sample = analogRead(A1);
  raw_samples.addValue(sample);
  int mV = ((raw_samples.getAverage() / ADC_units) * Vref) * 1000;

  voltage_samples.addValue(mV);
  float intermediate = ((((Vref * (float)pH7) / 
                          (float)ADC_units) * 1000) - 
                           voltage_samples.getAverage()) /
                           first_stage_gain;
  float pH = 7 - (intermediate / pH_stepsize);

  lcd.setCursor(0, 0);
  lcd.print("Raw: ");
  lcd.print(sample);
  lcd.print(" ");
  lcd.setCursor(0, 1);
  lcd.print(mV);
  lcd.print("mV, pH ");
  lcd.print(pH, 2);
  lcd.print("  ");
}
