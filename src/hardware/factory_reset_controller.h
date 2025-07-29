#ifndef FACTORY_RESET_CONTROLLER_H
#define FACTORY_RESET_CONTROLLER_H

#include <Arduino.h>
#include <functional>

/**
 * @brief Factory Reset Controller for managing GPIO button-based factory reset
 * 
 * This class manages the factory reset button functionality with proper timing,
 * state management, and callback support for clean separation of concerns.
 */
class FactoryResetController {
public:
    // Callback function type for factory reset execution
    using FactoryResetCallback = std::function<void()>;

private:
    uint8_t buttonPin;
    unsigned long holdTimeMs;
    unsigned long buttonPressStartMs;
    bool buttonPressed;
    bool resetTriggered;
    bool isInitialized;
    FactoryResetCallback resetCallback;

public:
    /**
     * @brief Construct a new Factory Reset Controller
     * @param pin GPIO pin for the factory reset button
     * @param holdTimeMs Time in milliseconds button must be held
     */
    explicit FactoryResetController(uint8_t pin, unsigned long holdTimeMs = 10000);

    /**
     * @brief Initialize the factory reset controller
     */
    void begin();

    /**
     * @brief Set the callback function to execute on factory reset
     * @param callback Function to call when factory reset is triggered
     */
    void setResetCallback(FactoryResetCallback callback);

    /**
     * @brief Update factory reset button state (called in main loop)
     */
    void update();

    /**
     * @brief Check if factory reset is currently in progress
     * @return true if reset was triggered
     */
    bool isResetTriggered() const { return resetTriggered; }

    /**
     * @brief Get the hold time requirement in seconds
     * @return Hold time in seconds
     */
    unsigned long getHoldTimeSeconds() const { return holdTimeMs / 1000; }

private:
    /**
     * @brief Execute the factory reset sequence
     */
    void performFactoryReset();
};

#endif // FACTORY_RESET_CONTROLLER_H
