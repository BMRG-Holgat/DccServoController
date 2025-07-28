#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include <ESP32Servo.h>
#include "config.h"

// Servo states
enum servoState {
    SERVO_NEUTRAL,
    SERVO_TO_THROWN,
    SERVO_THROWN,
    SERVO_TO_CLOSED,
    SERVO_CLOSED,
    SERVO_BOOT
};

// Servo speeds
enum servoSpeed {
    SPEED_INSTANT = 0,  // Move immediately to target position
    SPEED_FAST = 1,     // Move 3 degrees per update (45ms total for 45° swing)
    SPEED_NORMAL = 2,   // Move 2 degrees per update (67.5ms total for 45° swing)  
    SPEED_SLOW = 3      // Move 1 degree per update (135ms total for 45° swing)
};

// Virtual servo structure
struct VIRTUALSERVO {
    uint8_t pin;
    uint16_t address;
    uint8_t swing;
    int8_t offset;      // Offset from center position (-45 to +45 degrees)
    uint8_t speed;      // Movement speed (servoSpeed enum)
    bool invert;
    bool continuous;
    uint8_t state;
    uint8_t position;
    Servo *thisDriver;
};

// Global servo arrays
extern VIRTUALSERVO virtualservo[TOTAL_PINS];
extern VIRTUALSERVO *vsBoot;
extern Servo servoDriver[TOTAL_PINS];

// ESP32 PWM pins array
extern const uint8_t pwmPins[TOTAL_PINS];

// Pin mapping functions
uint8_t getGpioPinFromServoNumber(uint8_t servoNumber);
int8_t getServoNumberFromGpioPin(uint8_t gpioPin);

// Global timing variables
extern unsigned long currentMs;
extern unsigned long previousMs;
extern uint8_t bootTimer;
extern uint8_t tick;
extern bool ledState;

// Function declarations
void initializeServos();
void updateServos();
void moveServoToPosition(VIRTUALSERVO* vs, uint8_t targetPosition);

#endif // SERVO_CONTROLLER_H
