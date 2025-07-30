#include "servo_controller.h"

// Global servo arrays
VIRTUALSERVO virtualservo[TOTAL_PINS];
VIRTUALSERVO *vsBoot = nullptr;
Servo servoDriver[TOTAL_PINS];

// ESP32 PWM pins array - Using valid ESP32 servo pins only
// Available servo pins: 2,4,5,12-19,21-23,25-27,32-33
// Note: Pin 4 is reserved for DCC input, Pin 2 could interfere with boot
const uint8_t pwmPins[TOTAL_PINS] = {
    5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32
};

// Pin mapping functions
uint8_t getGpioPinFromServoNumber(uint8_t servoNumber) {
    if (servoNumber < TOTAL_PINS) {
        return pwmPins[servoNumber];
    }
    return 0;  // Invalid servo number
}

int8_t getServoNumberFromGpioPin(uint8_t gpioPin) {
    for (uint8_t i = 0; i < TOTAL_PINS; i++) {
        if (pwmPins[i] == gpioPin) {
            return i;
        }
    }
    return -1;  // Invalid GPIO pin
}

// Calculate maximum allowed offset for a given swing angle
// Offset cannot exceed 50% of swing or the absolute maximum, whichever is smaller
uint8_t getMaxAllowedOffset(uint8_t swing) {
    uint8_t halfSwing = swing / 2;
    return (halfSwing < SERVO_MAX_OFFSET) ? halfSwing : SERVO_MAX_OFFSET;
}

// Validate if an offset is within allowed range for a given swing
bool isValidOffset(int8_t offset, uint8_t swing) {
    uint8_t maxAllowed = getMaxAllowedOffset(swing);
    return (abs(offset) <= maxAllowed);
}

// Global timing variables
unsigned long currentMs;
unsigned long previousMs;
uint8_t bootTimer = 0;
uint8_t tick;
bool ledState;

void initializeServos() {
    // Allow allocation of all timers for ESP32Servo
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
}

void updateServos() {
    // Update all moving servos every 15mS
    // In normal non-invert mode, minPosition is turnout closed, and maxPosition is turnout thrown
    for (auto &vs : virtualservo) {
        uint8_t centerPosition = 90 + vs.offset;  // Apply offset to center position
        uint8_t maxPosition = centerPosition + vs.swing;
        uint8_t minPosition = centerPosition - vs.swing;
        
        switch (vs.state) {
        case SERVO_NEUTRAL:
            vs.position = centerPosition;  // Use offset center position
            if (!vs.thisDriver->attached()) vs.thisDriver->attach(vs.pin);
            break;
            
        case SERVO_TO_CLOSED:
            // Swing toward minPosition, unless invert is true
            if (vs.invert) {
                if (vs.speed == SPEED_INSTANT) {
                    vs.position = maxPosition;
                } else {
                    uint8_t step = (vs.speed == SPEED_FAST) ? 3 : (vs.speed == SPEED_NORMAL) ? 2 : 1;
                    uint8_t remaining = (vs.position < maxPosition) ? (maxPosition - vs.position) : 0;
                    vs.position += (remaining > 0) ? (step < remaining ? step : remaining) : 0;
                }
            } else {
                if (vs.speed == SPEED_INSTANT) {
                    vs.position = minPosition;
                } else {
                    uint8_t step = (vs.speed == SPEED_FAST) ? 3 : (vs.speed == SPEED_NORMAL) ? 2 : 1;
                    uint8_t remaining = (vs.position > minPosition) ? (vs.position - minPosition) : 0;
                    vs.position -= (remaining > 0) ? (step < remaining ? step : remaining) : 0;
                }
            }
        
            if ((vs.position == maxPosition) || (vs.position == minPosition)) {
                vs.state = SERVO_CLOSED;
            }

            if (!vs.thisDriver->attached()) vs.thisDriver->attach(vs.pin);
            break;
        
        case SERVO_TO_THROWN:
            // Swing toward maxPosition unless invert is true
            if (vs.invert) {
                if (vs.speed == SPEED_INSTANT) {
                    vs.position = minPosition;
                } else {
                    uint8_t step = (vs.speed == SPEED_FAST) ? 3 : (vs.speed == SPEED_NORMAL) ? 2 : 1;
                    uint8_t remaining = (vs.position > minPosition) ? (vs.position - minPosition) : 0;
                    vs.position -= (remaining > 0) ? (step < remaining ? step : remaining) : 0;
                }
            } else {
                if (vs.speed == SPEED_INSTANT) {
                    vs.position = maxPosition;
                } else {
                    uint8_t step = (vs.speed == SPEED_FAST) ? 3 : (vs.speed == SPEED_NORMAL) ? 2 : 1;
                    uint8_t remaining = (vs.position < maxPosition) ? (maxPosition - vs.position) : 0;
                    vs.position += (remaining > 0) ? (step < remaining ? step : remaining) : 0;
                }
            }

            if ((vs.position == maxPosition) || (vs.position == minPosition)) {
                vs.state = SERVO_THROWN;
            }

            if (!vs.thisDriver->attached()) vs.thisDriver->attach(vs.pin);
            break;

        case SERVO_THROWN:
            vs.position = vs.invert ? minPosition : maxPosition;
            if ((vs.thisDriver->attached()) && (!vs.continuous)) {
                vs.thisDriver->detach();
            }
            break;
            
        case SERVO_CLOSED:
            vs.position = vs.invert ? maxPosition : minPosition;
            if ((vs.thisDriver->attached()) && (!vs.continuous)) {
                vs.thisDriver->detach();
            }
            break;

        case SERVO_BOOT:
            if (vsBoot == nullptr) {
                // Handle next-up servo to boot. Servos are booted in the CLOSED position
                vsBoot = &vs;
                bootTimer = 34;
                vs.position = vs.invert ? maxPosition : minPosition;
                if (!vs.thisDriver->attached()) vs.thisDriver->attach(vs.pin);
                vs.thisDriver->write(vs.position);
            } else if (vsBoot == &vs) { 
                // If this is the current boot-servo, then decrement bootTimer
                bootTimer -= bootTimer > 0 ? 1 : 0;
                
                // Timed out?
                if (bootTimer == 0) {
                    vs.state = SERVO_CLOSED;
                    Serial.print("pin booted: ");
                    Serial.println(vs.pin, DEC);
                    // Release for next vs to boot
                    vsBoot = nullptr;
                }
            }
            break;
        }

        vs.thisDriver->write(vs.position);
    }
}
