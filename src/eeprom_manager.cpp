#include "eeprom_manager.h"
#include "servo_controller.h"
#include "wifi_controller.h"
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

void saveWiFiConfig() {
    // WiFi config is stored after controller and servo data
    int eeAddr = sizeof(bootController) + sizeof(virtualservo);
    
    // Debug: Show what we're about to save
    Serial.printf("Saving to EEPROM - Mode: %d, Enabled: %s\n", wifiConfig.mode, wifiConfig.enabled ? "true" : "false");
    Serial.printf("Saving Station SSID: '%s'\n", wifiConfig.stationSSID);
    Serial.printf("Saving Station Password: '%s'\n", wifiConfig.stationPassword);
    Serial.printf("Saving AP SSID: '%s'\n", wifiConfig.apSSID);
    Serial.printf("EEPROM address: %d\n", eeAddr);
    
    EEPROM.put(eeAddr, wifiConfig);
    EEPROM.commit(); // ESP32 specific - commit changes to flash
    Serial.println("✅ WiFi configuration saved to EEPROM and committed");
}

void loadWiFiConfig() {
    // WiFi config is stored after controller and servo data
    int eeAddr = sizeof(bootController) + sizeof(virtualservo);
    
    // Create a temporary structure to validate loaded data
    WiFiConfig tempConfig;
    EEPROM.get(eeAddr, tempConfig);
    
    // Debug: Show what was loaded from EEPROM
    Serial.printf("Loaded from EEPROM - Mode: %d, Enabled: %s\n", tempConfig.mode, tempConfig.enabled ? "true" : "false");
    Serial.printf("Loaded Station SSID: '%s'\n", tempConfig.stationSSID);
    Serial.printf("Loaded Station Password: '%s'\n", tempConfig.stationPassword);
    Serial.printf("Loaded AP SSID: '%s'\n", tempConfig.apSSID);
    
    // Check if the loaded config appears to be valid (magic number check would be better)
    // For now, check if the mode is within valid range and enabled flag makes sense
    bool configValid = (tempConfig.mode >= DCC_WIFI_OFF && tempConfig.mode <= DCC_WIFI_STATION);
    
    Serial.printf("Config validation: mode valid = %s, AP SSID length = %d\n", 
                  configValid ? "true" : "false", strlen(tempConfig.apSSID));
    
    if (configValid && strlen(tempConfig.apSSID) > 0) {
        // Config appears valid, use it
        wifiConfig = tempConfig;
        Serial.println("✅ WiFi configuration loaded from EEPROM and validated");
        Serial.printf("Active Mode: %d, Station SSID: '%s'\n", wifiConfig.mode, wifiConfig.stationSSID);
    } else {
        // Config is invalid or uninitialized, set defaults
        Serial.println("WiFi configuration invalid or uninitialized, setting defaults");
        wifiConfig.enabled = true;
        wifiConfig.mode = DCC_WIFI_AP;
        memset(wifiConfig.stationSSID, 0, WIFI_SSID_MAX_LENGTH);
        memset(wifiConfig.stationPassword, 0, WIFI_PASSWORD_MAX_LENGTH);
        wifiConfig.useStaticIP = false;
        
        // Generate default AP credentials
        generateDefaultCredentials();
        
        // Save the defaults to EEPROM
        saveWiFiConfig();
    }
}

void factoryResetAll() {
    Serial.println("Performing factory reset of all settings...");
    
    // Reset controller to defaults
    bootController = m_defaultController;
    bootController.isDirty = true;
    
    // Reset all servos to factory defaults: servo,addr,swing,offset,speed,invert,continuous = 0,0,25,0,0,0
    for (int i = 0; i < TOTAL_PINS; i++) {
        virtualservo[i].pin = pwmPins[i];
        virtualservo[i].address = 0;
        virtualservo[i].swing = 25;
        virtualservo[i].offset = 0;
        virtualservo[i].speed = 0;  // Instant
        virtualservo[i].invert = false;
        virtualservo[i].continuous = false;
        virtualservo[i].position = 90;  // Center position
        virtualservo[i].state = SERVO_BOOT;
    }
    
    // Reset WiFi configuration to defaults
    generateDefaultCredentials();
    wifiConfig.mode = DCC_WIFI_AP;
    wifiConfig.enabled = true;
    memset(wifiConfig.stationSSID, 0, WIFI_SSID_MAX_LENGTH);
    memset(wifiConfig.stationPassword, 0, WIFI_PASSWORD_MAX_LENGTH);
    
    // Save all settings
    putSettings();
    saveWiFiConfig();
    
    Serial.println("Factory reset complete");
}
