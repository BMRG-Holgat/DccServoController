#ifndef SERIAL_COMMANDS_H
#define SERIAL_COMMANDS_H

#include "config.h"

// Global serial communication variables
extern char receivedChars[numChars];
extern bool newData;

// Helper functions
bool isValidServoPin(uint8_t pin);
uint8_t validateAndConvertPin(uint8_t inputPin);

// Function declarations
void initializeSerial();
void recvWithEndMarker();
void processSerialCommands();
void processServoConfigCommand();
void processServoControlCommand();
void processDccEmulationCommand();
void processDisplayCommand();
void processAPConfigCommand();
void processStationConfigCommand();
void processFactoryResetCommand();
void processWiFiScanCommand();
void processWiFiStatusCommand();
void processMDNSTestCommand();
void processDccDebugCommand();

#endif // SERIAL_COMMANDS_H
