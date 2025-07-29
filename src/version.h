#ifndef VERSION_H
#define VERSION_H

// Software version information
#define SOFTWARE_VERSION "0.3.4"
#define VERSION_MAJOR 0
#define VERSION_MINOR 3
#define VERSION_PATCH 4

// Numeric version for EEPROM compatibility (MAJOR*100 + MINOR*10 + PATCH)
#define NUMERIC_VERSION 34

// Build information
#define BUILD_DATE "2025-01-29"
#define BUILD_TIME __TIME__

// Project information
#define PROJECT_NAME "ESP32 DCC Servo Controller"
#define PROJECT_AUTHOR "SimplyDCC"
#define PROJECT_URL "https://github.com/BMRG-Holgat/DccServoController"

// Version history and features
#define VERSION_HISTORY \
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
"• mDNS network discovery (accessible via dccservo.local)\n" \
"• Automatic mDNS service advertisement with device information\n" \
"• Enhanced serial console with 'w' command for detailed WiFi status\n" \
"• Factory reset function for complete settings reset\n" \
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
"• Up to 16 servo motors\n" \
"• 5V power supply for servos\n" \
"• Servo pins: 5,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32"

#endif // VERSION_H
