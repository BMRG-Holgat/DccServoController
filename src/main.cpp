#include "config.h"
#include "servo_controller.h"
#include "dcc_handler.h"
#include "eeprom_manager.h"
#include "serial_commands.h"
#include "wifi_controller.h"

// Auxiliary variables to store the current output state
String output26State = "off";
String output27State = "off";

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

    // Initialize GPIO pins
    pinMode(output26, OUTPUT);
    pinMode(output27, OUTPUT);
    digitalWrite(output26, LOW);
    digitalWrite(output27, LOW);

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

    // Handle timing and servo updates
    currentMs = millis();
    if (currentMs - previousMs >= SERVO_UPDATE_INTERVAL) {
        previousMs = currentMs;
        ++tick;
        
        if (tick >= LED_BLINK_CYCLES) {
            tick = 0;
            ledState = !ledState;
            // Optional: Blink status LEDs
            // digitalWrite(output26, ledState ? HIGH : LOW);
        }

        // Update all servo positions
        updateServos();
    }

    // Handle serial communication
    recvWithEndMarker();
    processSerialCommands();
    
    // Handle WiFi events
    handleWiFiEvents();
}
