#ifndef DCC_DEBUG_LOGGER_H
#define DCC_DEBUG_LOGGER_H

#include <Arduino.h>
#include "../config.h"

/**
 * @brief DCC Debug Logger for managing DCC packet logging and debugging
 * 
 * This class provides a circular buffer for DCC debug messages with
 * timestamps and enables/disables debug mode functionality.
 */
class DccDebugLogger {
private:
    String logBuffer[DCC_LOG_SIZE];
    unsigned long logTimestamp[DCC_LOG_SIZE];
    int logIndex;
    int logCount;
    bool debugEnabled;

public:
    /**
     * @brief Construct a new DCC Debug Logger
     */
    DccDebugLogger();

    /**
     * @brief Add a message to the debug log
     * @param message Debug message to log
     */
    void addMessage(const String& message);

    /**
     * @brief Toggle debug mode on/off
     */
    void toggleDebug();

    /**
     * @brief Enable debug mode
     */
    void enableDebug() { debugEnabled = true; }

    /**
     * @brief Disable debug mode
     */
    void disableDebug() { debugEnabled = false; }

    /**
     * @brief Check if debug mode is enabled
     * @return true if debug is enabled
     */
    bool isDebugEnabled() const { return debugEnabled; }

    /**
     * @brief Get the current log count
     * @return Number of messages in the log
     */
    int getLogCount() const { return logCount; }

    /**
     * @brief Get log message at index
     * @param index Index in the log buffer
     * @return Log message string
     */
    String getLogMessage(int index) const;

    /**
     * @brief Get timestamp for log message at index
     * @param index Index in the log buffer
     * @return Timestamp in milliseconds
     */
    unsigned long getLogTimestamp(int index) const;

    /**
     * @brief Clear all log messages
     */
    void clearLog();

    /**
     * @brief Get formatted log for web interface
     * @return HTML formatted log string
     */
    String getFormattedLogHtml() const;

    /**
     * @brief Print current log to serial console
     */
    void printLogToSerial() const;
};

// Global instance
extern DccDebugLogger dccDebugLogger;

#endif // DCC_DEBUG_LOGGER_H
