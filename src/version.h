#ifndef VERSION_H
#define VERSION_H

// Software version information
#define SOFTWARE_VERSION "v0.4.3"
#define VERSION_MAJOR 0
#define VERSION_MINOR 4
#define VERSION_PATCH 3

// Numeric version for EEPROM compatibility (MAJOR*100 + MINOR*10 + PATCH)
#define VERSION_NUMERIC 403
#define NUMERIC_VERSION 403

// Build information
#define BUILD_DATE "2025-07-30"
#define BUILD_TIME __TIME__

// Project information
#define PROJECT_NAME "ESP32 DCC Servo Controller"
#define PROJECT_AUTHOR "SimplyDCC"
#define PROJECT_URL "https://github.com/BMRG-Holgat/DccServoController"

// Version history and features
#define VERSION_HISTORY \
"v0.4.3 (2025-07-30):\n" \
"  + HOSTNAME CONFIGURATION: Added customizable device hostname for mDNS\n" \
"  + Users can now set custom hostname via web interface and serial commands\n" \
"  + Enhanced mDNS functionality with user-defined hostnames (e.g., mycontroller.local)\n" \
"  + Added 'hostname [name]' serial command for hostname management\n" \
"  + WIFI CORRUPTION FIX: Robust detection and recovery from EEPROM structure changes\n" \
"  + Fixed DHCP issues caused by corrupted WiFi configuration data\n" \
"  + Enhanced EEPROM validation to detect and repair corrupted IP settings\n" \
"  + Automatic reset and recovery when structure corruption is detected\n" \
"  + Improved debugging output for WiFi configuration troubleshooting\n" \
"\n" \
"v0.4.2 (2025-07-30):\n" \
"  + WEB INTERFACE ERROR MESSAGE IMPROVEMENTS: Fixed 'undefined' error messages\n" \
"  + Enhanced Save All Configuration to show proper 'No changes to save' message\n" \
"  + Updated JSON responses to include message field for better user feedback\n" \
"  + Improved JavaScript error handling to distinguish between success, no-changes, and errors\n" \
"  + Better user experience with clear status messages in orange for no-changes scenarios\n" \
"\n" \
"v0.4.1 (2025-01-30):\n" \
"  + ENHANCED SERVO OFFSET VALIDATION: Proper 50% swing angle constraint enforcement\n" \
"  + Added getMaxAllowedOffset() and isValidOffset() helper functions for consistency\n" \
"  + Updated serial commands with improved validation and detailed error messages\n" \
"  + Fixed web interface to use dynamic offset limits based on individual servo swing\n" \
"  + WIFI ARCHITECTURE IMPROVEMENT: Removed AP+Station mode (ESP32 limitation)\n" \
"  + Implemented automatic AP fallback when station connection fails\n" \
"  + Added legacy mode conversion for backward compatibility\n" \
"  + Enhanced all WiFi status displays and configuration interfaces\n" \
"  + Improved offset validation consistency across serial, web, and EEPROM interfaces\n" \
"\n" \
"v0.4.0 (2025-01-29):\n" \
"  + MAJOR CODE REFACTORING: Improved architecture and maintainability\n" \
"  + Created Hardware Abstraction Layer with LedController class\n" \
"  + Added FactoryResetController for button management with callbacks\n" \
"  + Implemented DccDebugLogger for centralized debug log management\n" \
"  + Created SystemManager for coordinated module initialization and updates\n" \
"  + Separated concerns: hardware, utilities, core systems\n" \
"  + Improved error handling and code organization\n" \
"  + Enhanced modularity and testability\n" \
"  + Cleaner main.cpp with reduced complexity\n" \
"  + Better separation of business logic from hardware control\n" \
"\n" \
"v0.3.19 (2025-07-29):\n" \
"  + Improved WiFi test connection error handling\n" \
"  + Graceful handling of 'Failed to fetch' errors during network testing\n" \
"  + Added informative warning message for network switch interruptions\n" \
"  + Enhanced user guidance with explanatory note about test behavior\n" \
"  + Better user experience during WiFi connectivity testing\n" \
"\n" \
"v0.3.18 (2025-07-29):\n" \
"  + Enhanced WiFi error suppression with more comprehensive logging levels\n" \
"  + Added detailed EEPROM save/load debugging for WiFi configuration\n" \
"  + Fixed WiFi mode persistence issue in test connection\n" \
"  + Improved validation and troubleshooting for WiFi credential persistence\n" \
"  + Enhanced debugging output for EEPROM WiFi configuration validation\n" \
"\n" \
"v0.3.17 (2025-07-29):\n" \
"  + Enhanced WiFi test connection EEPROM saving with detailed logging\n" \
"  + Added verification step to confirm credentials are saved correctly\n" \
"  + Automatic WiFi mode adjustment to enable station mode after successful test\n" \
"  + Improved debugging output for WiFi credential persistence\n" \
"  + Fixed potential issue with WiFi mode not enabling saved credentials\n" \
"\n" \
"v0.3.16 (2025-07-29):\n" \
"  + Simplified factory reset button feedback - removed countdown messages\n" \
"  + Factory reset now triggers automatic ESP32 reboot\n" \
"  + Clear initiation and completion messages only\n" \
"  + Added 3-second reboot countdown after factory reset\n" \
"  + Enhanced factory reset reliability with system restart\n" \
"\n" \
"v0.3.15 (2025-07-29):\n" \
"  + Fixed factory reset button countdown timing issue\n" \
"  + Countdown now properly decrements from 10 to 0 seconds\n" \
"  + Eliminated repeated countdown messages at same second\n" \
"  + Improved countdown message timing accuracy\n" \
"  + Enhanced button press feedback reliability\n" \
"\n" \
"v0.3.14 (2025-07-29):\n" \
"  + Added GPIO 0 button factory reset functionality\n" \
"  + Hold BOOT button for 10 seconds to trigger factory reset\n" \
"  + Visual LED feedback during factory reset process\n" \
"  + Console progress updates during button hold\n" \
"  + Automatic WiFi restart after button-triggered reset\n" \
"  + Enhanced user safety with hold-to-confirm mechanism\n" \
"\n" \
"v0.3.13 (2025-07-29):\n" \
"  + Suppressed WiFi error messages (errno 113 connection abort)\n" \
"  + Test connection now automatically saves credentials to EEPROM on success\n" \
"  + Added esp_log configuration to reduce WiFi debugging noise\n" \
"  + Enhanced user experience with automatic credential persistence\n" \
"  + Updated test connection feedback messages\n" \
"\n" \
"v0.3.12 (2025-07-29):\n" \
"  + Fixed WiFi network scanning functionality\n" \
"  + Improved WiFi scan handling for different connection states\n" \
"  + Fixed JavaScript syntax error in selectNetwork() function\n" \
"  + Enhanced WiFi scan error handling and debugging\n" \
"  + Added better WiFi mode management for scanning\n" \
"\n" \
"v0.3.11 (2025-07-29):\n" \
"  + Fixed WiFi credentials persistence - added saveWiFiConfig() calls\n" \
"  + Removed test scan response functionality\n" \
"  + Added password visibility toggle buttons for WiFi credentials\n" \
"  + Added test connection functionality for station SSID/password\n" \
"  + Improved WiFi configuration user experience\n" \
"  + Enhanced error handling and user feedback\n" \
"\n" \
"v0.3.10 (2025-01-29):\n" \
"  + Combined heartbeat and DCC signal indication back to single GPIO 2\n" \
"  + DCC signal flashes take priority over heartbeat (no conflict)\n" \
"  + Heartbeat state preserved and restored after DCC flash\n" \
"  + Simplified to single LED for all status indication\n" \
"\n" \
"v0.3.9 (2025-01-29):\n" \
"  + Separated heartbeat (GPIO 2) and DCC signal (GPIO 33) to independent pins\n" \
"  + Fixed conflict between heartbeat and DCC signal indication\n" \
"  + Added debug output for both LED functions\n" \
"  + Both LEDs now operate independently without interference\n" \
"\n" \
"v0.3.8 (2025-01-29):\n" \
"  + Combined heartbeat and DCC signal indication on single GPIO pin 33\n" \
"  + DCC signal flashes take priority over heartbeat blink pattern\n" \
"  + Heartbeat resumes when DCC signal flash completes\n" \
"  + Moved from GPIO 2 to GPIO 33 for better ESP32 compatibility\n" \
"  + Simplified hardware requirements to single status LED\n" \
"\n" \
"v0.3.7 (2025-01-29):\n" \
"  + Added DCC debug web interface (/dcc-debug) with real-time monitoring\n" \
"  + Web interface shows live DCC packet log with auto-refresh\n" \
"  + Activate/deactivate DCC debug mode from web browser\n" \
"  + Visual indicators for address matches vs ignored packets\n" \
"  + Added circular buffer log for last 50 DCC messages\n" \
"  + Navigation link added to main page for easy access\n" \
"\n" \
"v0.3.6 (2025-01-29):\n" \
"  + Added DCC receiving signal indicator on GPIO pin 2\n" \
"  + DCC signal LED flashes only for configured servo addresses\n" \
"  + Added 'z' command for DCC debug mode toggle\n" \
"  + Enhanced DCC packet monitoring with detailed console output\n" \
"  + Shows DCC address matches and servo actions in debug mode\n" \
"  + Moved heartbeat LED to GPIO pin 33 to free up pin 2\n" \
"  + Filters DCC packets to only process configured addresses\n" \
"\n" \
"v0.3.5 (2025-01-29):\n" \
"  + Added heartbeat LED functionality on GPIO pin 2\n" \
"  + Visual indicator showing device is running (1 second blink rate)\n" \
"  + Independent heartbeat timing from servo update cycles\n" \
"\n" \
"v0.3.4 (2025-01-28):\n" \
"  + Fixed compilation errors with missing ESPmDNS header\n" \
"  + Completed mDNS functionality with proper includes\n" \
"  + Enhanced serial commands now fully functional\n" \
"\n" \
"v0.3.3 (2025-01-28):\n" \
"  + Changed mDNS hostname to simple 'dccservo.local'\n" \
"  + Removed MAC address suffix from hostname for easier access\n" \
"  + Simplified network discovery with memorable hostname\n" \
"\n" \
"v0.3.2 (2025-01-28):\n" \
"  + Added mDNS functionality for network discovery\n" \
"  + Device accessible via friendly hostname (dccservo-[mac].local)\n" \
"  + Enhanced serial console with 'w' command for WiFi status\n" \
"  + Detailed WiFi status shows IP addresses, SSID, channel, signal strength\n" \
"  + Added BSSID, security type, and gateway information to status display\n" \
"  + mDNS service advertisement with device information\n" \
"  + Automatic mDNS restart on WiFi reconnection\n" \
"  + Web interface displays clickable mDNS hostname link\n" \
"\n" \
"v0.3.1 (2025-01-28):\n" \
"  + Fixed WiFi scan dropdown population issues\n" \
"  + Enhanced WiFi scan JavaScript with proper newlines for browser compatibility\n" \
"  + Integrated ArduinoJson library for robust JSON response formatting\n" \
"  + Added CORS headers to WiFi scan endpoint for improved compatibility\n" \
"  + Implemented test WiFi scan functionality for troubleshooting\n" \
"  + Added invert value display to servo control table\n" \
"  + Enhanced servo control table with comprehensive configuration view\n" \
"  + Maintained mobile-responsive design with 7-column servo control table\n" \
"\n" \
"v0.3.0 (2025-07-28):\n" \
"  + WiFi controller with Access Point and Station modes\n" \
"  + Web-based configuration and servo control interface\n" \
"  + Default AP: DCCAC_[MAC6] with password PASS_[MAC6]\n" \
"  + User-configurable WiFi credentials stored in EEPROM\n" \
"  + Factory reset function (clears WiFi and servo settings)\n" \
"  + Serial WiFi commands: wifi, ap, sta, factory\n" \
"  + Web server on port 80 with responsive UI\n" \
"  + Real-time servo control via web interface\n" \
"  + WiFi network scanning with dropdown selection\n" \
"  + Individual servo configuration save functionality\n" \
"  + Mobile-responsive design for all web pages\n" \
"  + Improved servo control table without GPIO references\n" \
"  + Optimized layout with compact padding for better readability\n" \
"  + UTF-8 character encoding support\n" \
"  + Enhanced EEPROM storage (1024 bytes)\n" \
"\n" \
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
"• WiFi Access Point and Station modes\n" \
"• Web-based configuration and servo control\n" \
"• Default AP: DCCAC_[MAC6] / PASS_[MAC6]\n" \
"• User-configurable WiFi credentials (EEPROM stored)\n" \
"• Enhanced WiFi network scanning with improved browser compatibility\n" \
"• Robust JSON response formatting with ArduinoJson library\n" \
"• CORS headers for better web interface compatibility\n" \
"• mDNS network discovery with customizable hostname (hostname.local)\n" \
"• User-configurable device hostname via web interface and serial commands\n" \
"• Automatic mDNS service advertisement with device information\n" \
"• Enhanced serial console with 'w' command for detailed WiFi status\n" \
"• DCC debug mode with 'z' command for packet monitoring\n" \
"• DCC receiving signal indicator on GPIO pin 2 (for valid addresses)\n" \
"• Heartbeat LED indicator on GPIO pin 33 (1 second blink rate)\n" \
"• Factory reset function for complete settings reset\n" \
"• GPIO 0 button factory reset (hold BOOT button for 10 seconds)\n" \
"• Individual servo configuration save capability\n" \
"• Comprehensive servo control table with invert status display\n" \
"• Mobile-responsive web interface design\n" \
"• 4-speed movement control (Instant/Fast/Normal/Slow)\n" \
"• Per-servo offset adjustment for center position\n" \
"• Dual numbering system (servo 0-15 + GPIO pins)\n" \
"• Comprehensive serial and web command interfaces\n" \
"• EEPROM persistence for all settings (1024 bytes)\n" \
"• UTF-8 character encoding support\n" \
"• Modular architecture for easy maintenance\n" \
"• PlatformIO build system with custom tools\n" \
"• Complete documentation and GitHub integration"

// Hardware specifications
#define HARDWARE_SPECS \
"Hardware Requirements:\n" \
"• ESP32 development board\n" \
"• DCC signal input on GPIO 4\n" \
"• Combined heartbeat/DCC indication LED on GPIO 2\n" \
"• Factory reset button on GPIO 0 (BOOT button)\n" \
"• Up to 16 servo motors\n" \
"• 5V power supply for servos\n" \
"• Servo pins: 5,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32"

#endif // VERSION_H
