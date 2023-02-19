/*
 * PCA9685 16 channel 12 bit PWM driver
 *
 * This library uses the arduino Wire library, using the default SDA/SCL pins of Arduino.
 */

#include "PCA9685.h"
#include <Wire.h>

// constructor with default address
PCA9685::PCA9685():i2c_address(PCA9685_DEFAULT_I2C_ADDRESS){}

// constructor with provided address
PCA9685::PCA9685(const uint8_t address):i2c_address(address){}

// initialize everything
void PCA9685::begin(){
  Wire.begin();
  setPWMFrequency(1000);
}

// change PWM frequency
void PCA9685::setPWMFrequency(uint8_t frequency){
  uint8_t frequency_int = frequency;
  if(frequency_int>1526){frequency_int=1526;}
  if(frequency_int<24){frequency_int=24;}
  uint8_t prescaler = (uint8_t) round((25000000.0 / (4096*frequency_int)) - 1);
  writeByte(PCA9685_PRESCALER, prescaler);
}

// configure outputs to either totem pole or open drain
void PCA9685::setOutputMode(bool totempole){
  uint8_t mode2 = readByte(PCA9685_MODE2);
  if(totempole){
    mode2 |= PCA9685_OUTDRV;
  }else{
    mode2 &= ~PCA9685_OUTDRV;
  }
  writeByte(PCA9685_MODE2, mode2);
}

// sets on and off times of one LED
// numbers 0..15
// 0..4095 for data values, 4096 to set the always on / always off bits
void PCA9685::setOutput(uint8_t number, uint16_t on, uint16_t off){
  uint8_t data[4];
  data[0] = (uint8_t)(on & 0x00FF);
  data[1] = (uint8_t)((on & 0xFF)>>8);
  data[2] = (uint8_t)(off & 0x00FF);
  data[1] = (uint8_t)((off & 0xFF)>>8);
  writeBytes((PCA9685_FIRSTLED + (4*number)), data, 4);
}

// sets PWM value, including control of the always on / always off flags
void PCA9685::setOutput(uint8_t number, uint16_t pwmvalue){
  uint8_t on = 0;
  uint8_t off = 0;
  if(pwmvalue > 4095){
    // always on
    on = 4096;
    off = 0;
  }else if(pwmvalue == 0){
    // always off
    on = 0;
    off = 4096;
  }else{
    // pwm value
    on = 0;
    off = pwmvalue;
  }
  setOutput(number, on, off);
}

// read a byte from I2C
uint8_t PCA9685::readByte(uint8_t address){
  Wire.beginTransmission(i2c_address);
  Wire.write(address);
  Wire.endTransmission();
  Wire.requestFrom(i2c_address, 1);
  return Wire.read();
}

// write a byte to I2C
void PCA9685::writeByte(uint8_t address, uint8_t data){
  Wire.beginTransmission(i2c_address);
  Wire.write(address);
  Wire.write(data);
  Wire.endTransmission();
}

// write consecutive bytes to I2C
void PCA9685::writeBytes(uint8_t address, uint8_t data[], uint8_t length){
  Wire.beginTransmission(i2c_address);
  Wire.write(address);
  for(uint8_t i=0;i<length;i++){
    Wire.write(data[i]);
  }
  Wire.endTransmission();
}

