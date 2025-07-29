#include "serial_commands.h"
#include "servo_controller.h"
#include "eeprom_manager.h"
#include "wifi_controller.h"
#include "config.h"
#include "version.h"
#include <esp_wifi.h>
#include <ESPmDNS.h>

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
    Serial.println("Commands: s p x d v w z");
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
            Serial.println("=== ESP32 DCC Servo Controller ===");
            Serial.print("Software Version: ");
            Serial.println(SOFTWARE_VERSION);
            Serial.print("Build Date: ");
            Serial.println(BUILD_DATE);
            Serial.print("Build Time: ");
            Serial.println(BUILD_TIME);
            Serial.print("Project: ");
            Serial.println(PROJECT_NAME);
            Serial.print("Author: ");
            Serial.println(PROJECT_AUTHOR);
            Serial.print("GitHub: ");
            Serial.println(PROJECT_URL);
            Serial.print("Numeric Version: ");
            Serial.println(NUMERIC_VERSION);
            Serial.println();
            Serial.println(FEATURE_LIST);
            Serial.println();
            Serial.println(HARDWARE_SPECS);
            break;
        case 'w':
            processWiFiStatusCommand();
            break;
        case 'z':
            processDccDebugCommand();
            break;
        case 'h':
        case '?':
            Serial.println("Commands:");
            Serial.println("s servo,addr,swing,offset,speed,invert,continuous - Configure servo");
            Serial.println("p servo,command - Manual control (c=closed, t=thrown, T=toggle, n=neutral)");
            Serial.println("d address,command - DCC emulation");
            Serial.println("x - Display all servo configurations");
            Serial.println("v - Show version and feature information");
            Serial.println("w - Show WiFi status (IP, SSID, channel, mDNS)");
            Serial.println("z - Toggle DCC debug mode (monitor DCC packets)");
            Serial.println("wifi - Show detailed WiFi configuration");
            Serial.println("scan - Scan for available WiFi networks");
            Serial.println("ap ssid,password - Configure Access Point");
            Serial.println("sta ssid,password - Configure Station mode");
            Serial.println("factory - Factory reset (clears WiFi and servo settings)");
            Serial.println("history - Show version history and changelog");
            Serial.println("mdns - Test mDNS functionality and restart if needed");
            Serial.println();
            Serial.println("Servo numbers: 0-15 (maps to GPIO pins automatically)");
            Serial.println("GPIO pins can also be used directly");
            Serial.println("Speed: 0=Instant, 1=Fast, 2=Normal, 3=Slow");
            break;
        case 'r':
            Serial.println("Virtual routes not yet implemented");
            break;
        default:
            // Check for multi-character commands
            String command = String(receivedChars);
            if (command.startsWith("wifi")) {
                printWiFiStatus();
            } else if (command.startsWith("scan")) {
                processWiFiScanCommand();
            } else if (command.startsWith("ap ")) {
                processAPConfigCommand();
            } else if (command.startsWith("sta ")) {
                processStationConfigCommand();
            } else if (command.startsWith("factory")) {
                processFactoryResetCommand();
            } else if (command.startsWith("history")) {
                Serial.println("=== Version History & Changelog ===");
                Serial.println(VERSION_HISTORY);
            } else if (command.startsWith("mdns")) {
                processMDNSTestCommand();
            } else {
                Serial.println("Unknown command. Type 'h' for help.");
            }
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

void processAPConfigCommand() {
    // Command format: ap ssid,password
    char *pch;
    char *ssid = nullptr;
    char *password = nullptr;
    int i = 0;
    
    pch = strtok(receivedChars, " ,");
    while (pch != NULL) {
        switch (i) {
            case 1:
                ssid = pch;
                break;
            case 2:
                password = pch;
                break;
        }
        ++i;
        pch = strtok(NULL, " ,");
    }
    
    if (ssid != nullptr && password != nullptr) {
        strncpy(wifiConfig.apSSID, ssid, WIFI_SSID_MAX_LENGTH - 1);
        wifiConfig.apSSID[WIFI_SSID_MAX_LENGTH - 1] = '\0';
        strncpy(wifiConfig.apPassword, password, WIFI_PASSWORD_MAX_LENGTH - 1);
        wifiConfig.apPassword[WIFI_PASSWORD_MAX_LENGTH - 1] = '\0';
        
        wifiConfig.mode = DCC_WIFI_AP;
        
        bootController.isDirty = true;
        putSettings();
        saveWiFiConfig();
        
        Serial.printf("AP configuration updated: SSID=%s, Password=%s\n", ssid, password);
        Serial.println("Restarting WiFi...");
        
        WiFi.disconnect();
        delay(1000);
        initializeWiFi();
    } else {
        Serial.println("Error: Usage: ap ssid,password");
    }
}

