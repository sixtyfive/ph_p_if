#include "Arduino.h"
#include <RunningAverage.h>
#include <TinyWireS.h>

#define ADC3 3 // Input from 2nd op-amp stage
#define PB4  4 // TX/cradle LED
#define I2C_ADDR 0x65

// These are about as high as the chip can handle, RAM-wise.
// Striving for loads of smoothing and a slowly changing output value
// as the probes themselves need around 2 minutes to adjust to a change.
static uint8_t raw_buffer_size = 30;
static uint8_t voltage_buffer_size = 15;

RunningAverage raw_samples(raw_buffer_size);
RunningAverage voltage_samples(voltage_buffer_size);

/*
static float first_stage_gain = 3.78; // factor (measured between 1st op-amp input and output)

static float V_ref = 4.94; // volts
static float pH_stepsize_ideal = 59.16; // millivolts; ideal pH probe slope (about 12 ADC units)
static float pH_stepsize = pH_stepsize_ideal; // millivolts; calculated from pH 4 and 6 calibration values

static uint16_t ADC_u = 1023 + 1; // ADC units; two handful of bits, pH 14
static float one_step_u = (float)ADC_u / 14.0;

static uint16_t pH7_ideal_u = ADC_u / 2; // the middle between pH 7 and pH 14
int8_t pH7_calibration_u = 0; // either a positive or a negative value!
uint16_t pH7_u = pH7_ideal_u + pH7_calibration_u;

static uint16_t pH4_ideal_u = pH7_ideal_u - (3 * one_step_u); // minus three pH steps
// int8_t pH4_calibration_u = 0; // again, either a positive or a negative value
uint16_t pH4_u = 294; // pH4_ideal_u + pH4_calibration_u;
*/

// Default values are ideal values
//
// MCU-specific
//
float ADC_u = 1024;
// Sent by I2C master
//
unsigned char master_data_bytes[4 * sizeof(float)];
float first_stage_gain = 3.78;
float V_ref = 4.94;
float pH7_u = 512;
float pH4_u = 294;
// Calculated
//
bool cal_data_received = false;
float pH_stepsize = 59.16;
float pH = 7;
int pH_bytes[sizeof(float)]; // int-representation of pH

void calc_stepsize()
{
  pH_stepsize = ((((V_ref * (pH7_u - pH4_u)) / ADC_u) * 1000.0) / first_stage_gain) / (float)(7 - 4);
}

void request_event()
{
  digitalWrite(PB4, HIGH);
  for (uint8_t i=0; i<sizeof(float); i++)
    TinyWireS.send(pH_bytes[i]);
  tws_delay(20);
  digitalWrite(PB4, LOW);
}

void receive_event(uint8_t num_bytes)
{
  for (uint8_t i; i<sizeof(master_data_bytes); i++)
    master_data_bytes[i] = TinyWireS.receive();
}

void setup()
{
  pinMode(ADC3, INPUT);
  pinMode(PB4, OUTPUT);

  // Give off a sign of life
  digitalWrite(PB4, HIGH);
  tws_delay(350);
  digitalWrite(PB4, LOW);
  
  raw_samples.clear();
  voltage_samples.clear();

  calc_stepsize();

  TinyWireS.begin(I2C_ADDR);
  TinyWireS.onRequest(request_event);
  TinyWireS.onReceive(receive_event);
}

void parse_calibration_data()
{
  memcpy(&first_stage_gain, &master_data_bytes[ 0], sizeof(float));
  memcpy(&V_ref,            &master_data_bytes[ 4], sizeof(float));
  memcpy(&pH7_u,            &master_data_bytes[ 8], sizeof(float));
  memcpy(&pH4_u,            &master_data_bytes[12], sizeof(float));

  if (first_stage_gain != 0 &&
      V_ref            != 0 &&
      pH7_u            != 0 &&
      pH4_u            != 0)
    cal_data_received = true;
}

void loop()
{
  if (! cal_data_received) parse_calibration_data();

  uint16_t sample = analogRead(ADC3);
  raw_samples.addValue(sample);
  float mV = ((raw_samples.getAverage() / (float)ADC_u) * V_ref) * 1000.0;
  voltage_samples.addValue(mV);

  // Tried putting it all into one big formula, doesn't work - something something overflow something?
  pH = (((V_ref * pH7_u) / ADC_u) * 1000.0);
  pH = (pH - voltage_samples.getAverage()) / first_stage_gain;
  pH = 7.0 - (pH / pH_stepsize);

  unsigned char *ch = (unsigned char*)&pH;
  for (uint8_t i=0; i<sizeof(float); i++)
    pH_bytes[i] = ch[i];

  TinyWireS_stop_check();
}
