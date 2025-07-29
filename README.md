# ESP32 DCC Servo Controller

A sophisticated DCC (Digital Command Control) servo controller for model railways, built for ESP32 microcontrollers using PlatformIO.

## Features

- **16 Servo Control**: Controls up to 16 servos using ESP32-compatible GPIO pins
- **DCC Integration**: Responds to DCC accessory decoder commands
- **Flexible Configuration**: Per-servo settings for swing, offset, speed, and inversion
- **Speed Control**: Four speed settings (Instant, Fast, Normal, Slow)
- **Serial Interface**: Complete command-line interface for configuration and testing
- **EEPROM Storage**: Persistent configuration storage
- **Dual Numbering**: Supports both logical servo numbers (0-15) and GPIO pin numbers

## Hardware Requirements

- ESP32 development board
- Up to 16 servo motors
- DCC signal input on GPIO 4
- 5V power supply for servos

## GPIO Pin Mapping

The controller uses ESP32-compatible servo pins mapped to logical servo numbers:

| Servo # | GPIO | Servo # | GPIO |
|---------|------|---------|------|
| 0       | 5    | 8       | 19   |
| 1       | 12   | 9       | 21   |
| 2       | 13   | 10      | 22   |
| 3       | 14   | 11      | 23   |
| 4       | 15   | 12      | 25   |
| 5       | 16   | 13      | 26   |
| 6       | 17   | 14      | 27   |
| 7       | 18   | 15      | 32   |

## Installation

### Prerequisites
- [PlatformIO](https://platformio.org/) installed
- ESP32 development environment set up

### Setup
1. Clone this repository:
   ```bash
   git clone <repository-url>
   cd DccServoController
   ```

2. Build the project:
   ```bash
   platformio run
   ```

3. Upload to ESP32:
   ```bash
   platformio run --target upload
   ```

4. Monitor serial output:
   ```bash
   platformio device monitor
   ```

## Usage

### Serial Commands

Connect to the ESP32 via serial at 115200 baud and use these commands:

#### Servo Configuration
```
s servo,addr,swing,offset,speed,invert,continuous
```
- `servo`: Servo number (0-15) or GPIO pin
- `addr`: DCC address (1-2048)
- `swing`: Movement range in degrees (0-90)
- `offset`: Center position offset (-45 to +45)
- `speed`: Movement speed (0=Instant, 1=Fast, 2=Normal, 3=Slow)
- `invert`: 0=Normal, 1=Inverted operation
- `continuous`: 0=Detach when idle, 1=Always attached

**Examples:**
```
s 0,100,25,0,2,0,0    # Servo 0, DCC addr 100, ±25°, normal speed
s 5,101,30,5,1,0,0    # GPIO 5, DCC addr 101, ±30°, +5° offset, fast
```

#### Manual Control
```
p servo,command
```
- `command`: c=closed, t=thrown, T=toggle, n=neutral

**Examples:**
```
p 0,t    # Move servo 0 to thrown position
p 5,c    # Move GPIO 5 servo to closed position
```

#### DCC Emulation
```
d address,command
```
**Examples:**
```
d 100,c    # Send close command to DCC address 100
d 101,t    # Send throw command to DCC address 101
```

#### Display Configuration
```
x    # Show all servo configurations
v    # Show version information
h    # Show help
```

### Configuration Parameters

#### Swing Range
- Range: 0-90 degrees
- Defines how far the servo moves from center position

#### Offset
- Range: -45 to +45 degrees  
- Fine-tunes the center position
- Cannot exceed 50% of swing value
- Useful for mechanical alignment

#### Speed Settings
- **Instant (0)**: Immediate movement
- **Fast (1)**: 3°/update (~45ms for 45° swing)
- **Normal (2)**: 2°/update (~67ms for 45° swing)
- **Slow (3)**: 1°/update (~135ms for 45° swing)

## Architecture

The project is organized into modular components:

- **`main.cpp`**: Main program coordination
- **`config.h`**: Hardware and timing configuration
- **`version.h`**: Version information
- **`servo_controller.*`**: Servo movement logic and hardware control
- **`dcc_handler.*`**: DCC signal processing
- **`eeprom_manager.*`**: Configuration persistence
- **`serial_commands.*`**: Command-line interface

## Version History

- **v0.1.0**: Servo enumeration system and dual numbering support
- **v0.0.2**: Servo pin compatibility fix and offset feature
- **v0.0.1**: Initial ESP32 conversion and modular structure

## Development

### Building
Use the provided batch files for easy development:
- `build.bat` - Build project
- `upload.bat` - Upload to ESP32
- `monitor.bat` - Open serial monitor
- `build_menu.bat` - Interactive build menu

### Testing
Configure and test servos using the serial interface:
1. Connect via serial monitor
2. Configure a servo: `s 0,100,25,0,2,0,0`
3. Test movement: `p 0,t`
4. View configuration: `x`

## Contributing

1. Follow the existing code structure
2. Update version numbers for releases
3. Test thoroughly with real hardware
4. Document any new features

## License

[Add your license information here]

## Hardware Notes

- DCC signal input on GPIO 4
- Servo pins are non-consecutive due to ESP32 hardware limitations
- 5V power supply recommended for servo motors
- Common ground required between ESP32 and servo power supply

## Troubleshooting

### Servo Not Moving
- Check GPIO pin assignment
- Verify servo power supply
- Check serial output for error messages

### DCC Not Working
- Verify DCC signal on GPIO 4
- Check DCC address configuration
- Monitor serial output for DCC packet information

### Configuration Not Saving
- Check EEPROM initialization messages
- Verify power supply stability
- Use `x` command to verify saved settings
