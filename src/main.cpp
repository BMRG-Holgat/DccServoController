#include "config.h"
#include "servo_controller.h"
#include "dcc_handler.h"
#include "eeprom_manager.h"
#include "serial_commands.h"
#include "wifi_controller.h"
#include "core/system_manager.h"
#include "utils/dcc_debug_logger.h"
#include <WiFi.h>

// Auxiliary variables to store the current output state
String output26State = "off";
String output27State = "off";

// Function to trigger DCC signal indication
void triggerDccSignal() {
    systemManager.triggerDccSignal();
}

// Function to add message to DCC debug log (for backward compatibility)
void addDccLogMessage(const String& message) {
    dccDebugLogger.addMessage(message);
}

// Function to enable/disable DCC debug (for backward compatibility)
void toggleDccDebug() {
    systemManager.toggleDccDebug();
}

void setup() {
    // Initialize serial communication
    initializeSerial();
    
    // Initialize EEPROM
    initializeEEPROM();
    
    // Restore servo array from EEPROM
    getSettings();
    
    // Load WiFi configuration from EEPROM
    loadWiFiConfig();
    
    Serial.println("Boot complete\n");

    // Initialize system manager (handles LED, factory reset, etc.)
    systemManager.begin();

    // Initialize servo system
    initializeServos();

    // Initialize DCC system
    initializeDCC();
    
    // Initialize WiFi system
    initializeWiFi();
}

void loop() {
    // Process DCC packets
    processDCC();

    // Update system manager (handles timing, LED, factory reset, etc.)
    systemManager.update();
    
    // Handle serial communication
    recvWithEndMarker();
    processSerialCommands();
    
    // Handle WiFi events
    handleWiFiEvents();
}
