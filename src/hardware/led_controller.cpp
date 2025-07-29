#include "led_controller.h"
#include "../config.h"

LedController::LedController(uint8_t pin) 
    : ledPin(pin)
    , heartbeatPreviousMs(0)
    , dccSignalStartMs(0)
    , heartbeatState(false)
    , dccSignalActive(false)
    , isInitialized(false) {
}

void LedController::begin() {
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    isInitialized = true;
    
    Serial.printf("LED Controller initialized on GPIO pin %d\n", ledPin);
    
    // Test LED during initialization
    testLed();
}

void LedController::updateHeartbeat() {
    if (!isInitialized) return;
    
    unsigned long currentMs = millis();
    if (currentMs - heartbeatPreviousMs >= HEARTBEAT_INTERVAL) {
        heartbeatPreviousMs = currentMs;
        heartbeatState = !heartbeatState;
        
        // Only update LED if DCC signal is not currently active
        if (!dccSignalActive) {
            digitalWrite(ledPin, heartbeatState ? HIGH : LOW);
        }
    }
}

void LedController::triggerDccSignal() {
    if (!isInitialized) return;
    
    dccSignalStartMs = millis();
    dccSignalActive = true;
    digitalWrite(ledPin, HIGH);
    
    Serial.println("DCC signal LED triggered");
}

void LedController::updateDccSignal() {
    if (!isInitialized || !dccSignalActive) return;
    
    unsigned long currentMs = millis();
    if (currentMs - dccSignalStartMs >= DCC_SIGNAL_DURATION) {
        dccSignalActive = false;
        Serial.println("DCC signal LED ended, restoring heartbeat state");
        
        // Restore heartbeat state when DCC signal ends
        digitalWrite(ledPin, heartbeatState ? HIGH : LOW);
    }
}

void LedController::rapidBlink(int count, int delayMs) {
    if (!isInitialized) return;
    
    for (int i = 0; i < count; i++) {
        digitalWrite(ledPin, HIGH);
        delay(delayMs);
        digitalWrite(ledPin, LOW);
        delay(delayMs);
    }
}

void LedController::testLed() {
    if (!isInitialized) return;
    
    Serial.println("Testing LED functionality...");
    rapidBlink(3, 200);
    Serial.println("LED test completed");
}
