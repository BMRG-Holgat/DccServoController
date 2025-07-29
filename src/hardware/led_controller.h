#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Arduino.h>

/**
 * @brief LED Controller for managing heartbeat and DCC signal indication
 * 
 * This class manages the combined heartbeat/DCC signal LED functionality,
 * providing a clean interface for LED state management.
 */
class LedController {
private:
    uint8_t ledPin;
    unsigned long heartbeatPreviousMs;
    unsigned long dccSignalStartMs;
    bool heartbeatState;
    bool dccSignalActive;
    bool isInitialized;

public:
    /**
     * @brief Construct a new Led Controller object
     * @param pin GPIO pin for the LED
     */
    explicit LedController(uint8_t pin);

    /**
     * @brief Initialize the LED controller
     */
    void begin();

    /**
     * @brief Update heartbeat LED state (called in main loop)
     */
    void updateHeartbeat();

    /**
     * @brief Trigger DCC signal indication
     */
    void triggerDccSignal();

    /**
     * @brief Update DCC signal LED state (called in main loop)
     */
    void updateDccSignal();

    /**
     * @brief Blink LED rapidly for indication (blocking)
     * @param count Number of blinks
     * @param delayMs Delay between blinks in milliseconds
     */
    void rapidBlink(int count, int delayMs = 100);

    /**
     * @brief Test LED functionality
     */
    void testLed();

    /**
     * @brief Check if DCC signal is currently active
     * @return true if DCC signal LED is active
     */
    bool isDccSignalActive() const { return dccSignalActive; }

    /**
     * @brief Get current heartbeat state
     * @return true if heartbeat LED should be on
     */
    bool getHeartbeatState() const { return heartbeatState; }
};

#endif // LED_CONTROLLER_H
