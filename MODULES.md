# Module Documentation

## Configuration Module (config.h)
Centralized configuration constants and pin definitions.

### Key Definitions:
- `TOTAL_PINS` - Number of servo pins (16)
- `BASE_PIN` - First servo pin (GPIO 5)
- `DCC_PIN` - DCC input pin (GPIO 4)
- `SERVO_UPDATE_INTERVAL` - Servo update timing (15ms)
- `LED_BLINK_CYCLES` - LED timing cycles
- `SERVO_CENTER_POSITION` - Default servo center (90°)
- `SERVO_MAX_OFFSET` - Maximum offset range (±45°)

## version.h
Version and build information constants:
- `SOFTWARE_VERSION` - Current version (semantic versioning: "0.0.1")
- `VERSION_MAJOR`, `VERSION_MINOR`, `VERSION_PATCH` - Numeric version components
- `BUILD_DATE` - Build date string
- `PROJECT_NAME` - Project name and description

## Servo Controller Module (servo_controller.h/cpp)
Manages servo positioning, movement, and state transitions.

### Key Functions:
- `initializeServos()` - Initialize ESP32 PWM timers
- `updateServos()` - Process servo state machine (called every 15ms)

### Servo States:
- `SERVO_NEUTRAL` - Servo at 90° center position
- `SERVO_TO_CLOSED` - Moving to closed position
- `SERVO_CLOSED` - At closed position
- `SERVO_TO_THROWN` - Moving to thrown position
- `SERVO_THROWN` - At thrown position
- `SERVO_BOOT` - Initial boot sequence

## DCC Handler Module (dcc_handler.h/cpp)
Processes DCC packets and converts them to servo commands.

### Key Functions:
- `initializeDCC()` - Initialize DCC decoder on GPIO 4
- `processDCC()` - Process incoming DCC packets
- `notifyDccAccTurnoutOutput()` - Handle accessory decoder commands

### DCC Configuration:
- Supports standard DCC accessory decoder addressing
- Addresses 1-2048 supported
- Direction: 0=closed, 1=thrown

## EEPROM Manager Module (eeprom_manager.h/cpp)
Handles persistent storage of servo configurations.

### Key Functions:
- `initializeEEPROM()` - Initialize ESP32 EEPROM emulation
- `getSettings()` - Load settings from EEPROM on boot
- `putSettings()` - Save settings to EEPROM when modified

### Storage Structure:
- Controller metadata (version, dirty flag)
- Array of servo configurations (pin, address, swing, etc.)

## Serial Commands Module (serial_commands.h/cpp)
Provides command-line interface for configuration and testing.

### Key Functions:
- `initializeSerial()` - Initialize serial communication at 115200 baud
- `recvWithEndMarker()` - Parse incoming serial data
- `processSerialCommands()` - Execute commands

### Commands:
- `s pin,addr,swing,invert,continuous` - Configure servo
- `p pin,command` - Manual servo control
- `d address,command` - DCC command emulation
- `x` - Display all configurations
- `h` - Help

## Main Module (main.cpp)
Coordinates all modules and provides the main program loop.

### Setup Sequence:
1. Initialize serial communication
2. Initialize EEPROM and load settings
3. Initialize GPIO pins
4. Initialize servo system
5. Initialize DCC system

### Main Loop:
1. Process DCC packets
2. Update servo positions (every 15ms)
3. Process serial commands

## Inter-Module Communication
- Global variables declared in headers, defined in .cpp files
- Shared data structures (VIRTUALSERVO array)
- Function calls between modules for coordinated actions
- Event-driven architecture for DCC and serial input

## Error Handling
- Input validation in serial commands
- Range checking for pin numbers and DCC addresses
- Graceful fallbacks for invalid configurations
- Serial feedback for all operations