void processStationConfigCommand() {
    // Command format: sta ssid,password
    char *pch;
    char *ssid = nullptr;
    char *password = nullptr;
    int i = 0;
    
    pch = strtok(receivedChars, " ,");
    while (pch != NULL) {
        switch (i) {
            case 1:
                ssid = pch;
                break;
            case 2:
                password = pch;
                break;
        }
        ++i;
        pch = strtok(NULL, " ,");
    }
    
    if (ssid != nullptr && password != nullptr) {
        strncpy(wifiConfig.stationSSID, ssid, WIFI_SSID_MAX_LENGTH - 1);
        wifiConfig.stationSSID[WIFI_SSID_MAX_LENGTH - 1] = '\0';
        strncpy(wifiConfig.stationPassword, password, WIFI_PASSWORD_MAX_LENGTH - 1);
        wifiConfig.stationPassword[WIFI_PASSWORD_MAX_LENGTH - 1] = '\0';
        
        wifiConfig.mode = DCC_WIFI_STATION;
        
        bootController.isDirty = true;
        putSettings();
        saveWiFiConfig();
        
        Serial.printf("Station configuration updated: SSID=%s, Password=%s\n", ssid, password);
        Serial.println("Restarting WiFi...");
        
        WiFi.disconnect();
        delay(1000);
        initializeWiFi();
    } else {
        Serial.println("Error: Usage: sta ssid,password");
    }
}

void processFactoryResetCommand() {
    Serial.println("Are you sure you want to perform a factory reset? (y/N)");
    Serial.println("This will reset all WiFi settings and servo configurations.");
    
    // Wait for user confirmation
    unsigned long startTime = millis();
    while (millis() - startTime < 10000) {  // 10 second timeout
        if (Serial.available()) {
            char response = Serial.read();
            if (response == 'y' || response == 'Y') {
                factoryResetAll();
                Serial.println("Factory reset complete. Restarting WiFi...");
                WiFi.disconnect();
                delay(1000);
                initializeWiFi();
                return;
            } else {
                Serial.println("Factory reset cancelled.");
                return;
            }
        }
        delay(100);
    }
    Serial.println("Factory reset cancelled (timeout).");
}

void processWiFiScanCommand() {
    Serial.println("Scanning for WiFi networks...");
    
    int numNetworks = WiFi.scanNetworks();
    
    if (numNetworks > 0) {
        Serial.printf("Found %d networks:\n", numNetworks);
        Serial.println("SSID\t\t\tRSSI\tChannel\tEncryption");
        Serial.println("----------------------------------------");
        
        for (int i = 0; i < numNetworks; i++) {
            String ssid = WiFi.SSID(i);
            int32_t rssi = WiFi.RSSI(i);
            uint8_t channel = WiFi.channel(i);
            wifi_auth_mode_t encryption = WiFi.encryptionType(i);
            
            // Pad SSID for alignment
            String paddedSSID = ssid;
            while (paddedSSID.length() < 24) {
                paddedSSID += " ";
            }
            
            String encType;
            switch (encryption) {
                case WIFI_AUTH_OPEN: encType = "Open"; break;
                case WIFI_AUTH_WEP: encType = "WEP"; break;
                case WIFI_AUTH_WPA_PSK: encType = "WPA"; break;
                case WIFI_AUTH_WPA2_PSK: encType = "WPA2"; break;
                case WIFI_AUTH_WPA_WPA2_PSK: encType = "WPA/WPA2"; break;
                case WIFI_AUTH_WPA2_ENTERPRISE: encType = "WPA2-Enterprise"; break;
                case WIFI_AUTH_WPA3_PSK: encType = "WPA3"; break;
                default: encType = "Unknown"; break;
            }
            
            Serial.printf("%s\t%d\t%d\t%s\n", 
                         paddedSSID.c_str(), 
                         rssi, 
                         channel, 
                         encType.c_str());
        }
        
        Serial.println("----------------------------------------");
        Serial.println("Use 'sta SSID,PASSWORD' to connect to a network");
    } else {
        Serial.println("No networks found");
    }
    
    // Clean up scan results
    WiFi.scanDelete();
}

