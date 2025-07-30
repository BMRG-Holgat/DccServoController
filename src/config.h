#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>  // Include Arduino types

// Pin definitions
#define TOTAL_PINS 16
#define BASE_PIN 5  // First servo pin (for backward compatibility)
#define DCC_PIN 4   // GPIO 4 for DCC signal input
#define DCC_SIGNAL_PIN 2   // GPIO 2 for DCC receiving signal indicator (shared with heartbeat)
#define HEARTBEAT_PIN 2   // GPIO 2 for heartbeat LED indicator

// Note: Servo pins are not consecutive due to ESP32 hardware limitations
// Valid servo pins: 5,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32
// These map to logical servo numbers 0-15 for user interface

// Helper functions for pin mapping
// Convert logical servo number (0-15) to actual GPIO pin
// Convert actual GPIO pin to logical servo number (0-15, or -1 if invalid)

// Output pins
const int output26 = 26;
const int output27 = 27;

// Serial configuration
#define SERIAL_BAUD 115200
const uint8_t numChars = 32;

// EEPROM configuration
#define EEPROM_SIZE 1024  // Increased size for WiFi configuration storage

// Timing constants
#define SERVO_UPDATE_INTERVAL 15  // milliseconds
#define LED_BLINK_CYCLES 33       // 15ms * 33 = ~495ms
#define HEARTBEAT_INTERVAL 1000   // milliseconds - heartbeat blink rate
#define DCC_SIGNAL_DURATION 100   // milliseconds - DCC signal LED on duration
#define DCC_LOG_SIZE 50           // DCC debug log buffer size

// Servo constants
#define SERVO_CENTER_POSITION 90  // Default center position (degrees)
#define SERVO_MAX_OFFSET 45       // Absolute maximum offset from center (+/- degrees)
                                  // Note: Actual offset limit is 50% of swing angle, whichever is smaller

#endif // CONFIG_H
