#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <Arduino.h>
#include "../hardware/led_controller.h"
#include "../hardware/factory_reset_controller.h"
#include "../utils/dcc_debug_logger.h"

/**
 * @brief System Manager for coordinating all system components
 * 
 * This class acts as the main coordinator for all system components,
 * providing a clean interface for initialization and main loop operations.
 */
class SystemManager {
private:
    LedController* ledController;
    FactoryResetController* factoryResetController;
    
    // Timing variables
    unsigned long currentMs;
    unsigned long previousMs;
    uint16_t tick;
    bool ledState;
    
    // Status variables
    bool isInitialized;

public:
    /**
     * @brief Construct a new System Manager
     */
    SystemManager();

    /**
     * @brief Destroy the System Manager
     */
    ~SystemManager();

    /**
     * @brief Initialize all system components
     */
    void begin();

    /**
     * @brief Main system update loop
     */
    void update();

    /**
     * @brief Get LED controller instance
     * @return Pointer to LED controller
     */
    LedController* getLedController() { return ledController; }

    /**
     * @brief Get factory reset controller instance
     * @return Pointer to factory reset controller
     */
    FactoryResetController* getFactoryResetController() { return factoryResetController; }

    /**
     * @brief Trigger DCC signal indication
     */
    void triggerDccSignal();

    /**
     * @brief Toggle DCC debug mode
     */
    void toggleDccDebug();

    /**
     * @brief Check if system is properly initialized
     * @return true if initialized
     */
    bool isSystemInitialized() const { return isInitialized; }

private:
    /**
     * @brief Initialize hardware components
     */
    void initializeHardware();

    /**
     * @brief Initialize system timing
     */
    void initializeTiming();

    /**
     * @brief Update system timing and servo operations
     */
    void updateTiming();

    /**
     * @brief Factory reset callback function
     */
    void performFactoryReset();
};

// Global system manager instance
extern SystemManager systemManager;

#endif // SYSTEM_MANAGER_H
