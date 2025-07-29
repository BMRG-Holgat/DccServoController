#include "system_manager.h"
#include "../config.h"
#include "../servo_controller.h"
#include "../eeprom_manager.h"
#include "../wifi_controller.h"

// Global instance
SystemManager systemManager;

SystemManager::SystemManager()
    : ledController(nullptr)
    , factoryResetController(nullptr)
    , currentMs(0)
    , previousMs(0)
    , tick(0)
    , ledState(false)
    , isInitialized(false) {
}

SystemManager::~SystemManager() {
    delete ledController;
    delete factoryResetController;
}

void SystemManager::begin() {
    Serial.println("🚀 Initializing System Manager...");
    
    initializeHardware();
    initializeTiming();
    
    isInitialized = true;
    Serial.println("✅ System Manager initialization complete");
}

void SystemManager::initializeHardware() {
    // Initialize LED controller
    ledController = new LedController(HEARTBEAT_PIN);
    ledController->begin();
    
    // Initialize factory reset controller
    factoryResetController = new FactoryResetController(0, 10000); // GPIO 0, 10 seconds
    factoryResetController->begin();
    
    // Set up factory reset callback
    factoryResetController->setResetCallback([this]() {
        this->performFactoryReset();
    });
    
    // Initialize other GPIO pins
    pinMode(output26, OUTPUT);
    pinMode(output27, OUTPUT);
    digitalWrite(output26, LOW);
    digitalWrite(output27, LOW);
    
    Serial.println("Hardware initialization complete");
}

void SystemManager::initializeTiming() {
    currentMs = millis();
    previousMs = currentMs;
    tick = 0;
    ledState = false;
    
    Serial.println("Timing system initialized");
}

void SystemManager::update() {
    if (!isInitialized) return;
    
    // Update current time
    currentMs = millis();
    
    // Update timing-based operations
    updateTiming();
    
    // Update hardware controllers
    ledController->updateHeartbeat();
    ledController->updateDccSignal();
    factoryResetController->update();
}

void SystemManager::updateTiming() {
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
}

void SystemManager::triggerDccSignal() {
    if (ledController) {
        ledController->triggerDccSignal();
    }
    
    // Add to debug log
    dccDebugLogger.addMessage("DCC signal triggered");
}

void SystemManager::toggleDccDebug() {
    dccDebugLogger.toggleDebug();
}

void SystemManager::performFactoryReset() {
    Serial.println("🔄 Executing factory reset sequence...");
    
    // Blink LED rapidly to indicate factory reset in progress
    if (ledController) {
        ledController->rapidBlink(10, 100);
    }
    
    // Perform the actual factory reset
    factoryResetAll();
    
    Serial.println("Factory reset sequence completed");
}