void processWiFiStatusCommand() {
    Serial.println("=== WiFi Status ===");
    
    // Show current mode
    Serial.printf("Mode: ");
    switch (wifiConfig.mode) {
        case DCC_WIFI_OFF:
            Serial.println("Disabled");
            break;
        case DCC_WIFI_AP:
            Serial.println("Access Point Only");
            break;
        case DCC_WIFI_STATION:
            Serial.println("Station Only");
            break;
        case DCC_WIFI_AP_STATION:
            Serial.println("AP + Station");
            break;
        default:
            Serial.println("Unknown");
            break;
    }
    
    Serial.printf("Enabled: %s\n", wifiConfig.enabled ? "Yes" : "No");
    
    // Access Point information
    if (wifiConfig.mode == DCC_WIFI_AP || wifiConfig.mode == DCC_WIFI_AP_STATION) {
        Serial.println("\n--- Access Point ---");
        Serial.printf("AP SSID: %s\n", wifiConfig.apSSID);
        Serial.printf("AP IP: %s\n", WiFi.softAPIP().toString().c_str());
        Serial.printf("AP Clients: %d\n", WiFi.softAPgetStationNum());
        
        // Get AP channel (ESP32 specific)
        wifi_config_t conf;
        esp_wifi_get_config(WIFI_IF_AP, &conf);
        Serial.printf("AP Channel: %d\n", conf.ap.channel);
    }
    
    // Station information
    if (wifiConfig.mode == DCC_WIFI_STATION || wifiConfig.mode == DCC_WIFI_AP_STATION) {
        Serial.println("\n--- Station ---");
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("Connected to: %s\n", WiFi.SSID().c_str());
            Serial.printf("Station IP: %s\n", WiFi.localIP().toString().c_str());
            Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
            Serial.printf("Subnet Mask: %s\n", WiFi.subnetMask().toString().c_str());
            Serial.printf("DNS 1: %s\n", WiFi.dnsIP(0).toString().c_str());
            Serial.printf("DNS 2: %s\n", WiFi.dnsIP(1).toString().c_str());
            Serial.printf("Signal Strength: %d dBm\n", WiFi.RSSI());
            Serial.printf("Channel: %d\n", WiFi.channel());
            
            // Get detailed connection info
            wifi_ap_record_t ap_info;
            if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
                Serial.printf("BSSID: %02x:%02x:%02x:%02x:%02x:%02x\n",
                             ap_info.bssid[0], ap_info.bssid[1], ap_info.bssid[2],
                             ap_info.bssid[3], ap_info.bssid[4], ap_info.bssid[5]);
                
                String authMode;
                switch (ap_info.authmode) {
                    case WIFI_AUTH_OPEN: authMode = "Open"; break;
                    case WIFI_AUTH_WEP: authMode = "WEP"; break;
                    case WIFI_AUTH_WPA_PSK: authMode = "WPA"; break;
                    case WIFI_AUTH_WPA2_PSK: authMode = "WPA2"; break;
                    case WIFI_AUTH_WPA_WPA2_PSK: authMode = "WPA/WPA2"; break;
                    case WIFI_AUTH_WPA2_ENTERPRISE: authMode = "WPA2 Enterprise"; break;
                    case WIFI_AUTH_WPA3_PSK: authMode = "WPA3"; break;
                    case WIFI_AUTH_WPA2_WPA3_PSK: authMode = "WPA2/WPA3"; break;
                    default: authMode = "Unknown"; break;
                }
                Serial.printf("Security: %s\n", authMode.c_str());
            }
        } else {
            Serial.println("Status: Not connected");
            if (strlen(wifiConfig.stationSSID) > 0) {
                Serial.printf("Configured SSID: %s\n", wifiConfig.stationSSID);
            } else {
                Serial.println("No station SSID configured");
            }
        }
    }
    
    // Device information
    Serial.println("\n--- Device Info ---");
    Serial.printf("MAC Address: %s\n", getMacAddress().c_str());
    
    // mDNS information with troubleshooting
    String mdnsHostname = getMDNSHostname();
    Serial.printf("mDNS Hostname: %s.local\n", mdnsHostname.c_str());
    
    // Test mDNS resolution
    IPAddress resolvedIP = MDNS.queryHost(mdnsHostname);
    if (resolvedIP != IPAddress(0, 0, 0, 0)) {
        Serial.printf("mDNS Status: ✓ Active (resolves to %s)\n", resolvedIP.toString().c_str());
    } else {
        Serial.println("mDNS Status: ⚠ Not resolving");
    }
    
    // Show alternative access methods
    Serial.println("\n--- Access Methods ---");
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("Direct IP: http://%s\n", WiFi.localIP().toString().c_str());
    }
    if (wifiConfig.mode == DCC_WIFI_AP || wifiConfig.mode == DCC_WIFI_AP_STATION) {
        Serial.printf("AP Direct: http://%s\n", WiFi.softAPIP().toString().c_str());
    }
    Serial.printf("mDNS Link: http://%s.local\n", mdnsHostname.c_str());
    
    Serial.printf("\nFree Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("WiFi Mode: %d\n", WiFi.getMode());
    
    // mDNS troubleshooting tips
    if (resolvedIP == IPAddress(0, 0, 0, 0)) {
        Serial.println("\n--- mDNS Troubleshooting ---");
        Serial.println("If .local address doesn't work:");
        Serial.println("• Use direct IP address instead");
        Serial.println("• Check if your router supports mDNS/Bonjour");
        Serial.println("• Try from a different device/browser");
        Serial.println("• Windows: Install Bonjour Print Services");
        Serial.println("• Router: Enable mDNS/Multicast forwarding");
    }
    
    Serial.println("==================");
}

