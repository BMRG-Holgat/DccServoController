# Module Documentation

## Architecture Overview (v0.4.0)
The ESP32 DCC Servo Controller has been refactored into a clean, modular architecture with clear separation of concerns:

- **Core**: System coordination and management
- **Hardware**: Hardware abstraction layer for GPIO, LEDs, buttons
- **Utils**: Utility classes for logging and debugging
- **Business Logic**: Servo control, DCC handling, WiFi management

## Core Modules

### System Manager (core/system_manager.h/cpp)
Central coordinator for all system components with clean initialization and update lifecycle.

**Key Features:**
- Coordinates all hardware and software modules
- Manages system timing and servo updates
- Provides clean separation between main.cpp and business logic
- Handles factory reset callbacks and system state

**Key Functions:**
- `begin()` - Initialize all system components
- `update()` - Main system update loop
- `triggerDccSignal()` - Coordinate DCC signal indication
- `toggleDccDebug()` - Toggle debug mode

## Hardware Abstraction Layer

### LED Controller (hardware/led_controller.h/cpp)
Manages heartbeat and DCC signal indication with proper state management.

**Key Features:**
- Combined heartbeat/DCC signal LED functionality
- Non-blocking LED state management
- Priority system (DCC signal takes precedence over heartbeat)
- LED testing and rapid blink functionality

**Key Functions:**
- `begin()` - Initialize LED hardware
- `updateHeartbeat()` - Update heartbeat LED state
- `triggerDccSignal()` - Trigger DCC signal indication
- `rapidBlink()` - Rapid blinking for visual feedback

### Factory Reset Controller (hardware/factory_reset_controller.h/cpp)
Manages GPIO button-based factory reset with proper timing and callbacks.

**Key Features:**
- Non-blocking button press detection
- Configurable hold time (default: 10 seconds)
- Callback-based architecture for clean separation
- Automatic system restart after reset

**Key Functions:**
- `begin()` - Initialize button hardware
- `update()` - Non-blocking button state monitoring
- `setResetCallback()` - Set factory reset callback function

## Utility Modules

### DCC Debug Logger (utils/dcc_debug_logger.h/cpp)
Centralized logging system for DCC debug messages with circular buffer.

**Key Features:**
- Circular buffer for efficient memory usage
- Timestamp tracking for all messages
- Debug mode enable/disable functionality
- Web interface integration for log display

**Key Functions:**
- `addMessage()` - Add debug message to log
- `toggleDebug()` - Toggle debug mode
- `getFormattedLogHtml()` - Get HTML formatted log for web interface
- `clearLog()` - Clear all log messages

## Configuration Module (config.h)
Centralized configuration constants and pin definitions.

### Key Definitions:
- `TOTAL_PINS` - Number of servo pins (16)
- `BASE_PIN` - First servo pin (GPIO 5)
- `DCC_PIN` - DCC input pin (GPIO 4)
- `HEARTBEAT_PIN` - LED pin (GPIO 2)
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
