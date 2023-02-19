# PCA9685 library
This is a library for controling the PCA9685 PWM driver via I2C from an Arduino.

This library only provides basic functionality, and uses Arduino's Wire library, so the default SDA/SCL pins have to be used.

# API

## Constructors
```cpp
PCA9685();
PCA9685(conconst uint8_t address);
```

## Initialization and configuration
```cpp
void begin();
void setPWMFrequency(uint8_t frequency);
void setOutputMode(bool totempole);
```

## Controling LEDs
```cpp
void setOutput(uint8_t number, uint16_t on, uint16_t off);
```

# Examples

## Minimum viable code
```cpp
PCA9685 pwm = new PCA9685();
pwm.begin();
pwm.setOutput(0, 0, 4095);
```