void processMDNSTestCommand() {
    Serial.println("=== mDNS Test & Restart ===");
    
    String hostname = getMDNSHostname();
    Serial.printf("Testing mDNS hostname: %s.local\n", hostname.c_str());
    
    // Test current mDNS status
    IPAddress resolvedIP = MDNS.queryHost(hostname);
    if (resolvedIP != IPAddress(0, 0, 0, 0)) {
        Serial.printf("✓ Current mDNS is working: resolves to %s\n", resolvedIP.toString().c_str());
    } else {
        Serial.println("✗ Current mDNS is not resolving");
    }
    
    // Show network status
    Serial.println("\n--- Network Status ---");
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("WiFi Status: Connected to %s\n", WiFi.SSID().c_str());
        Serial.printf("Station IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("WiFi Status: Not connected to station");
    }
    
    if (wifiConfig.mode == DCC_WIFI_AP || wifiConfig.mode == DCC_WIFI_AP_STATION) {
        Serial.printf("AP Status: Active (%s)\n", WiFi.softAPIP().toString().c_str());
        Serial.printf("AP Clients: %d\n", WiFi.softAPgetStationNum());
    }
    
    // Restart mDNS
    Serial.println("\n--- Restarting mDNS ---");
    MDNS.end();
    delay(500);
    
    setupMDNS();
    
    // Wait and test again
    delay(2000);
    resolvedIP = MDNS.queryHost(hostname);
    
    Serial.println("\n--- Test Results ---");
    if (resolvedIP != IPAddress(0, 0, 0, 0)) {
        Serial.printf("✓ mDNS restart successful: %s.local -> %s\n", 
                     hostname.c_str(), resolvedIP.toString().c_str());
        Serial.println("✓ Device should be accessible via mDNS");
    } else {
        Serial.printf("⚠ mDNS still not resolving after restart\n");
        Serial.println("\nTroubleshooting suggestions:");
        Serial.println("• Use direct IP addresses instead of .local");
        Serial.println("• Check router mDNS/Bonjour support");
        Serial.println("• Try 'ping dccservo-XXXXXX.local' from computer");
        Serial.println("• Windows users: install Bonjour Print Services");
        Serial.println("• Some corporate networks block mDNS traffic");
    }
    
    Serial.println("\n--- Alternative Access ---");
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("Direct Station IP: http://%s\n", WiFi.localIP().toString().c_str());
    }
    if (wifiConfig.mode == DCC_WIFI_AP || wifiConfig.mode == DCC_WIFI_AP_STATION) {
        Serial.printf("Direct AP IP: http://%s\n", WiFi.softAPIP().toString().c_str());
    }
    
    Serial.println("==================");
}

void processDccDebugCommand() {
    extern void toggleDccDebug();
    extern bool dccDebugEnabled;
    
    toggleDccDebug();
    
    Serial.println("=== DCC Debug Mode ===");
    Serial.printf("Status: %s\n", dccDebugEnabled ? "ENABLED" : "DISABLED");
    
    if (dccDebugEnabled) {
        Serial.println("DCC packet monitoring is now active.");
        Serial.println("You will see debug output for:");
        Serial.println("• All received DCC accessory packets");
        Serial.println("• Packets matching configured servo addresses");
        Serial.println("• Servo actions triggered by DCC commands");
        Serial.println("• DCC signal LED will flash on GPIO 2 for valid packets");
    } else {
        Serial.println("DCC packet monitoring is now disabled.");
        Serial.println("Only matched servo actions will be processed.");
    }
    
    // Show configured DCC addresses
    Serial.println("\nConfigured DCC addresses:");
    bool hasAddresses = false;
    for (int i = 0; i < TOTAL_PINS; i++) {
        if (virtualservo[i].address != 0) {
            Serial.printf("  Servo %d (GPIO %d): Address %d\n", 
                         i, virtualservo[i].pin, virtualservo[i].address);
            hasAddresses = true;
        }
    }
    
    if (!hasAddresses) {
        Serial.println("  No DCC addresses configured!");
        Serial.println("  Use 's' command to configure servo addresses.");
    }
    
    Serial.println("\nType 'z' again to toggle debug mode.");
    Serial.println("==================");
}
