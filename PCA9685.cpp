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

uint8_t PCA9685::getAddress(){
  return this->i2c_address;
}

// initialize everything
void PCA9685::begin(){
  // default mode: auto increment activated, other features off
#if(PCA9685_DEBUG == true)
  Serial.print("setting mode1 to ");
  Serial.println(PCA9685_AI);
#endif
  writeByte(PCA9685_MODE1, (PCA9685_AI | PCA9685_RESTART));
  setPWMFrequency(1000);
}

// change PWM frequency
void PCA9685::setPWMFrequency(uint16_t frequency){
  uint16_t frequency_int = frequency;
  if(frequency_int>1526){frequency_int=1526;}
  if(frequency_int<24){frequency_int=24;}
  uint16_t prescaler = (uint16_t) round((25000000.0 / (4096*frequency_int)) - 1);
#if(PCA9685_DEBUG == true)
  Serial.print("setPWMFrequency(");
  Serial.print(frequency);
  Serial.print(") --> prescaler = ");
  Serial.println(prescaler);
#endif
  // only allowed while sleep is active, so sleep, set prescaler, unsleep
  sleep();
  writeByte(PCA9685_PRESCALER, prescaler);
  unsleep();
}

// configure outputs to either totem pole or open drain
void PCA9685::setOutputMode(bool totempole){
  uint8_t mode2 = readByte(PCA9685_MODE2);
  if(totempole){
    mode2 |= PCA9685_OUTDRV;
  }else{
    mode2 &= ~PCA9685_OUTDRV;
  }
#if(PCA9685_DEBUG == true)
  Serial.print("setOutputMode(");
  Serial.print(totempole);
  Serial.print(") --> mode2 = ");
  Serial.println(mode2);
#endif
  writeByte(PCA9685_MODE2, mode2);
}

void PCA9685::sleep(){
  uint8_t mode1 = readByte(PCA9685_MODE1);
  mode1 |= PCA9685_SLEEP;
  writeByte(PCA9685_MODE1, mode1);
}

void PCA9685::unsleep(){
  uint8_t mode1 = readByte(PCA9685_MODE1);
  mode1 &= ~PCA9685_SLEEP;
  writeByte(PCA9685_MODE1, mode1);
}

// sets on and off times of one LED
// numbers 0..15
// 0..4095 for data values, 4096 to set the always on / always off bits
void PCA9685::setOutput(uint8_t number, uint16_t on, uint16_t off){
  uint8_t data[4];
  data[0] = (uint8_t)(on & 0xFF); // ON_L
  data[1] = (uint8_t)((on>>8) & 0x1F); // ON_H
  data[2] = (uint8_t)(off & 0xFF); // OFF_L
  data[3] = (uint8_t)((off>>8) & 0x1F); // OFF_H
#if(PCA9685_DEBUG == true)
  Serial.print("writing to address ");
  Serial.print((PCA9685_FIRSTLED + (4*number)));
  Serial.print(" an array with values [");
  Serial.print(data[0]);
  Serial.print(",");
  Serial.print(data[1]);
  Serial.print(",");
  Serial.print(data[2]);
  Serial.print(",");
  Serial.print(data[3]);
  Serial.println("]");
#endif
  writeFourBytes((PCA9685_FIRSTLED + (4*number)), data[0], data[1], data[2], data[3]);
}

