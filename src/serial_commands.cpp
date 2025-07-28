#include "serial_commands.h"
#include "servo_controller.h"
#include "eeprom_manager.h"
#include "config.h"
#include "version.h"

// Helper function to check if pin is valid and convert servo number to GPIO if needed
uint8_t validateAndConvertPin(uint8_t inputPin) {
    // If input is 0-15, treat as servo number and convert to GPIO
    if (inputPin < TOTAL_PINS) {
        return getGpioPinFromServoNumber(inputPin);
    }
    
    // Otherwise, check if it's a valid GPIO pin
    if (isValidServoPin(inputPin)) {
        return inputPin;
    }
    
    return 0;  // Invalid pin
}

// Helper function to check if pin is valid
bool isValidServoPin(uint8_t pin) {
    for (int i = 0; i < TOTAL_PINS; i++) {
        if (pwmPins[i] == pin) {
            return true;
        }
    }
    return false;
}

// Global serial communication variables
char receivedChars[numChars];
bool newData = false;

void initializeSerial() {
    Serial.begin(SERIAL_BAUD);
    delay(1000);
    Serial.print(PROJECT_NAME);
    Serial.print(" v");
    Serial.println(SOFTWARE_VERSION);
    Serial.println("Commands: s p x d v");
    Serial.println("Type 'h' for help");
}

void recvWithEndMarker() {
    static uint8_t ndx = 0;
    char endMarker = '\n';
    char rc;

    while ((Serial.available() > 0) && (newData == false)) {
        rc = Serial.read();
        
        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        } else {
            receivedChars[ndx] = '\0'; // Terminate the string
            ndx = 0;
            newData = true;
        }
    }
}

void processSerialCommands() {
    if (!newData) return;
    
    newData = false;
    
    switch (receivedChars[0]) {
        case 's':
            processServoConfigCommand();
            break;
        case 'p':
            processServoControlCommand();
            break;
        case 'd':
            processDccEmulationCommand();
            break;
        case 'x':
            processDisplayCommand();
            break;
        case 'v':
            Serial.println("=== Version Information ===");
            Serial.print("Software Version: ");
            Serial.println(SOFTWARE_VERSION);
            Serial.print("Build Date: ");
            Serial.println(BUILD_DATE);
            Serial.print("Project: ");
            Serial.println(PROJECT_NAME);
            Serial.print("Numeric Version: ");
            Serial.println(NUMERIC_VERSION);
            break;
        case 'h':
        case '?':
            Serial.println("Commands:");
            Serial.println("s servo,addr,swing,offset,speed,invert,continuous - Configure servo");
            Serial.println("p servo,command - Manual control (c=closed, t=thrown, T=toggle, n=neutral)");
            Serial.println("d address,command - DCC emulation");
            Serial.println("x - Display all servo configurations");
            Serial.println("v - Show version information");
            Serial.println();
            Serial.println("Servo numbers: 0-15 (maps to GPIO pins automatically)");
            Serial.println("GPIO pins can also be used directly");
            Serial.println("Speed: 0=Instant, 1=Fast, 2=Normal, 3=Slow");
            break;
        case 'r':
            Serial.println("Virtual routes not yet implemented");
            break;
        default:
            Serial.println("Unknown command. Type 'h' for help.");
            break;
    }
}

