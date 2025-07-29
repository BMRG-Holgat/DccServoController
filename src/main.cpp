#include "config.h"
#include "servo_controller.h"
#include "dcc_handler.h"
#include "eeprom_manager.h"
#include "serial_commands.h"
#include "wifi_controller.h"
#include <WiFi.h>

// Auxiliary variables to store the current output state
String output26State = "off";
String output27State = "off";

// Heartbeat variables
unsigned long heartbeatPreviousMs = 0;
bool heartbeatState = false;

// DCC signal indication variables
unsigned long dccSignalStartMs = 0;
bool dccSignalActive = false;
bool dccDebugEnabled = false;

// Factory reset button variables (GPIO 0)
#define FACTORY_RESET_PIN 0
#define FACTORY_RESET_HOLD_TIME 10000  // 10 seconds in milliseconds
unsigned long factoryResetButtonPressStartMs = 0;
bool factoryResetButtonPressed = false;
bool factoryResetTriggered = false;

// DCC debug log - circular buffer
String dccLogBuffer[DCC_LOG_SIZE];
int dccLogIndex = 0;
int dccLogCount = 0;
unsigned long dccLogTimestamp[DCC_LOG_SIZE];

// Function to add message to DCC debug log
void addDccLogMessage(const String& message) {
    dccLogBuffer[dccLogIndex] = message;
    dccLogTimestamp[dccLogIndex] = millis();
    dccLogIndex = (dccLogIndex + 1) % DCC_LOG_SIZE;
    if (dccLogCount < DCC_LOG_SIZE) {
        dccLogCount++;
    }
}

// Function to trigger DCC signal indication
void triggerDccSignal() {
    dccSignalStartMs = millis();
    dccSignalActive = true;
    digitalWrite(HEARTBEAT_PIN, HIGH);  // Use same pin as heartbeat
    Serial.println("DCC signal triggered");
}

// Function to enable/disable DCC debug
void toggleDccDebug() {
    dccDebugEnabled = !dccDebugEnabled;
    Serial.printf("DCC Debug: %s\n", dccDebugEnabled ? "ENABLED" : "DISABLED");
}

// Function to perform factory reset via button
void performButtonFactoryReset() {
    Serial.println("🔄 Factory reset activated! Resetting all settings to defaults...");
    
    // Perform the factory reset
    factoryResetAll();
    
    // Blink LED rapidly to indicate factory reset in progress
    for (int i = 0; i < 10; i++) {
        digitalWrite(HEARTBEAT_PIN, HIGH);
        delay(100);
        digitalWrite(HEARTBEAT_PIN, LOW);
        delay(100);
    }
    
    Serial.println("✅ Factory reset complete! System will reboot in 3 seconds...");
    
    // Countdown to reboot
    for (int i = 3; i > 0; i--) {
        Serial.printf("⏳ Rebooting in %d...\n", i);
        delay(1000);
    }
    
    Serial.println("🔄 Rebooting now...");
    delay(500);
    
    // Restart the ESP32
    ESP.restart();
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

    // Initialize GPIO pins
    pinMode(output26, OUTPUT);
    pinMode(output27, OUTPUT);
    digitalWrite(output26, LOW);
    digitalWrite(output27, LOW);
    
    // Initialize combined heartbeat/DCC signal LED on pin 2
    pinMode(HEARTBEAT_PIN, OUTPUT);
    digitalWrite(HEARTBEAT_PIN, LOW);
    Serial.printf("Combined heartbeat/DCC signal LED initialized on GPIO pin %d\n", HEARTBEAT_PIN);
    
    // Initialize factory reset button on GPIO 0 (BOOT button)
    pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);
    Serial.printf("Factory reset button initialized on GPIO pin %d (hold for %d seconds)\n", FACTORY_RESET_PIN, FACTORY_RESET_HOLD_TIME / 1000);
    
    // Test LED by blinking it during setup
    for (int i = 0; i < 3; i++) {
        digitalWrite(HEARTBEAT_PIN, HIGH);
        delay(200);
        digitalWrite(HEARTBEAT_PIN, LOW);
        delay(200);
    }
    Serial.println("LED test completed");

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
    
    // Handle heartbeat LED (unless DCC signal is active)
    if (currentMs - heartbeatPreviousMs >= HEARTBEAT_INTERVAL) {
        heartbeatPreviousMs = currentMs;
        heartbeatState = !heartbeatState;
        // Only update heartbeat if DCC signal is not currently active
        if (!dccSignalActive) {
            digitalWrite(HEARTBEAT_PIN, heartbeatState ? HIGH : LOW);
        }
    }
    
    // Handle DCC signal indication LED (takes priority over heartbeat)
    if (dccSignalActive && (currentMs - dccSignalStartMs >= DCC_SIGNAL_DURATION)) {
        dccSignalActive = false;
        Serial.println("DCC signal ended, restoring heartbeat state");
        // When DCC signal ends, restore heartbeat state
        digitalWrite(HEARTBEAT_PIN, heartbeatState ? HIGH : LOW);
    }

    // Handle serial communication
    recvWithEndMarker();
    processSerialCommands();
    
    // Handle factory reset button (GPIO 0)
    bool buttonPressed = (digitalRead(FACTORY_RESET_PIN) == LOW);
    
    if (buttonPressed && !factoryResetButtonPressed && !factoryResetTriggered) {
        // Button just pressed - start timing
        factoryResetButtonPressStartMs = currentMs;
        factoryResetButtonPressed = true;
        Serial.println("🔘 Factory reset initiated - hold BOOT button for 10 seconds...");
    } else if (!buttonPressed && factoryResetButtonPressed) {
        // Button released before timeout
        factoryResetButtonPressed = false;
        Serial.println("🔘 Factory reset cancelled - button released");
    } else if (buttonPressed && factoryResetButtonPressed && !factoryResetTriggered) {
        // Button still pressed - check if held long enough
        unsigned long holdTime = currentMs - factoryResetButtonPressStartMs;
        
        if (holdTime >= FACTORY_RESET_HOLD_TIME) {
            performButtonFactoryReset();
            factoryResetButtonPressed = false;
        }
    }
    
    // Handle WiFi events
    handleWiFiEvents();
}