// sets PWM value, including control of the always on / always off flags
void PCA9685::setOutput(uint8_t number, uint16_t pwmvalue){
  uint16_t on = 0;
  uint16_t off = 0;
  if(pwmvalue >= 4095){
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
#if(PCA9685_DEBUG == true)
  Serial.print("setOutput(");
  Serial.print(number);
  Serial.print(",");
  Serial.print(on);
  Serial.print(",");
  Serial.print(off);
  Serial.println(")");
#endif
  setOutput(number, on, off);
}

// sets [count] number of outputs to their PWM values, spreadíng the on/off times to decrease EMI
// the algorithm overlaps the on-times as much as necessary to fit within the 4096 steps
void PCA9685::setOutputs(uint8_t count, uint16_t pwmvalue[]){
  int totalOnTimeOfDimmedLEDs = 0; // length of total non-off, non-on, dimmed LED on-time
  int numberOfDimmedLEDs = 0;
  for(int i = 0; i < count; i++){
    if(pwmvalue[i] > 0 && pwmvalue[i] < 4095){
      totalOnTimeOfDimmedLEDs += pwmvalue[i];
      numberOfDimmedLEDs++;
    }
  }
  // the individual overlap is the total "too much" time, divided by the number of LEDs
  // overlap is allowed to become negative, this will space out the short on-times over the available time
  int overlap = (totalOnTimeOfDimmedLEDs - 4095) / numberOfDimmedLEDs;
  uint16_t on[16];
  uint16_t off[16];
  uint16_t currentoffset = 0;
  for(int i = 0; i < count; i++){
    if(pwmvalue[i] > 0 && pwmvalue[i] < 4095){
      // this is a dimmed LED, apply offset
      on[i] = 0 + currentoffset;
      off[i] = pwmvalue[i] + currentoffset;
      currentoffset = 4096 + off[i] - overlap;
      // normalize to 0..4095
      while(on[i] > 4095){on[i] -= 4096;}
      while(off[i] > 4095){off[i] -= 4096;}
    }else if(pwmvalue[i] == 0){
      // LED is off
      on[i] = 0;
      off[i] = 4096;
    }else{
      // LED is on
      on[i] = 4096;
      off[i] = 0;
    }
  }
  // convert to byte array: ON_L, ON_H, OFF_L, OFF_H
  int dataindex = 0;
  uint8_t data[64];
  for(int i = 0; i < count; i++){
    data[0 + dataindex] = (uint8_t)(on[i] & 0xFF); // ON_L
    data[1 + dataindex] = (uint8_t)((on[i]>>8) & 0x1F); // ON_H
    data[2 + dataindex] = (uint8_t)(off[i] & 0xFF); // OFF_L
    data[3 + dataindex] = (uint8_t)((off[i]>>8) & 0x1F); // OFF_H
    dataindex += 4;
#if(PCA9685_DEBUG == true)
  Serial.print("setOutput(");
  Serial.print(i);
  Serial.print(",");
  Serial.print(on[i]);
  Serial.print(",");
  Serial.print(off[i]);
  Serial.println(")");
#endif
  }
  // send to PWM driver as one block transmission
  writeBytes(PCA9685_FIRSTLED, data, dataindex);
}

// read a byte from I2C
uint8_t PCA9685::readByte(uint8_t address){
  Wire.beginTransmission(i2c_address);
  Wire.write(address);
  Wire.endTransmission(true);
  Wire.requestFrom(i2c_address, 1);
  return Wire.read();
}

// write a byte to I2C
void PCA9685::writeByte(uint8_t address, uint8_t data){
#if(PCA9685_DEBUG == true)
  Serial.print("i2c_address=");
  Serial.println(i2c_address);
  Serial.print("address=");
  Serial.println(address);
#endif
  Wire.beginTransmission(i2c_address);
  Wire.write(address);
  Wire.write(data);
  uint8_t retval = Wire.endTransmission(true);
#if(PCA9685_DEBUG == true)
  Serial.print("writeByte(");
  Serial.print(address);
  Serial.print(",");
  Serial.print(data);
  Serial.print(") = ");
  Serial.println(retval);
#endif
}

// write 4 bytes to I2C
void PCA9685::writeFourBytes(uint8_t address, uint8_t data0, uint8_t data1, uint8_t data2, uint8_t data3){
#if(PCA9685_DEBUG == true)
  Serial.print("i2c_address=");
  Serial.println(i2c_address);
  Serial.print("address=");
  Serial.println(address);
#endif
  Wire.beginTransmission(i2c_address);
  Wire.write(address);
  Wire.write(data0);
  Wire.write(data1);
  Wire.write(data2);
  Wire.write(data3);
  uint8_t retval = Wire.endTransmission(true);
#if(PCA9685_DEBUG == true)
  Serial.print("writeFourBytes(");
  Serial.print(address);
  Serial.print(",");
  Serial.print(data0);
  Serial.print(",");
  Serial.print(data1);
  Serial.print(",");
  Serial.print(data2);
  Serial.print(",");
  Serial.print(data3);
  Serial.print(") = ");
  Serial.println(retval);
#endif
}

// write consecutive bytes to I2C
void PCA9685::writeBytes(uint8_t address, uint8_t data[], uint8_t length){
#if(PCA9685_DEBUG == true)
  Serial.print("i2c_address=");
  Serial.println(i2c_address);
  Serial.print("address=");
  Serial.println(address);
#endif
  Wire.beginTransmission(i2c_address);
  Wire.write(address);
  for(uint8_t i=0;i<length;i++){
    Wire.write(data[i]);
  }
  uint8_t retval = Wire.endTransmission(true);
#if(PCA9685_DEBUG == true)
  Serial.print("writeByte(");
  Serial.print(address);
  Serial.print(",[");
  for(uint8_t k=0;k<length;k++){
    if(k>0){Serial.print(",");}
    Serial.print(data[k]);
  }
  Serial.print("]) = ");
  Serial.println(retval);
#endif
}

