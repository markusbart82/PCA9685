# PCA9685 library
This is a library for controling the PCA9685 PWM driver via I2C from an Arduino.

This library only provides basic functionality, and uses Arduino's Wire library, so the default SDA/SCL pins have to be used.

# API

## Constructors
```cpp
// constructor with default address (0x40)
PCA9685();

// constructor with custom address
PCA9685(conconst uint8_t address);
```

## Initialization and configuration
```cpp
// initializes the library
void begin();

// frequency: 24..1526
void setPWMFrequency(uint8_t frequency);

// true: totem pole
// false: open drain
void setOutputMode(bool totempole);
```

## Controling LEDs
```cpp
// number: 0..15
// on/off: 0..4095 for normal operation, 4096 to set always on / always off flag
void setOutput(uint8_t number, uint16_t on, uint16_t off);

// number: 0..15
// pwmvalue: 0..4095
// automatically sets the always on / always off flags
void setOutput(uint8_t number, uint16_t pwmvalue);
```

# Examples

## Minimum viable code
```cpp
PCA9685 pwm = new PCA9685();
pwm.begin();
pwm.setOutput(0, 4095);
```
