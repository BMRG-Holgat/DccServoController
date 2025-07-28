#ifndef VERSION_H
#define VERSION_H

// Software version information
#define SOFTWARE_VERSION "0.2.0"
#define VERSION_MAJOR 0
#define VERSION_MINOR 2
#define VERSION_PATCH 0

// Numeric version for EEPROM compatibility (MAJOR*100 + MINOR*10 + PATCH)
#define NUMERIC_VERSION 20

// Build information
#define BUILD_DATE "2025-07-28"
#define BUILD_TIME __TIME__

// Project information
#define PROJECT_NAME "ESP32 DCC Servo Controller"
#define PROJECT_AUTHOR "SimplyDCC"
#define PROJECT_URL "https://github.com/BMRG-Holgat/DccServoController"

// Version history and features
#define VERSION_HISTORY \
"v0.2.0 (2025-07-28):\n" \
"  + GitHub repository created with comprehensive documentation\n" \
"  + Professional README.md with GPIO mapping and command reference\n" \
"  + Proper .gitignore for PlatformIO projects\n" \
"  + Complete project setup with version control\n" \
"\n" \
"v0.1.0 (2025-07-28):\n" \
"  + Servo enumeration system (logical numbering 0-15)\n" \
"  + Dual numbering support (servo numbers + GPIO pins)\n" \
"  + Enhanced serial interface with enumerated servo commands\n" \
"  + Immediate servo movement feedback on configuration changes\n" \
"  + User-friendly servo addressing system\n" \
"\n" \
"v0.0.2 (2025-07-28):\n" \
"  + Per-servo offset adjustment (-45 to +45 degrees)\n" \
"  + 4-speed servo movement control (Instant/Fast/Normal/Slow)\n" \
"  + ESP32 servo pin compatibility fix\n" \
"  + Enhanced EEPROM persistence for new features\n" \
"  + Type-safe servo movement calculations\n" \
"\n" \
"v0.0.1 (2025-07-28):\n" \
"  + Initial ESP32 conversion from Arduino Nano\n" \
"  + Modular architecture with 8 source files\n" \
"  + 16 servo support with ESP32-compatible GPIO pins\n" \
"  + DCC accessory decoder functionality\n" \
"  + Serial command interface\n" \
"  + EEPROM configuration storage\n" \
"  + PlatformIO build system"

// Current features summary
#define FEATURE_LIST \
"ESP32 DCC Servo Controller Features:\n" \
"• 16 servo control with ESP32-compatible GPIO pins\n" \
"• DCC accessory decoder integration\n" \
"• 4-speed movement control (Instant/Fast/Normal/Slow)\n" \
"• Per-servo offset adjustment for center position\n" \
"• Dual numbering system (servo 0-15 + GPIO pins)\n" \
"• Comprehensive serial command interface\n" \
"• EEPROM persistence for all settings\n" \
"• Modular architecture for easy maintenance\n" \
"• PlatformIO build system with custom tools\n" \
"• Complete documentation and GitHub integration"

// Hardware specifications
#define HARDWARE_SPECS \
"Hardware Requirements:\n" \
"• ESP32 development board\n" \
"• DCC signal input on GPIO 4\n" \
"• Up to 16 servo motors\n" \
"• 5V power supply for servos\n" \
"• Servo pins: 5,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32"

#endif // VERSION_H
