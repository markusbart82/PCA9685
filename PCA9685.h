#ifndef _PCA9685_H
#define _PCA9685_H

#include <Arduino.h>
#include <Wire.h>

// addresses
#define PCA9685_DEFAULT_I2C_ADDRESS 0x40
#define PCA9685_MODE1 0x00 // mode register 1
#define PCA9685_MODE2 0x01 // mode register 2
#define PCA9685_FIRSTLED 0x06 // each LED has ON_L, ON_H, OFF_L, OFF_H addresses
#define PCA9685_PRESCALER 0xFE

// MODE1
#define PCA9685_RESTART 0x80
#define PCA9685_EXTCLK 0x40
#define PCA9685_AI 0x20
#define PCA9685_SLEEP 0x10
#define PCA9685_SUB1 0x08
#define PCA9685_SUB2 0x04
#define PCA9685_SUB3 0x02
#define PCA9685_ALLCALL 0x01
// MODE2
#define PCA9685_INVRT 0x10 // 1: output logic inverted, 0: output logic not inverted
#define PCA9685_OCH 0x08 // 1: output change on ACK, 0: output change on STOP
#define PCA9685_OUTDRV 0x04 // 1: totem pole, 0: open-drain
#define PCA9685_OUTNE 0x03 // output disabled leads to... 00: LEDs off, 01: LEDs on, 10: LEDs high impendance

#define PCA9685_DEBUG true

class PCA9685{
  public:
    PCA9685();
    PCA9685(const uint8_t address);
    uint8_t getAddress();
    void begin();
    void setPWMFrequency(uint16_t frequency);
    void setOutputMode(bool totempole);
    void setOutput(uint8_t number, uint16_t on, uint16_t off);
    void setOutput(uint8_t number, uint16_t pwmvalue);
  private:
    const uint8_t i2c_address;
    uint8_t readByte(uint8_t address);
    void writeByte(uint8_t address, uint8_t data);
    void writeFourBytes(uint8_t address, uint8_t data0, uint8_t data1, uint8_t data2, uint8_t data3);
    void writeBytes(uint8_t address, uint8_t data[], uint8_t length);
};

#endif