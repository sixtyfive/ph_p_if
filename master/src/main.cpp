#include "Arduino.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RunningAverage.h>

#define PH_P_IF_ADDR 0x65
#define DISPLAY_ADDR 0x3F
LiquidCrystal_I2C LCD = LiquidCrystal_I2C(DISPLAY_ADDR, 2,1,0,4,5,6,7,3, POSITIVE);
RunningAverage samples(255);

unsigned char data[sizeof(float)];
bool cal_data_sent = false;
float cal_data[] = {3.78 /* 1st stage gain */, 4.94 /* V_ref */, 512 /* pH 7 raw */, 294 /* pH 4 raw */};
uint8_t cal_data_bytes[sizeof(cal_data) * sizeof(float)];

void setup() {
  pinMode(A0, INPUT);
  samples.clear();

  Serial.begin(9600);
  LCD.begin(16, 2);
  
  Serial.println("Hi there!");
  LCD.print("Hi there!");

  Wire.begin(); 
  delay(2000);
}

float read_samples()
{
  samples.addValue(analogRead(A0));
  return samples.getAverage();
}

float read_wire()
{
  uint8_t req_bytes = Wire.requestFrom(PH_P_IF_ADDR, sizeof(float));
  
  for (int i=0; i<req_bytes; i++) {
    data[i] = Wire.read();
  }

  float pH = 0;
  memcpy(&pH, data, sizeof(pH));

  return pH;
}

void print_data(float raw_avg, float pH)
{
  Serial.print(pH, 5);
  Serial.print("\t");
  Serial.println(raw_avg, 1);

  LCD.setCursor(0, 0);
  LCD.print("Raw ");
  LCD.print((int)raw_avg);
  LCD.print("      ");

  LCD.setCursor(0, 1);
  LCD.print("pH ");
  LCD.print(pH, 1);
  LCD.print("      ");
}

void send_calibration_data()
{
  Wire.beginTransmission(PH_P_IF_ADDR);

  for (uint8_t i=0; i<sizeof(cal_data)/sizeof(float); i++) {
    unsigned char *ch = (unsigned char*)&cal_data[i];
    for (uint8_t j=0; j<sizeof(float); j++) {
      cal_data_bytes[i*j] = ch[j];
      Wire.write(ch[j]);
    }
  }

  Wire.endTransmission();
  cal_data_sent = true;
}

void loop()
{
  if (! cal_data_sent) send_calibration_data();
  float raw_avg = read_samples();
  float pH = read_wire();
  print_data(raw_avg, pH);
}
