#include "dcc_debug_logger.h"

// Global instance
DccDebugLogger dccDebugLogger;

DccDebugLogger::DccDebugLogger()
    : logIndex(0)
    , logCount(0)
    , debugEnabled(false) {
}

void DccDebugLogger::addMessage(const String& message) {
    logBuffer[logIndex] = message;
    logTimestamp[logIndex] = millis();
    logIndex = (logIndex + 1) % DCC_LOG_SIZE;
    
    if (logCount < DCC_LOG_SIZE) {
        logCount++;
    }
    
    // Print to serial if debug is enabled
    if (debugEnabled) {
        Serial.println(message);
    }
}

void DccDebugLogger::toggleDebug() {
    debugEnabled = !debugEnabled;
    Serial.printf("DCC Debug: %s\n", debugEnabled ? "ENABLED" : "DISABLED");
}

String DccDebugLogger::getLogMessage(int index) const {
    if (index < 0 || index >= logCount) {
        return "";
    }
    
    // Calculate actual buffer index
    int bufferIndex = (logIndex - logCount + index + DCC_LOG_SIZE) % DCC_LOG_SIZE;
    return logBuffer[bufferIndex];
}

unsigned long DccDebugLogger::getLogTimestamp(int index) const {
    if (index < 0 || index >= logCount) {
        return 0;
    }
    
    // Calculate actual buffer index
    int bufferIndex = (logIndex - logCount + index + DCC_LOG_SIZE) % DCC_LOG_SIZE;
    return logTimestamp[bufferIndex];
}

void DccDebugLogger::clearLog() {
    logIndex = 0;
    logCount = 0;
    Serial.println("DCC debug log cleared");
}

String DccDebugLogger::getFormattedLogHtml() const {
    String html = "<div class='log-container'>";
    
    if (logCount == 0) {
        html += "<p><em>No DCC debug messages logged yet.</em></p>";
    } else {
        html += "<table class='log-table'>";
        html += "<tr><th>Time</th><th>Message</th></tr>";
        
        for (int i = 0; i < logCount; i++) {
            unsigned long timestamp = getLogTimestamp(i);
            String message = getLogMessage(i);
            
            html += "<tr>";
            html += "<td>" + String(timestamp) + "ms</td>";
            html += "<td>" + message + "</td>";
            html += "</tr>";
        }
        
        html += "</table>";
    }
    
    html += "</div>";
    return html;
}

void DccDebugLogger::printLogToSerial() const {
    Serial.println("=== DCC Debug Log ===");
    
    if (logCount == 0) {
        Serial.println("No messages logged.");
    } else {
        for (int i = 0; i < logCount; i++) {
            unsigned long timestamp = getLogTimestamp(i);
            String message = getLogMessage(i);
            Serial.printf("[%lu ms] %s\n", timestamp, message.c_str());
        }
    }
    
    Serial.println("==================");
}