void processServoConfigCommand() {
    // Command format: s servo,addr,swing,offset,speed,invert,continuous
    VIRTUALSERVO vsParse;
    char *pch;
    int i = 0;
    pch = strtok(receivedChars, " ,");
    
    while (pch != NULL) {
        switch (i) {
            case 1:
                {
                    uint8_t inputPin = atoi(pch);
                    vsParse.pin = validateAndConvertPin(inputPin);
                    if (vsParse.pin == 0) {
                        i = 10;  // Error code
                        Serial.println("Error: Invalid servo number/pin");
                        Serial.println("Valid: 0-15 (servo numbers) or GPIO: 5,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32");
                    }
                }
                break;
            case 2:
                vsParse.address = atoi(pch);
                // If out of range 1-2048 then throw an error
                if ((vsParse.address > 2048) || (vsParse.address == 0)) {
                    i = 10;
                    Serial.println("Error: Invalid DCC address (1-2048)");
                }
                break;
            case 3:
                vsParse.swing = atoi(pch);
                if (vsParse.swing > 90) {
                    i = 10;
                    Serial.println("Error: Invalid swing range (max 90 degrees)");
                }
                break;
            case 4:
                vsParse.offset = atoi(pch);
                if ((vsParse.offset < -SERVO_MAX_OFFSET) || (vsParse.offset > SERVO_MAX_OFFSET)) {
                    i = 10;
                    Serial.println("Error: Invalid offset range (-45 to +45 degrees)");
                }
                // Additional validation: offset cannot exceed 50% of swing value
                if (abs(vsParse.offset) > (vsParse.swing / 2)) {
                    i = 10;
                    Serial.println("Error: Offset cannot exceed 50% of swing value");
                }
                break;
            case 5:
                vsParse.speed = atoi(pch);
                if (vsParse.speed > SPEED_SLOW) {
                    i = 10;
                    Serial.println("Error: Invalid speed (0=Instant, 1=Fast, 2=Normal, 3=Slow)");
                }
                break;
            case 6:
                vsParse.invert = atoi(pch) == 0 ? false : true;
                break;
            case 7:
                vsParse.continuous = atoi(pch) == 0 ? false : true;
                break;
        }
        ++i;
        pch = strtok(NULL, " ,");
    }

    if (i != 8) {
        Serial.println("Error: Invalid command format");
        Serial.println("Usage: s servo,addr,swing,offset,speed,invert,continuous");
        Serial.println("Note: Offset cannot exceed 50% of swing value");
        Serial.println("Speed: 0=Instant, 1=Fast, 2=Normal, 3=Slow");
        Serial.println("Example: s 0,100,25,0,2,0,0  (servo 0, normal speed)");
        Serial.println("Example: s 5,101,30,5,1,0,0  (GPIO 5, fast speed)");
    } else {
        Serial.println("OK - Servo configured");
        
        // Match to a pin member of servo slot and copy it over
        for (auto &vs : virtualservo) {
            if (vs.pin == vsParse.pin) {
                // First copy servo-driver pointer to vsParse
                vsParse.thisDriver = vs.thisDriver;
                // Then copy vsParse to virtualservo[]
                vs = vsParse;
                
                // Set servo to closed position when configuration changes
                uint8_t centerPosition = 90 + vs.offset;  // Apply offset to center position
                if (vs.invert) {
                    vs.position = centerPosition + vs.swing;  // Max position for inverted
                } else {
                    vs.position = centerPosition - vs.swing;  // Min position for normal
                }
                vs.state = SERVO_TO_CLOSED;
                
                // Immediately attach servo and start movement to closed position
                if (!vs.thisDriver->attached()) {
                    vs.thisDriver->attach(vs.pin);
                }
                
                Serial.print("Servo ");
                Serial.print(getServoNumberFromGpioPin(vs.pin));
                Serial.print(" moving to closed position (");
                Serial.print(vs.position);
                Serial.println("°)");
                
                // Write to EEPROM
                bootController.isDirty = true;
                putSettings();
                break;
            }
        }
    }
}

