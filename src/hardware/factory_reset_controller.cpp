#include "factory_reset_controller.h"

FactoryResetController::FactoryResetController(uint8_t pin, unsigned long holdTimeMs)
    : buttonPin(pin)
    , holdTimeMs(holdTimeMs)
    , buttonPressStartMs(0)
    , buttonPressed(false)
    , resetTriggered(false)
    , isInitialized(false)
    , resetCallback(nullptr) {
}

void FactoryResetController::begin() {
    pinMode(buttonPin, INPUT_PULLUP);
    isInitialized = true;
    
    Serial.printf("Factory reset button initialized on GPIO pin %d (hold for %lu seconds)\n", 
                  buttonPin, holdTimeMs / 1000);
}

void FactoryResetController::setResetCallback(FactoryResetCallback callback) {
    resetCallback = callback;
}

void FactoryResetController::update() {
    if (!isInitialized || resetTriggered) return;
    
    bool currentButtonState = (digitalRead(buttonPin) == LOW);
    unsigned long currentMs = millis();
    
    if (currentButtonState && !buttonPressed) {
        // Button just pressed - start timing
        buttonPressStartMs = currentMs;
        buttonPressed = true;
        Serial.println("🔘 Factory reset initiated - hold BOOT button for 10 seconds...");
        
    } else if (!currentButtonState && buttonPressed) {
        // Button released before timeout
        buttonPressed = false;
        Serial.println("🔘 Factory reset cancelled - button released");
        
    } else if (currentButtonState && buttonPressed) {
        // Button still pressed - check if held long enough
        unsigned long holdTime = currentMs - buttonPressStartMs;
        
        if (holdTime >= holdTimeMs) {
            performFactoryReset();
        }
    }
}

void FactoryResetController::performFactoryReset() {
    resetTriggered = true;
    buttonPressed = false;
    
    Serial.println("🔄 Factory reset activated! Resetting all settings to defaults...");
    
    // Execute the callback if provided
    if (resetCallback) {
        resetCallback();
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
