#ifndef EEPROM_MANAGER_H
#define EEPROM_MANAGER_H

#include "config.h"
#include "version.h"

// Create numeric version for EEPROM compatibility (MMmmpp format: Major*10000 + Minor*100 + Patch)
#define NUMERIC_VERSION (VERSION_MAJOR * 10000 + VERSION_MINOR * 100 + VERSION_PATCH)

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
