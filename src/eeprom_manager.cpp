#include "eeprom_manager.h"
#include "servo_controller.h"
#include "config.h"
#include <EEPROM.h>

// Global controller objects
CONTROLLER bootController;
CONTROLLER m_defaultController;

void initializeEEPROM() {
    // Initialize EEPROM with specified size (ESP32 compatible)
    EEPROM.begin(EEPROM_SIZE);
}

void getSettings() {
    int eeAddr = 0;
    EEPROM.get(eeAddr, bootController);
    
    if (m_defaultController.softwareVersion != bootController.softwareVersion) {
        // If software version has changed, we need to re-initialize EEPROM with factory defaults
        Serial.println("Restoring factory defaults");
        EEPROM.put(0, m_defaultController);
        eeAddr += sizeof(m_defaultController);
        
        // Use valid ESP32 servo pins from pwmPins array. Set defaults
        int i = 0;
        for (auto &s : virtualservo) {
            s.pin = pwmPins[i];  // Use actual valid servo pins
            s.address = 0;  // Default to no DCC address assigned
            s.invert = 0;
            s.position = 90;
            s.swing = 25;
            s.offset = 0;  // Default offset
            s.speed = SPEED_NORMAL;  // Default to normal speed
            s.continuous = 0;
            s.state = SERVO_BOOT;
            ++i;
        }
        
        // Write back default values
        EEPROM.put(eeAddr, virtualservo);
        EEPROM.commit(); // ESP32 specific - commit changes to flash
    }

    // Either way, now populate our structs with EEPROM values
    eeAddr = 0;
    EEPROM.get(eeAddr, bootController);
    eeAddr += sizeof(bootController);
    EEPROM.get(eeAddr, virtualservo);
    
    // Initialize the pin assignments and servo drivers
    int i = 0;
    
    for (auto &s : virtualservo) {
        s.pin = pwmPins[i];  // Use valid ESP32 servo pins
        s.state = SERVO_BOOT;
        
        // Minimum useful swing is 5 degrees
        if ((s.swing < 5) || (s.swing > 90)) s.swing = 25;  // Default to 25 degrees
        
        // Ensure offset is within valid range
        if ((s.offset < -SERVO_MAX_OFFSET) || (s.offset > SERVO_MAX_OFFSET)) s.offset = 0;
        
        // Ensure speed is valid
        if (s.speed > SPEED_SLOW) s.speed = SPEED_NORMAL;
        
        // Calculate closed position (we may be inverted) then back off 5 degrees and set that as position
        uint8_t centerPosition = 90 + s.offset;  // Apply offset to center position
        if (s.invert) {
            // Max position
            s.position = centerPosition + s.swing - 5;
        } else {
            // Min position, normal for closed
            s.position = centerPosition - s.swing + 5;
        }

        // Initialize the servo driver
        servoDriver[i].detach();  // Don't attach at this time as it will assert an unhelpful position
        s.thisDriver = &servoDriver[i];
        ++i;
    }
    
    Serial.print("\nSoftware version: ");
    Serial.print(bootController.softwareVersion, DEC);
    Serial.println("\n...............\n");
}

void putSettings() {
    int eeAddr = 0;
    if (bootController.isDirty == false) { 
        return; 
    }
    
    EEPROM.put(eeAddr, bootController);
    eeAddr += sizeof(bootController);
    EEPROM.put(eeAddr, virtualservo);
    EEPROM.commit(); // ESP32 specific - commit changes to flash
    
    Serial.println("Settings saved to EEPROM");
    bootController.isDirty = false;
}
