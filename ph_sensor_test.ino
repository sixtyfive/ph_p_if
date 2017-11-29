#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RunningAverage.h>

#define IF_PIN A0

RunningAverage Measurement(60);
int samples = 0;
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void setup()
{
  lcd.begin(16, 2);
  lcd.print("Hi there!");

  Measurement.clear();

  delay(1000);
}

void loop()
{
  Measurement.addValue(analogRead(IF_PIN));
  samples++;
  
  lcd.setCursor(0, 0);
  lcd.print(Measurement.getAverage());
  lcd.print("         ");

  if (samples == 600) {
    samples = 0;
    Measurement.clear();
  }
  
  float voltage = (Measurement.getAverage() / 6.0) * (5.0 / 1024.0);

  lcd.setCursor(0, 1);
  lcd.print(voltage);
  lcd.print("V         ");
  
  delay(30);
}

/*
#define SensorPin A0
#define CENTER 522

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void setup()
{  
  lcd.begin(16, 2); // rows, cols
  lcd.print("Hi there!");

  Serial.begin(9600);
  
  delay(1000);
}

void loop()
{
  float voltage, pH;
  char V_string[16];
  char pH_string[16];
  char lcd_l1_output_string[16];
  char lcd_l1_format_string[] = "%sV (%i)";
  char lcd_l2_output_string[16];
  char lcd_l2_format_string[] = "pH %s";
  
  int rawValue = analogRead(SensorPin);
  int buf[10];

  for(int i=0; i<10; i++) { 
     buf[i] = rawValue;
     delay(10);
  }
  
  int avgValue = 0;
  for (int i=2; i<8; i++)
    avgValue += (buf[i] - CENTER);

  voltage = (((float)avgValue) / 6.0) * (5.0 / 1024.0);
  
  dtostrf(voltage, 4, 3, V_string); // third argument: decimal places
  sprintf(lcd_l1_output_string, lcd_l1_format_string, V_string, rawValue);

  pH = voltage * (-1.33) + 7.0;

  dtostrf(pH, 3, 1, pH_string); // third argument: decimal places
  sprintf(lcd_l2_output_string, lcd_l2_format_string, pH_string);
  
  // lcd.clear(); // only causes flickering and it's not like string lengths were changing...
  lcd.setCursor(0, 0);
  lcd.print(lcd_l1_output_string);
  lcd.setCursor(0, 1);
  lcd.print(lcd_l2_output_string);

  Serial.print(lcd_l1_output_string);
  Serial.print(" - ");
  Serial.println(lcd_l2_output_string);
  
  delay(1000);
}
*/