void processServoControlCommand() {
    // Command format: p pin,command
    char *pch;
    int i = 0;
    pch = strtok(receivedChars, " ,");
    int p = -1;
    VIRTUALSERVO *targetVirtualServo = nullptr;

    while (pch != NULL) {
        switch (i) {
            case 1:
                // Pin or servo number
                {
                    uint8_t inputPin = atoi(pch);
                    p = validateAndConvertPin(inputPin);
                    if (p == 0) {
                        i = 10;
                        Serial.println("Error: Invalid servo number/pin");
                        Serial.println("Valid: 0-15 (servo numbers) or GPIO: 5,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32");
                        p = -1;
                        break;
                    }
                    // p is valid, use this to lookup the servo slot
                    for (auto &vs : virtualservo) {
                        if (vs.pin != p) continue;
                        targetVirtualServo = &vs;
                    }
                }
                break;

            case 2:
                if (targetVirtualServo == nullptr) { 
                    i = 10;
                    break; 
                }
                switch (pch[0]) {
                    case 'c':
                        targetVirtualServo->state = SERVO_TO_CLOSED;
                        break;
                    case 't':
                        targetVirtualServo->state = SERVO_TO_THROWN;
                        break;
                    case 'n':
                        targetVirtualServo->state = SERVO_NEUTRAL;
                        break;
                    case 'T':
                        if (targetVirtualServo->state == SERVO_CLOSED) {
                            targetVirtualServo->state = SERVO_TO_THROWN;
                        } else {
                            targetVirtualServo->state = SERVO_TO_CLOSED;
                        }
                        break;
                }
                break;
        }
        ++i;
        pch = strtok(NULL, " ,");
    }

    if (i == 3) {
        Serial.println("OK - Servo command executed");
    } else {
        Serial.println("Error: Invalid command format");
        Serial.println("Usage: p servo,command");
        Serial.println("Commands: c=closed, t=thrown, T=toggle, n=neutral");
        Serial.println("Example: p 0,t  (servo 0, thrown)");
        Serial.println("Example: p 12,c (GPIO 12, closed)");
    }
}

void processDccEmulationCommand() {
    // Command format: d address,command
    char *pch;
    int i = 0;
    pch = strtok(receivedChars, " ,");
    int a = -1;
    
    while (pch != NULL) {
        switch (i) {
            case 1:
                // Address
                a = atoi(pch);
                if ((a < 1) || (a > 2048)) {
                    i = 10;
                    Serial.println("Error: Invalid DCC address (1-2048)");
                    break;
                }
                break;

            case 2:
                // Command
                for (auto &vs : virtualservo) {
                    if (vs.address != a) continue;

                    switch (pch[0]) {
                        case 't':
                            vs.state = SERVO_TO_THROWN;
                            break;
                        case 'n':
                            vs.state = SERVO_NEUTRAL;
                            break;
                        case 'T':
                            if (vs.state == SERVO_CLOSED) {
                                vs.state = SERVO_TO_THROWN;
                            } else {
                                vs.state = SERVO_TO_CLOSED;
                            }
                            break;
                        default:
                            vs.state = SERVO_TO_CLOSED;
                            break;
                    }
                }
                break;
        }
        ++i;
        pch = strtok(NULL, " ,");
    }

    if (i == 3) {
        Serial.println("OK - DCC command emulated");
    } else {
        Serial.println("Error: Invalid command format");
        Serial.println("Usage: d address,command");
        Serial.println("Commands: c=closed, t=thrown, T=toggle, n=neutral");
        Serial.println("Example: d 100,c");
    }
}

void processDisplayCommand() {
    Serial.println("Servo Configuration:");
    Serial.println("Servo\tGPIO\tAddr\tSwing\tOffset\tSpeed\tInvert\tCont\tStatus");
    Serial.println("-----\t----\t----\t-----\t------\t-----\t------\t----\t------");
    
    const char* speedNames[] = {"Instant", "Fast", "Normal", "Slow"};
    
    for (uint8_t i = 0; i < TOTAL_PINS; i++) {
        auto vs = virtualservo[i];
        int8_t servoNum = getServoNumberFromGpioPin(vs.pin);
        
        Serial.print(servoNum, DEC);
        Serial.print("\t");
        Serial.print(vs.pin, DEC);
        Serial.print("\t");
        Serial.print(vs.address, DEC);
        Serial.print("\t");
        Serial.print(vs.swing, DEC);
        Serial.print("\t");
        Serial.print(vs.offset, DEC);
        Serial.print("\t");
        Serial.print(speedNames[vs.speed]);
        Serial.print("\t");
        Serial.print(vs.invert, DEC);
        Serial.print("\t");
        Serial.print(vs.continuous, DEC);
        Serial.print("\t");
        
        if (vs.thisDriver == nullptr) {
            Serial.println("No Driver");
        } else {
            Serial.println("OK");
        }
    }
}
