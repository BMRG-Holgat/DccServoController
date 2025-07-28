#ifndef EEPROM_MANAGER_H
#define EEPROM_MANAGER_H

#include "config.h"
#include "version.h"

// Create numeric version for EEPROM compatibility  
// Note: Using the NUMERIC_VERSION from version.h for consistency
// Format: MAJOR*100 + MINOR*10 + PATCH (to maintain compatibility with existing EEPROM data)

// Controller structure for EEPROM storage
struct CONTROLLER {
    long softwareVersion = NUMERIC_VERSION;  // Numeric version for comparison
    bool isDirty = false;  // Will be true if EEPROM needs a write
    long long padding;     // Fixes a bug on some platforms where EEPROM contents corrupt on readback
};

// Global controller objects
extern CONTROLLER bootController;
extern CONTROLLER m_defaultController;

// Function declarations
void initializeEEPROM();
void getSettings();
void putSettings();

#endif // EEPROM_MANAGER_H
