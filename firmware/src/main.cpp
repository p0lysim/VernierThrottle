#include <Arduino.h>
#include <Wire.h>
#include <Joystick.h>

// AS5600 I2C Address
#define AS5600_ADDRESS 0x36

// AS5600 Register Addresses
#define AS5600_RAW_ANGLE_H 0x0C // Raw angle high byte
#define AS5600_RAW_ANGLE_L 0x0D // Raw angle low byte

// Potentiometer pin
#define POT_PIN A5

// Last value read from the AS5600
uint16_t _last_AS5600_Value = 0;

// Last value read from the potentiometer
float _last_Potentiometer_Value = 0;

// Current combined value [0 - 100]
float _current_value = 0;

uint8_t joystick_ID = JOYSTICK_DEFAULT_REPORT_ID;
uint8_t joystick_Type = JOYSTICK_TYPE_JOYSTICK;
uint8_t nb_btn = 32;
uint8_t nb_hatSwitch = 0;
bool XAxis = false;
bool YAxis = false;
bool ZAxis = false;
bool RxAxis = false;
bool RyAxis = false;
bool RzAxis = false;
bool Rudder = false;
bool Throttle = true;
bool Accelerator = false;
bool Brake = false;
bool Steering = false;
Joystick_ Joystick(joystick_ID, joystick_Type, nb_btn, nb_hatSwitch, XAxis, YAxis, ZAxis, RxAxis, RyAxis, RzAxis, Rudder, Throttle, Accelerator, Brake, Steering);

// Function to read a 16-bit value from AS5600
uint16_t readAS5600(uint8_t reg_high, uint8_t reg_low)
{

  uint8_t highByte, lowByte;

  // Read high byte
  Wire.beginTransmission(AS5600_ADDRESS);
  Wire.write(reg_high);
  Wire.endTransmission(false);
  Wire.requestFrom(AS5600_ADDRESS, 1);
  highByte = Wire.read();

  // Read low byte
  Wire.beginTransmission(AS5600_ADDRESS);
  Wire.write(reg_low);
  Wire.endTransmission(false);
  Wire.requestFrom(AS5600_ADDRESS, 1);
  lowByte = Wire.read();

  return (highByte << 8) | lowByte;
}

// Function to read raw angle (0-4095, 12-bit)
uint16_t readRawAngle()
{
  return readAS5600(AS5600_RAW_ANGLE_H, AS5600_RAW_ANGLE_L) & 0x0FFF;
}

int readPotentiometerValue()
{
  int potValue = analogRead(POT_PIN); // Read (0-1023)
  return potValue;
}

float PotValueToPercentage(int potValue)
{
  return ((float)potValue * 100.0) / 1023.0;
}

void setup()
{

  Serial.begin(115200);
  Wire.begin();

  delay(1000);

  _last_AS5600_Value = readRawAngle();

  float potValue = readPotentiometerValue(); // Read potentiometer [0-1023]
  _current_value = potValue;
  _last_Potentiometer_Value = potValue;

  Joystick.begin();
  Joystick.setThrottleRange(0, 1023);

  delay(1000);
}

void loop()
{
  // float debug_current_value_1 = _current_value;
  // float debug_current_value_2;
  // float debug_current_value_3;

  /*-------------------------------------------
  * AS56090 handling
  -------------------------------------------*/
  uint16_t as5600Value = readRawAngle(); // Read AS5600 magnetic sensor [0-4095]

  // AS6500 increment

  float as5600_Increment = (float)((int16_t)as5600Value - (int16_t)_last_AS5600_Value);

  // Case where we completed a full clockwise turn
  if (_last_AS5600_Value > 3000 && as5600Value < 1000)
  {
    as5600_Increment = (4095 - (int16_t)_last_AS5600_Value) + (int16_t)as5600Value;
  }
  // Case where we completed a full counter-clockwise turn
  else if (_last_AS5600_Value < 1000 && as5600Value > 3000)
  {
    as5600_Increment = -((4095 - (int16_t)as5600Value) + (int16_t)_last_AS5600_Value);
  }

  float _increment_as5600_precis = (float)(((as5600_Increment * 0.1) / 4095.0) * 1023.0);
  _increment_as5600_precis *= -1;

  if (_increment_as5600_precis != 0)
  {
    _current_value += _increment_as5600_precis;
    _last_AS5600_Value = as5600Value;
    // debug_current_value_2 = _current_value;
  }

  /*-------------------------------------------
  * Potentiometer handling
  -------------------------------------------*/
  float potValue = readPotentiometerValue(); // Read potentiometer [0-1023]
  float pot_Increment = potValue - _last_Potentiometer_Value;

  // Remap the potentiometer increment to match the range with the AS5600 offset
  float pot_increment_remapped = 0;

  if (pot_Increment < -1)
  {
    pot_increment_remapped = (pot_Increment * _current_value) / _last_Potentiometer_Value;
  }
  else if (pot_Increment > 1)
  {
    pot_increment_remapped = (pot_Increment * (1023.0 - _current_value)) / (1023.0 - _last_Potentiometer_Value);
  }

  if (pot_increment_remapped != 0)
  {
    _current_value += pot_increment_remapped;
    _last_Potentiometer_Value = potValue;
    // debug_current_value_3 = _current_value;
  }

  // Ensure the current value stays within [0 - 1023]
  if (_current_value < 0)
  {
    _current_value = 0;
  }
  else if (_current_value > 1023)
  {
    _current_value = 1023;
  }

  Joystick.setThrottle(_current_value);

  // Serial.println(PotValueToPercentage(_current_value));

  // if(pot_increment_remapped != 0){
  //   Serial.print("    ----> ");
  //   Serial.println(pot_increment_remapped);
  // }

  //  if (_increment_as5600_precis != 0 || pot_increment_remapped != 0)
  //  {
  //    Serial.print(_current_value);
  //    Serial.print(" | ");
  //    Serial.print(_increment_as5600_precis);
  //    Serial.print(" | ");
  //    Serial.print(pot_increment_remapped);
  //    Serial.print(" | ");
  //    Serial.print(debug_current_value_1);
  //    Serial.print(" ->  ");
  //    Serial.print(debug_current_value_2);
  //    Serial.print(" ->  ");
  //    Serial.println(debug_current_value_3);
  //  }

  // delay(100); // Update every 100ms
}