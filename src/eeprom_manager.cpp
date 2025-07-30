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
    
    Serial.printf("DEBUG: WiFiConfig structure size: %d bytes\n", sizeof(WiFiConfig));
    Serial.printf("DEBUG: Loading WiFi config from EEPROM address: %d\n", eeAddr);
    
    // Create a temporary structure to validate loaded data
    WiFiConfig tempConfig;
    EEPROM.get(eeAddr, tempConfig);
    
    // Debug: Show what was loaded from EEPROM
    Serial.printf("Loaded from EEPROM - Mode: %d, Enabled: %s\n", tempConfig.mode, tempConfig.enabled ? "true" : "false");
    Serial.printf("Loaded Station SSID: '%s'\n", tempConfig.stationSSID);
    Serial.printf("Loaded Station Password: '%s'\n", tempConfig.stationPassword);
    Serial.printf("Loaded AP SSID: '%s'\n", tempConfig.apSSID);
    
    // Check if the loaded config appears to be valid
    // After adding hostname field, old configs may be corrupted, so be more strict
    bool configValid = (tempConfig.mode >= DCC_WIFI_OFF && tempConfig.mode <= DCC_WIFI_STATION);
    
    // Additional validation: check if SSID contains reasonable characters
    bool ssidValid = true;
    for (int i = 0; i < WIFI_SSID_MAX_LENGTH && tempConfig.apSSID[i] != '\0'; i++) {
        if (tempConfig.apSSID[i] < 32 || tempConfig.apSSID[i] > 126) {
            ssidValid = false;
            break;
        }
    }
    
    // Check for corrupted IP addresses (DNS servers appearing in wrong fields)
    bool ipValid = true;
    uint32_t staticIPValue = (uint32_t)tempConfig.staticIP;
    uint32_t gatewayValue = (uint32_t)tempConfig.gateway;
    
    // Detect if DNS servers (8.8.8.8 or 8.8.4.4) are in static IP or gateway fields
    // This indicates structure corruption from the hostname field addition
    if (staticIPValue == IPAddress(8, 8, 4, 4) || staticIPValue == IPAddress(8, 8, 8, 8) ||
        gatewayValue == IPAddress(8, 8, 4, 4) || gatewayValue == IPAddress(8, 8, 8, 8)) {
        Serial.println("⚠ CORRUPTION DETECTED: DNS servers found in IP config fields");
        Serial.printf("Static IP: %s, Gateway: %s\n", 
                      tempConfig.staticIP.toString().c_str(), tempConfig.gateway.toString().c_str());
        ipValid = false;
    }
    
    // Check hostname field for corruption
    bool hostnameValid = true;
    if (strlen(tempConfig.hostname) > 0) {
        for (int i = 0; i < strlen(tempConfig.hostname) && i < WIFI_HOSTNAME_MAX_LENGTH; i++) {
            char c = tempConfig.hostname[i];
            if (c < 32 || c > 126) {
                hostnameValid = false;
                break;
            }
        }
    }
    
    // Overall validation - all checks must pass
    configValid = configValid && ssidValid && ipValid && hostnameValid;
    
    // Check for completely garbage data (common after structure changes)
    if (!ssidValid || strlen(tempConfig.apSSID) == 0 || !ipValid) {
        Serial.println("WiFi config appears corrupted (likely due to structure change), forcing reset");
        configValid = false;
    }
    
    Serial.printf("Config validation: mode=%s, SSID=%s, IP=%s, hostname=%s, overall=%s\n", 
                  (tempConfig.mode >= DCC_WIFI_OFF && tempConfig.mode <= DCC_WIFI_STATION) ? "OK" : "BAD",
                  ssidValid ? "OK" : "BAD", 
                  ipValid ? "OK" : "BAD",
                  hostnameValid ? "OK" : "BAD",
                  configValid ? "VALID" : "INVALID");
    
    if (configValid && strlen(tempConfig.apSSID) > 0) {
        // Config appears valid, use it
        wifiConfig = tempConfig;
        
        // Ensure hostname is set (for backward compatibility with older configs)
        if (strlen(wifiConfig.hostname) == 0) {
            strncpy(wifiConfig.hostname, "dccservo", WIFI_HOSTNAME_MAX_LENGTH - 1);
            wifiConfig.hostname[WIFI_HOSTNAME_MAX_LENGTH - 1] = '\0';
            Serial.println("Set default hostname for existing configuration");
            saveWiFiConfig();  // Save the updated config with hostname
        }
        
        Serial.println("✅ WiFi configuration loaded from EEPROM and validated");
        Serial.printf("Active Mode: %d, Station SSID: '%s'\n", wifiConfig.mode, wifiConfig.stationSSID);
        Serial.printf("Hostname: %s\n", wifiConfig.hostname);
    } else {
        // Config is invalid or uninitialized, set defaults
        Serial.println("WiFi configuration invalid or uninitialized, setting defaults");
        wifiConfig.enabled = true;
        wifiConfig.mode = DCC_WIFI_AP;
        memset(wifiConfig.stationSSID, 0, WIFI_SSID_MAX_LENGTH);
        memset(wifiConfig.stationPassword, 0, WIFI_PASSWORD_MAX_LENGTH);
        memset(wifiConfig.hostname, 0, WIFI_HOSTNAME_MAX_LENGTH);
        strncpy(wifiConfig.hostname, "dccservo", WIFI_HOSTNAME_MAX_LENGTH - 1);
        wifiConfig.hostname[WIFI_HOSTNAME_MAX_LENGTH - 1] = '\0';
        wifiConfig.useStaticIP = false;
        
        // Reset all IP fields to proper defaults (not corrupted DNS addresses)
        wifiConfig.staticIP = IPAddress(192, 168, 1, 100);
        wifiConfig.gateway = IPAddress(192, 168, 1, 1);
        wifiConfig.subnet = IPAddress(255, 255, 255, 0);
        wifiConfig.dns1 = IPAddress(8, 8, 8, 8);
        wifiConfig.dns2 = IPAddress(8, 8, 4, 4);
        
        Serial.printf("Reset to defaults - Hostname: %s\n", wifiConfig.hostname);
        Serial.printf("Reset to defaults - Static IP: %s, Gateway: %s\n", 
                      wifiConfig.staticIP.toString().c_str(), wifiConfig.gateway.toString().c_str());
        
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
    strncpy(wifiConfig.hostname, "dccservo", WIFI_HOSTNAME_MAX_LENGTH - 1);
    wifiConfig.hostname[WIFI_HOSTNAME_MAX_LENGTH - 1] = '\0';
    
    // Save all settings
    putSettings();
    saveWiFiConfig();
    
    Serial.println("Factory reset complete");
}
