#include "wifi_controller.h"
#include "servo_controller.h"
#include "eeprom_manager.h"
#include "serial_commands.h"
#include "version.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Global WiFi objects
WiFiConfig wifiConfig;
WebServer webServer(80);

// Utility functions
String getMacAddress() {
    return WiFi.macAddress();
}

String getSpeedString(int speed) {
    switch (speed) {
        case SPEED_INSTANT: return "Instant";
        case SPEED_FAST: return "Fast";
        case SPEED_NORMAL: return "Normal";
        case SPEED_SLOW: return "Slow";
        default: return "Unknown";
    }
}

String getLastSixMacChars() {
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    return mac.substring(mac.length() - 6);
}

void initializeWiFi() {
    Serial.println("Initializing WiFi...");
    
    // Generate default credentials if not set or if they contain uppercase MAC characters
    if (strlen(wifiConfig.apSSID) == 0 || needsCredentialUpdate()) {
        generateDefaultCredentials();
        // Save the updated credentials
        bootController.isDirty = true;
        putSettings();
    }
    
    // Set WiFi mode based on configuration
    switch (wifiConfig.mode) {
        case DCC_WIFI_AP:
            setupAccessPoint();
            break;
        case DCC_WIFI_STATION:
            setupStation();
            break;
        case DCC_WIFI_AP_STATION:
            setupAccessPoint();
            setupStation();
            break;
        default:
            Serial.println("WiFi disabled");
            WiFi.mode(WIFI_OFF);
            return;
    }
    
    // Start web server if WiFi is enabled
    if (wifiConfig.enabled && wifiConfig.mode != DCC_WIFI_OFF) {
        startWebServer();
    }
    
    printWiFiStatus();
}

void generateDefaultCredentials() {
    String macSuffix = getLastSixMacChars();
    
    // Convert to lowercase manually
    for (int i = 0; i < macSuffix.length(); i++) {
        macSuffix[i] = tolower(macSuffix[i]);
    }
    
    // Generate default AP SSID: DCCAC_ + last 6 MAC chars
    snprintf(wifiConfig.apSSID, WIFI_SSID_MAX_LENGTH, "DCCAC_%s", macSuffix.c_str());
    
    // Generate default AP password: PASS_ + last 6 MAC chars
    snprintf(wifiConfig.apPassword, WIFI_PASSWORD_MAX_LENGTH, "PASS_%s", macSuffix.c_str());
    
    Serial.println("Generated default WiFi credentials:");
    Serial.printf("AP SSID: %s\n", wifiConfig.apSSID);
    Serial.printf("AP Password: %s\n", wifiConfig.apPassword);
}

void setupAccessPoint() {
    Serial.println("Setting up Access Point...");
    
    WiFi.mode(WIFI_AP);
    
    bool success = WiFi.softAP(wifiConfig.apSSID, wifiConfig.apPassword, WIFI_AP_CHANNEL, false, WIFI_AP_MAX_CONNECTIONS);
    
    if (success) {
        IPAddress apIP = WiFi.softAPIP();
        Serial.printf("Access Point started successfully\n");
        Serial.printf("AP SSID: %s\n", wifiConfig.apSSID);
        Serial.printf("AP Password: %s\n", wifiConfig.apPassword);
        Serial.printf("AP IP Address: %s\n", apIP.toString().c_str());
    } else {
        Serial.println("Failed to start Access Point");
    }
}

void setupStation() {
    if (strlen(wifiConfig.stationSSID) == 0) {
        Serial.println("No station SSID configured");
        return;
    }
    
    Serial.printf("Connecting to WiFi network: %s\n", wifiConfig.stationSSID);
    
    WiFi.mode(WIFI_STA);
    
    // Configure static IP if enabled
    if (wifiConfig.useStaticIP) {
        WiFi.config(wifiConfig.staticIP, wifiConfig.gateway, wifiConfig.subnet, wifiConfig.dns1, wifiConfig.dns2);
    }
    
    WiFi.begin(wifiConfig.stationSSID, wifiConfig.stationPassword);
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_TIMEOUT_MS) {
        delay(500);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.printf("Connected to %s\n", wifiConfig.stationSSID);
        Serial.printf("Station IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println();
        Serial.println("Failed to connect to WiFi network");
        
        // Fall back to AP mode if station connection fails
        Serial.println("Falling back to Access Point mode...");
        setupAccessPoint();
    }
}

void startWebServer() {
    // Set up web server routes
    webServer.on("/", handleRoot);
    webServer.on("/config", HTTP_GET, handleConfig);
    webServer.on("/config", HTTP_POST, updateWiFiConfig);
    webServer.on("/scan", HTTP_GET, handleWiFiScan);
    webServer.on("/servo", HTTP_GET, handleServoControl);
    webServer.on("/servo", HTTP_POST, handleServoControl);
    webServer.on("/servo-config", HTTP_GET, handleServoConfig);
    webServer.on("/servo-config", HTTP_POST, updateServoConfig);
    webServer.on("/factory-reset", HTTP_POST, handleFactoryReset);
    webServer.onNotFound(handleNotFound);
    
    webServer.begin();
    Serial.println("Web server started on port 80");
}

void handleRoot() {
    String html = "<!DOCTYPE html><html><head><title>ESP32 DCC Servo Controller</title>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body{font-family:Arial,sans-serif;margin:0;padding:20px;background-color:#f5f5f5;}";
    html += ".container{max-width:800px;margin:0 auto;background:white;padding:20px;border-radius:8px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}";
    html += "h1{color:#333;text-align:center;margin-bottom:10px;}";
    html += "h2{color:#666;text-align:center;margin-top:0;margin-bottom:30px;}";
    html += "h3{color:#333;border-bottom:2px solid #4CAF50;padding-bottom:5px;}";
    html += ".button{background:#4CAF50;color:white;padding:12px 24px;border:none;border-radius:6px;cursor:pointer;margin:8px;font-size:16px;text-decoration:none;display:inline-block;transition:background 0.3s;}";
    html += ".button:hover{background:#45a049;}";
    html += ".danger{background:#f44336;}";
    html += ".danger:hover{background:#da190b;}";
    html += ".nav-buttons{text-align:center;margin:20px 0;display:flex;flex-wrap:wrap;justify-content:center;gap:10px;}";
    html += ".info-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(250px,1fr));gap:20px;margin:20px 0;}";
    html += ".info-card{background:#f9f9f9;padding:15px;border-radius:6px;border-left:4px solid #4CAF50;}";
    html += ".info-item{margin:8px 0;padding:5px 0;}";
    html += ".info-label{font-weight:bold;color:#333;}";
    html += ".info-value{color:#666;margin-left:10px;}";
    html += "@media (max-width:600px){";
    html += ".container{margin:10px;padding:15px;}";
    html += ".button{width:100%;margin:5px 0;padding:15px;font-size:18px;}";
    html += ".nav-buttons{flex-direction:column;align-items:center;}";
    html += ".info-grid{grid-template-columns:1fr;}";
    html += "h1{font-size:24px;}";
    html += "h2{font-size:18px;}";
    html += "}";
    html += "</style></head><body>";
    
    html += "<div class='container'>";
    html += "<h1>ESP32 DCC Servo Controller</h1>";
    html += "<h2>Version: " + String(SOFTWARE_VERSION) + "</h2>";
    
    html += "<div class='info-grid'>";
    
    // WiFi Status Card
    html += "<div class='info-card'>";
    html += "<h3>WiFi Status</h3>";
    html += "<div class='info-item'><span class='info-label'>Mode:</span><span class='info-value'>";
    switch (wifiConfig.mode) {
        case DCC_WIFI_AP: html += "Access Point"; break;
        case DCC_WIFI_STATION: html += "Station"; break;
        case DCC_WIFI_AP_STATION: html += "AP + Station"; break;
        default: html += "Disabled"; break;
    }
    html += "</span></div>";
    
    if (wifiConfig.mode == DCC_WIFI_AP || wifiConfig.mode == DCC_WIFI_AP_STATION) {
        html += "<div class='info-item'><span class='info-label'>AP SSID:</span><span class='info-value'>" + String(wifiConfig.apSSID) + "</span></div>";
        html += "<div class='info-item'><span class='info-label'>AP IP:</span><span class='info-value'>" + WiFi.softAPIP().toString() + "</span></div>";
    }
    
    if (wifiConfig.mode == DCC_WIFI_STATION || wifiConfig.mode == DCC_WIFI_AP_STATION) {
        if (WiFi.status() == WL_CONNECTED) {
            html += "<div class='info-item'><span class='info-label'>Connected to:</span><span class='info-value'>" + WiFi.SSID() + "</span></div>";
            html += "<div class='info-item'><span class='info-label'>Station IP:</span><span class='info-value'>" + WiFi.localIP().toString() + "</span></div>";
        } else {
            html += "<div class='info-item'><span class='info-label'>Station:</span><span class='info-value'>Not connected</span></div>";
        }
    }
    html += "</div>";
    
    // Device Information Card
    html += "<div class='info-card'>";
    html += "<h3>Device Information</h3>";
    html += "<div class='info-item'><span class='info-label'>MAC Address:</span><span class='info-value'>" + getMacAddress() + "</span></div>";
    html += "<div class='info-item'><span class='info-label'>Free Heap:</span><span class='info-value'>" + String(ESP.getFreeHeap()) + " bytes</span></div>";
    html += "<div class='info-item'><span class='info-label'>Uptime:</span><span class='info-value'>" + String(millis() / 1000) + " seconds</span></div>";
    html += "</div>";
    
    html += "</div>";
    
    // Navigation buttons
    html += "<div class='nav-buttons'>";
    html += "<a href='/config' class='button'>WiFi Configuration</a>";
    html += "<a href='/servo' class='button'>Servo Control</a>";
    html += "<a href='/servo-config' class='button'>Servo Configuration</a>";
    html += "</div>";
    
    html += "</div></body></html>";
    
    webServer.send(200, "text/html", html);
}

void handleConfig() {
    String html = "<!DOCTYPE html><html><head><title>WiFi Configuration</title>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body{font-family:Arial,sans-serif;margin:0;padding:20px;background-color:#f5f5f5;}";
    html += ".container{max-width:600px;margin:0 auto;background:white;padding:20px;border-radius:8px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}";
    html += "h1{color:#333;text-align:center;margin-bottom:30px;}";
    html += "h3{color:#333;border-bottom:2px solid #4CAF50;padding-bottom:5px;margin-top:30px;}";
    html += ".form-group{margin:20px 0;}";
    html += "label{display:block;margin-bottom:8px;font-weight:bold;color:#333;}";
    html += "input,select{width:100%;padding:12px;border:2px solid #ddd;border-radius:6px;box-sizing:border-box;font-size:16px;transition:border-color 0.3s;}";
    html += "input:focus,select:focus{border-color:#4CAF50;outline:none;}";
    html += ".input-group{display:flex;gap:10px;align-items:flex-end;}";
    html += ".input-group input{flex:1;}";
    html += ".button{background:#4CAF50;color:white;padding:12px 24px;border:none;border-radius:6px;cursor:pointer;font-size:16px;transition:background 0.3s;white-space:nowrap;}";
    html += ".button:hover{background:#45a049;}";
    html += ".button:disabled{background:#ccc;cursor:not-allowed;}";
    html += ".danger{background:#f44336;}";
    html += ".danger:hover{background:#da190b;}";
    html += ".network-list{margin-top:10px;display:none;padding:10px;background:#f9f9f9;border-radius:6px;}";
    html += ".button-group{display:flex;gap:10px;justify-content:center;margin:20px 0;}";
    html += "hr{margin:30px 0;border:none;border-top:1px solid #ddd;}";
    html += "@media (max-width:600px){";
    html += ".container{margin:10px;padding:15px;}";
    html += ".input-group{flex-direction:column;align-items:stretch;}";
    html += ".input-group .button{margin-top:10px;}";
    html += ".button-group{flex-direction:column;}";
    html += "h1{font-size:24px;}";
    html += "}";
    html += "</style></head><body>";
    
    html += "<div class='container'>";
    html += "<h1>WiFi Configuration</h1>";
    html += "<form method='POST'>";
    
    // WiFi Mode
    html += "<div class='form-group'>";
    html += "<label for='mode'>WiFi Mode:</label>";
    html += "<select id='mode' name='mode'>";
    html += "<option value='0'" + String(wifiConfig.mode == DCC_WIFI_OFF ? " selected" : "") + ">Disabled</option>";
    html += "<option value='1'" + String(wifiConfig.mode == DCC_WIFI_AP ? " selected" : "") + ">Access Point Only</option>";
    html += "<option value='2'" + String(wifiConfig.mode == DCC_WIFI_STATION ? " selected" : "") + ">Station Only</option>";
    html += "<option value='3'" + String(wifiConfig.mode == DCC_WIFI_AP_STATION ? " selected" : "") + ">AP + Station</option>";
    html += "</select>";
    html += "</div>";
    
    // Station Settings
    html += "<h3>Station Settings</h3>";
    html += "<div class='form-group'>";
    html += "<label for='stationSSID'>Station SSID:</label>";
    html += "<div class='input-group'>";
    html += "<input type='text' id='stationSSID' name='stationSSID' value='" + String(wifiConfig.stationSSID) + "' maxlength='31' placeholder='Enter network name'>";
    html += "<button type='button' class='button' onclick='scanWiFiNetworks()' id='scanBtn'>Scan Networks</button>";
    html += "</div>";
    html += "<div id='networkList' class='network-list'>";
    html += "<label>Available Networks:</label>";
    html += "<select id='networkSelect' onchange='selectNetwork()' style='width:100%;margin-top:5px;'>";
    html += "<option value=''>Select a network...</option>";
    html += "</select>";
    html += "</div>";
    html += "</div>";
    
    html += "<div class='form-group'>";
    html += "<label for='stationPassword'>Station Password:</label>";
    html += "<input type='password' id='stationPassword' name='stationPassword' value='" + String(wifiConfig.stationPassword) + "' maxlength='63' placeholder='Enter password'>";
    html += "</div>";
    
    // Access Point Settings
    html += "<h3>Access Point Settings</h3>";
    html += "<div class='form-group'>";
    html += "<label for='apSSID'>AP SSID:</label>";
    html += "<input type='text' id='apSSID' name='apSSID' value='" + String(wifiConfig.apSSID) + "' maxlength='31' placeholder='Access point name'>";
    html += "</div>";
    
    html += "<div class='form-group'>";
    html += "<label for='apPassword'>AP Password:</label>";
    html += "<input type='password' id='apPassword' name='apPassword' value='" + String(wifiConfig.apPassword) + "' maxlength='63' placeholder='Access point password'>";
    html += "</div>";
    
    // Buttons
    html += "<div class='button-group'>";
    html += "<button type='submit' class='button'>Save Configuration</button>";
    html += "<button type='button' class='button' onclick=\"location.href='/'\">Cancel</button>";
    html += "</div>";
    
    html += "</form>";
    
    // Factory Reset Section
    html += "<hr>";
    html += "<h3>Factory Reset</h3>";
    html += "<p>This will reset all WiFi settings to defaults and reset all servos to factory values.</p>";
    html += "<form method='POST' action='/factory-reset' onsubmit='return confirm(\"Are you sure you want to perform a factory reset?\");'>";
    html += "<div style='text-align:center;'>";
    html += "<button type='submit' class='button danger'>Factory Reset</button>";
    html += "</div>";
    html += "</form>";
    
    // JavaScript for WiFi scanning
    html += "<script>";
    html += "function scanWiFiNetworks() {";
    html += "  const button = document.getElementById('scanBtn');";
    html += "  const networkSelect = document.getElementById('networkSelect');";
    html += "  const networkList = document.getElementById('networkList');";
    html += "  ";
    html += "  button.disabled = true;";
    html += "  button.textContent = 'Scanning...';";
    html += "  networkList.style.display = 'none';";
    html += "  ";
    html += "  fetch('/scan')";
    html += "    .then(response => {";
    html += "      if (!response.ok) {";
    html += "        throw new Error('Network response was not ok');";
    html += "      }";
    html += "      return response.json();";
    html += "    })";
    html += "    .then(data => {";
    html += "      console.log('Scan response:', data);";
    html += "      ";
    html += "      // Clear existing options";
    html += "      networkSelect.innerHTML = '<option value=\"\">Select a network...</option>';";
    html += "      ";
    html += "      if (data.networks && data.networks.length > 0) {";
    html += "        data.networks.forEach(network => {";
    html += "          const option = document.createElement('option');";
    html += "          option.value = network.ssid;";
    html += "          option.textContent = network.ssid + ' (' + network.rssi + ' dBm, ' + network.encryption + ')';";
    html += "          networkSelect.appendChild(option);";
    html += "        });";
    html += "        networkList.style.display = 'block';";
    html += "        console.log('Found ' + data.networks.length + ' networks');";
    html += "      } else {";
    html += "        console.log('No networks found in response');";
    html += "        alert('No networks found. Please try again.');";
    html += "      }";
    html += "    })";
    html += "    .catch(error => {";
    html += "      console.error('Error scanning networks:', error);";
    html += "      alert('Error scanning networks: ' + error.message);";
    html += "    })";
    html += "    .finally(() => {";
    html += "      button.disabled = false;";
    html += "      button.textContent = 'Scan Networks';";
    html += "    });";
    html += "}";
    html += "";
    html += "function selectNetwork() {";
    html += "  const networkSelect = document.getElementById('networkSelect');";
    html += "  const stationSSID = document.getElementById('stationSSID');";
    html += "  ";
    html += "  if (networkSelect.value) {";
    html += "    stationSSID.value = networkSelect.value;";
    html += "  }";
    html += "}";
    html += "</script>";
    
    html += "</div></body></html>";
    
    webServer.send(200, "text/html", html);
}

void updateWiFiConfig() {
    bool configChanged = false;
    
    // Update WiFi mode
    if (webServer.hasArg("mode")) {
        DccWiFiMode newMode = (DccWiFiMode)webServer.arg("mode").toInt();
        if (newMode != wifiConfig.mode) {
            wifiConfig.mode = newMode;
            configChanged = true;
        }
    }
    
    // Update station settings
    if (webServer.hasArg("stationSSID")) {
        String newSSID = webServer.arg("stationSSID");
        if (newSSID != String(wifiConfig.stationSSID)) {
            strncpy(wifiConfig.stationSSID, newSSID.c_str(), WIFI_SSID_MAX_LENGTH - 1);
            wifiConfig.stationSSID[WIFI_SSID_MAX_LENGTH - 1] = '\0';
            configChanged = true;
        }
    }
    
    if (webServer.hasArg("stationPassword")) {
        String newPassword = webServer.arg("stationPassword");
        if (newPassword != String(wifiConfig.stationPassword)) {
            strncpy(wifiConfig.stationPassword, newPassword.c_str(), WIFI_PASSWORD_MAX_LENGTH - 1);
            wifiConfig.stationPassword[WIFI_PASSWORD_MAX_LENGTH - 1] = '\0';
            configChanged = true;
        }
    }
    
    // Update AP settings
    if (webServer.hasArg("apSSID")) {
        String newSSID = webServer.arg("apSSID");
        if (newSSID != String(wifiConfig.apSSID)) {
            strncpy(wifiConfig.apSSID, newSSID.c_str(), WIFI_SSID_MAX_LENGTH - 1);
            wifiConfig.apSSID[WIFI_SSID_MAX_LENGTH - 1] = '\0';
            configChanged = true;
        }
    }
    
    if (webServer.hasArg("apPassword")) {
        String newPassword = webServer.arg("apPassword");
        if (newPassword != String(wifiConfig.apPassword)) {
            strncpy(wifiConfig.apPassword, newPassword.c_str(), WIFI_PASSWORD_MAX_LENGTH - 1);
            wifiConfig.apPassword[WIFI_PASSWORD_MAX_LENGTH - 1] = '\0';
            configChanged = true;
        }
    }
    
    if (configChanged) {
        // Mark EEPROM as dirty to save changes
        bootController.isDirty = true;
        putSettings();
        
        Serial.println("WiFi configuration updated");
        
        // Restart WiFi with new settings
        WiFi.disconnect();
        delay(1000);
        initializeWiFi();
    }
    
    // Redirect back to config page
    webServer.sendHeader("Location", "/config");
    webServer.send(302, "text/plain", "");
}

void handleServoControl() {
    if (webServer.method() == HTTP_POST) {
        // Handle servo control commands
        if (webServer.hasArg("servo") && webServer.hasArg("command")) {
            int servoNum = webServer.arg("servo").toInt();
            String command = webServer.arg("command");
            
            // Process servo command similar to serial interface
            if (servoNum >= 0 && servoNum < TOTAL_PINS) {
                if (command == "close" || command == "c") {
                    virtualservo[servoNum].state = SERVO_TO_CLOSED;
                } else if (command == "throw" || command == "t") {
                    virtualservo[servoNum].state = SERVO_TO_THROWN;
                } else if (command == "toggle" || command == "T") {
                    // Toggle servo position
                    if (virtualservo[servoNum].state == SERVO_CLOSED) {
                        virtualservo[servoNum].state = SERVO_TO_THROWN;
                    } else {
                        virtualservo[servoNum].state = SERVO_TO_CLOSED;
                    }
                } else if (command == "neutral" || command == "n") {
                    virtualservo[servoNum].state = SERVO_NEUTRAL;
                }
                
                webServer.send(200, "application/json", "{\"status\":\"success\"}");
                return;
            }
        }
        
        webServer.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid parameters\"}");
        return;
    }
    
    // GET request - show servo control page
    String html = "<!DOCTYPE html><html><head><title>Servo Control</title>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body{font-family:Arial,sans-serif;margin:0;padding:20px;background-color:#f5f5f5;}";
    html += ".container{max-width:1200px;margin:0 auto;background:white;padding:20px;border-radius:8px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}";
    html += "h1{color:#333;text-align:center;margin-bottom:30px;}";
    html += ".nav-buttons{display:flex;gap:10px;justify-content:center;margin:20px 0;flex-wrap:wrap;}";
    html += ".table-container{overflow-x:auto;margin:20px 0;}";
    html += "table{width:100%;border-collapse:collapse;min-width:700px;}";
    html += "th,td{padding:8px;text-align:center;border:1px solid #ddd;}";
    html += "th{background-color:#4CAF50;color:white;font-weight:bold;}";
    html += "tr:nth-child(even){background-color:#f9f9f9;}";
    html += "tr:hover{background-color:#f5f5f5;}";
    html += ".button{background:#4CAF50;color:white;padding:4px 8px;border:none;border-radius:4px;cursor:pointer;margin:1px;font-size:11px;transition:background 0.3s;}";
    html += ".button:hover{background:#45a049;}";
    html += ".button-close{background:#2196F3;}";
    html += ".button-close:hover{background:#0b7dda;}";
    html += ".button-throw{background:#ff9800;}";
    html += ".button-throw:hover{background:#e68900;}";
    html += ".button-neutral{background:#9e9e9e;}";
    html += ".button-neutral:hover{background:#757575;}";
    html += ".nav-button{background:#4CAF50;color:white;padding:12px 24px;border:none;border-radius:6px;cursor:pointer;font-size:16px;transition:background 0.3s;}";
    html += ".nav-button:hover{background:#45a049;}";
    html += ".action-buttons{display:flex;gap:2px;justify-content:center;flex-wrap:wrap;}";
    html += "@media (max-width:768px){";
    html += ".container{margin:10px;padding:15px;}";
    html += "h1{font-size:24px;}";
    html += ".table-container{margin:15px -15px;}";
    html += "table{font-size:12px;min-width:600px;}";
    html += "th,td{padding:6px 3px;}";
    html += ".button{padding:3px 6px;font-size:10px;margin:1px;}";
    html += ".nav-buttons{flex-direction:column;align-items:center;}";
    html += ".action-buttons{flex-direction:column;gap:1px;}";
    html += "}";
    html += "@media (max-width:480px){";
    html += "table{font-size:10px;min-width:500px;}";
    html += "th,td{padding:4px 2px;}";
    html += ".button{padding:2px 4px;font-size:9px;}";
    html += "}";
    html += "</style></head><body>";
    
    html += "<div class='container'>";
    html += "<h1>Servo Control</h1>";
    
    html += "<div class='nav-buttons'>";
    html += "<button class='nav-button' onclick=\"location.href='/'\">Home</button>";
    html += "<button class='nav-button' onclick=\"location.href='/servo-config'\">Servo Configuration</button>";
    html += "</div>";
    
    html += "<div class='table-container'>";
    html += "<table>";
    html += "<thead>";
    html += "<tr>";
    html += "<th>Servo</th>";
    html += "<th>DCC Address</th>";
    html += "<th>Swing (deg)</th>";
    html += "<th>Offset (deg)</th>";
    html += "<th>Speed</th>";
    html += "<th>Actions</th>";
    html += "</tr>";
    html += "</thead>";
    html += "<tbody>";
    
    // Generate servo control rows
    for (int i = 0; i < TOTAL_PINS; i++) {
        html += "<tr>";
        html += "<td><strong>" + String(i) + "</strong></td>";
        html += "<td>" + String(virtualservo[i].address) + "</td>";
        html += "<td>" + String(virtualservo[i].swing) + "</td>";
        html += "<td>" + String(virtualservo[i].offset) + "</td>";
        html += "<td>" + getSpeedString(virtualservo[i].speed) + "</td>";
        html += "<td>";
        html += "<div class='action-buttons'>";
        html += "<button class='button button-close' onclick='controlServo(" + String(i) + ", \"close\")'>Close</button>";
        html += "<button class='button button-throw' onclick='controlServo(" + String(i) + ", \"throw\")'>Throw</button>";
        html += "<button class='button button-neutral' onclick='controlServo(" + String(i) + ", \"neutral\")'>Neutral</button>";
        html += "</div>";
        html += "</td>";
        html += "</tr>";
    }
    
    html += "</tbody>";
    html += "</table>";
    html += "</div>";
    
    // JavaScript for servo control
    html += "<script>";
    html += "function controlServo(servo, command) {";
    html += "  fetch('/servo', {";
    html += "    method: 'POST',";
    html += "    headers: {'Content-Type': 'application/x-www-form-urlencoded'},";
    html += "    body: 'servo=' + servo + '&command=' + command";
    html += "  }).then(response => response.json())";
    html += "    .then(data => {";
    html += "      if (data.status === 'success') {";
    html += "        console.log('Servo ' + servo + ' ' + command + ' command sent');";
    html += "      } else {";
    html += "        alert('Error: ' + data.message);";
    html += "      }";
    html += "    }).catch(error => console.error('Error:', error));";
    html += "}";
    html += "</script>";
    
    html += "</div></body></html>";
    
    webServer.send(200, "text/html", html);
}

void handleServoConfig() {
    String html = "<!DOCTYPE html><html><head><title>Servo Configuration</title>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body{font-family:Arial,sans-serif;margin:0;padding:20px;background-color:#f5f5f5;}";
    html += ".container{max-width:1200px;margin:0 auto;background:white;padding:20px;border-radius:8px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}";
    html += "h1{color:#333;text-align:center;margin-bottom:30px;}";
    html += ".nav-buttons{display:flex;gap:10px;justify-content:center;margin:20px 0;flex-wrap:wrap;}";
    html += ".servo-config{border:2px solid #ddd;padding:15px;margin:15px 0;border-radius:8px;background:#f9f9f9;box-shadow:0 2px 5px rgba(0,0,0,0.1);}";
    html += ".form-row{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:15px;margin:15px 0;}";
    html += ".form-group{display:flex;flex-direction:column;}";
    html += "label{display:block;margin-bottom:8px;font-weight:bold;font-size:14px;color:#333;}";
    html += "input,select{padding:8px;border:2px solid #ddd;border-radius:6px;box-sizing:border-box;font-size:14px;transition:border-color 0.3s;}";
    html += "input:focus,select:focus{border-color:#4CAF50;outline:none;}";
    html += ".button{background:#4CAF50;color:white;padding:10px 16px;border:none;border-radius:6px;cursor:pointer;font-size:14px;transition:background 0.3s;margin:2px;}";
    html += ".button:hover{background:#45a049;}";
    html += ".test-button{background:#2196F3;padding:6px 12px;font-size:12px;}";
    html += ".test-button:hover{background:#0b7dda;}";
    html += ".save-button{background:#ff9800;padding:12px 24px;font-size:16px;}";
    html += ".save-button:hover{background:#e68900;}";
    html += ".nav-button{background:#4CAF50;color:white;padding:12px 24px;border:none;border-radius:6px;cursor:pointer;font-size:16px;transition:background 0.3s;}";
    html += ".nav-button:hover{background:#45a049;}";
    html += "h3{margin:0 0 15px 0;color:#333;font-size:18px;border-bottom:2px solid #4CAF50;padding-bottom:5px;}";
    html += ".test-controls{display:flex;gap:8px;justify-content:center;margin-top:10px;flex-wrap:wrap;}";
    html += ".servo-save-controls{display:flex;justify-content:center;margin:10px 0;padding-top:10px;border-top:1px solid #ddd;}";
    html += ".save-controls{display:flex;gap:15px;justify-content:center;margin:30px 0;flex-wrap:wrap;}";
    html += "@media (max-width:768px){";
    html += ".container{margin:10px;padding:15px;}";
    html += "h1{font-size:24px;}";
    html += ".form-row{grid-template-columns:1fr;gap:10px;}";
    html += ".servo-config{padding:12px;margin:10px 0;}";
    html += "h3{font-size:16px;}";
    html += ".nav-buttons{flex-direction:column;align-items:center;}";
    html += ".test-controls{flex-direction:column;}";
    html += ".servo-save-controls{margin:15px 0;}";
    html += ".save-controls{flex-direction:column;align-items:center;}";
    html += "}";
    html += "@media (max-width:480px){";
    html += "input,select{font-size:16px;padding:10px;}";
    html += ".button{padding:8px 12px;font-size:14px;}";
    html += ".test-button{padding:8px 10px;font-size:12px;}";
    html += "}";
    html += "</style></head><body>";
    
    html += "<div class='container'>";
    html += "<h1>Servo Configuration</h1>";
    
    html += "<div class='nav-buttons'>";
    html += "<button class='nav-button' onclick=\"location.href='/'\">Home</button>";
    html += "<button class='nav-button' onclick=\"location.href='/servo'\">Servo Control</button>";
    html += "</div>";
    
    html += "<form id='servoConfigForm'>";
    
    // Generate servo configuration sections
    for (int i = 0; i < TOTAL_PINS; i++) {
        html += "<div class='servo-config'>";
        html += "<h3>Servo " + String(i) + "</h3>";
        
        html += "<div class='form-row'>";
        
        html += "<div class='form-group'>";
        html += "<label for='addr" + String(i) + "'>DCC Address</label>";
        html += "<input type='number' id='addr" + String(i) + "' name='addr" + String(i) + "' value='" + String(virtualservo[i].address) + "' min='0' max='2048'>";
        html += "</div>";
        
        html += "<div class='form-group'>";
        html += "<label for='swing" + String(i) + "'>Swing (degrees)</label>";
        html += "<input type='number' id='swing" + String(i) + "' name='swing" + String(i) + "' value='" + String(virtualservo[i].swing) + "' min='1' max='90'>";
        html += "</div>";
        
        html += "<div class='form-group'>";
        html += "<label for='offset" + String(i) + "'>Offset (degrees)</label>";
        html += "<input type='number' id='offset" + String(i) + "' name='offset" + String(i) + "' value='" + String(virtualservo[i].offset) + "' min='-45' max='45'>";
        html += "</div>";
        
        html += "<div class='form-group'>";
        html += "<label for='speed" + String(i) + "'>Speed</label>";
        html += "<select id='speed" + String(i) + "' name='speed" + String(i) + "'>";
        html += "<option value='0'" + String(virtualservo[i].speed == SPEED_INSTANT ? " selected" : "") + ">Instant</option>";
        html += "<option value='1'" + String(virtualservo[i].speed == SPEED_FAST ? " selected" : "") + ">Fast</option>";
        html += "<option value='2'" + String(virtualservo[i].speed == SPEED_NORMAL ? " selected" : "") + ">Normal</option>";
        html += "<option value='3'" + String(virtualservo[i].speed == SPEED_SLOW ? " selected" : "") + ">Slow</option>";
        html += "</select>";
        html += "</div>";
        
        html += "<div class='form-group'>";
        html += "<label for='invert" + String(i) + "'>Invert</label>";
        html += "<select id='invert" + String(i) + "' name='invert" + String(i) + "'>";
        html += "<option value='0'" + String(!virtualservo[i].invert ? " selected" : "") + ">No</option>";
        html += "<option value='1'" + String(virtualservo[i].invert ? " selected" : "") + ">Yes</option>";
        html += "</select>";
        html += "</div>";
        
        html += "</div>";
        
        html += "<div class='test-controls'>";
        html += "<label style='margin-bottom:8px;text-align:center;'>Test Servo:</label>";
        html += "<button type='button' class='button test-button' onclick='testServo(" + String(i) + ", \"close\")'>Close</button>";
        html += "<button type='button' class='button test-button' onclick='testServo(" + String(i) + ", \"throw\")'>Throw</button>";
        html += "<button type='button' class='button test-button' onclick='testServo(" + String(i) + ", \"neutral\")'>Neutral</button>";
        html += "</div>";
        
        html += "<div class='servo-save-controls'>";
        html += "<button type='button' class='button save-button' onclick='saveServoConfig(" + String(i) + ")'>Save Servo " + String(i) + "</button>";
        html += "</div>";
        
        html += "</div>";
    }
    
    html += "<div class='save-controls'>";
    html += "<button type='button' class='button save-button' onclick='saveAllConfigs()'>Save All Configurations</button>";
    html += "<button type='button' class='button' onclick='loadDefaults()'>Load Defaults</button>";
    html += "</div>";
    
    html += "</form>";
    
    // JavaScript for servo configuration
    html += "<script>";
    html += "function testServo(servo, command) {";
    html += "  fetch('/servo', {";
    html += "    method: 'POST',";
    html += "    headers: {'Content-Type': 'application/x-www-form-urlencoded'},";
    html += "    body: 'servo=' + servo + '&command=' + command";
    html += "  }).then(response => response.json())";
    html += "    .then(data => {";
    html += "      if (data.status === 'success') {";
    html += "        console.log('Servo ' + servo + ' ' + command + ' test sent');";
    html += "      } else {";
    html += "        alert('Error: ' + data.message);";
    html += "      }";
    html += "    }).catch(error => console.error('Error:', error));";
    html += "}";
    html += "";
    html += "function saveServoConfig(servoIndex) {";
    html += "  const addr = document.getElementById('addr' + servoIndex).value;";
    html += "  const swing = document.getElementById('swing' + servoIndex).value;";
    html += "  const offset = document.getElementById('offset' + servoIndex).value;";
    html += "  const speed = document.getElementById('speed' + servoIndex).value;";
    html += "  const invert = document.getElementById('invert' + servoIndex).value;";
    html += "  ";
    html += "  const params = new URLSearchParams();";
    html += "  params.append('servo', servoIndex);";
    html += "  params.append('addr' + servoIndex, addr);";
    html += "  params.append('swing' + servoIndex, swing);";
    html += "  params.append('offset' + servoIndex, offset);";
    html += "  params.append('speed' + servoIndex, speed);";
    html += "  params.append('invert' + servoIndex, invert);";
    html += "  ";
    html += "  fetch('/servo-config', {";
    html += "    method: 'POST',";
    html += "    headers: {'Content-Type': 'application/x-www-form-urlencoded'},";
    html += "    body: params.toString()";
    html += "  }).then(response => response.json())";
    html += "    .then(data => {";
    html += "      if (data.status === 'success') {";
    html += "        alert('Servo ' + servoIndex + ' configuration saved successfully!');";
    html += "      } else {";
    html += "        alert('Error saving servo ' + servoIndex + ' configuration: ' + data.message);";
    html += "      }";
    html += "    }).catch(error => {";
    html += "      console.error('Error:', error);";
    html += "      alert('Error saving servo ' + servoIndex + ' configuration');";
    html += "    });";
    html += "}";
    html += "";
    html += "function saveAllConfigs() {";
    html += "  const form = document.getElementById('servoConfigForm');";
    html += "  const formData = new FormData(form);";
    html += "  const params = new URLSearchParams(formData);";
    html += "  ";
    html += "  fetch('/servo-config', {";
    html += "    method: 'POST',";
    html += "    headers: {'Content-Type': 'application/x-www-form-urlencoded'},";
    html += "    body: params.toString()";
    html += "  }).then(response => response.json())";
    html += "    .then(data => {";
    html += "      if (data.status === 'success') {";
    html += "        alert('Configuration saved successfully!');";
    html += "      } else {";
    html += "        alert('Error saving configuration: ' + data.message);";
    html += "      }";
    html += "    }).catch(error => {";
    html += "      console.error('Error:', error);";
    html += "      alert('Error saving configuration');";
    html += "    });";
    html += "}";
    html += "";
    html += "function loadDefaults() {";
    html += "  if (confirm('Are you sure you want to load default values for all servos?')) {";
    html += "    location.reload();";
    html += "  }";
    html += "}";
    html += "</script>";
    
    html += "</div></body></html>";
    
    webServer.send(200, "text/html", html);
}

void updateServoConfig() {
    bool configChanged = false;
    
    // Check if this is a single servo update
    if (webServer.hasArg("servo")) {
        int servoIndex = webServer.arg("servo").toInt();
        
        if (servoIndex >= 0 && servoIndex < TOTAL_PINS) {
            String addrParam = "addr" + String(servoIndex);
            String swingParam = "swing" + String(servoIndex);
            String offsetParam = "offset" + String(servoIndex);
            String speedParam = "speed" + String(servoIndex);
            String invertParam = "invert" + String(servoIndex);
            
            if (webServer.hasArg(addrParam)) {
                int newAddr = webServer.arg(addrParam).toInt();
                if (newAddr != virtualservo[servoIndex].address) {
                    virtualservo[servoIndex].address = newAddr;
                    configChanged = true;
                }
            }
            
            if (webServer.hasArg(swingParam)) {
                int newSwing = webServer.arg(swingParam).toInt();
                if (newSwing != virtualservo[servoIndex].swing && newSwing >= 1 && newSwing <= 90) {
                    virtualservo[servoIndex].swing = newSwing;
                    configChanged = true;
                }
            }
            
            if (webServer.hasArg(offsetParam)) {
                int newOffset = webServer.arg(offsetParam).toInt();
                if (newOffset != virtualservo[servoIndex].offset && newOffset >= -45 && newOffset <= 45) {
                    virtualservo[servoIndex].offset = newOffset;
                    configChanged = true;
                }
            }
            
            if (webServer.hasArg(speedParam)) {
                int newSpeed = webServer.arg(speedParam).toInt();
                if (newSpeed != virtualservo[servoIndex].speed && newSpeed >= 0 && newSpeed <= 3) {
                    virtualservo[servoIndex].speed = newSpeed;
                    configChanged = true;
                }
            }
            
            if (webServer.hasArg(invertParam)) {
                bool newInvert = webServer.arg(invertParam).toInt() == 1;
                if (newInvert != virtualservo[servoIndex].invert) {
                    virtualservo[servoIndex].invert = newInvert;
                    configChanged = true;
                }
            }
            
            if (configChanged) {
                // Mark EEPROM as dirty to save changes
                bootController.isDirty = true;
                putSettings();
                
                Serial.printf("Servo %d configuration updated\n", servoIndex);
                webServer.send(200, "application/json", "{\"status\":\"success\"}");
            } else {
                webServer.send(200, "application/json", "{\"status\":\"no_changes\"}");
            }
            return;
        }
    }
    
    // Update all servo configurations (existing functionality for "Save All")
    for (int i = 0; i < TOTAL_PINS; i++) {
        String addrParam = "addr" + String(i);
        String swingParam = "swing" + String(i);
        String offsetParam = "offset" + String(i);
        String speedParam = "speed" + String(i);
        String invertParam = "invert" + String(i);
        
        if (webServer.hasArg(addrParam)) {
            int newAddr = webServer.arg(addrParam).toInt();
            if (newAddr != virtualservo[i].address) {
                virtualservo[i].address = newAddr;
                configChanged = true;
            }
        }
        
        if (webServer.hasArg(swingParam)) {
            int newSwing = webServer.arg(swingParam).toInt();
            if (newSwing != virtualservo[i].swing && newSwing >= 1 && newSwing <= 90) {
                virtualservo[i].swing = newSwing;
                configChanged = true;
            }
        }
        
        if (webServer.hasArg(offsetParam)) {
            int newOffset = webServer.arg(offsetParam).toInt();
            if (newOffset != virtualservo[i].offset && newOffset >= -45 && newOffset <= 45) {
                virtualservo[i].offset = newOffset;
                configChanged = true;
            }
        }
        
        if (webServer.hasArg(speedParam)) {
            int newSpeed = webServer.arg(speedParam).toInt();
            if (newSpeed != virtualservo[i].speed && newSpeed >= 0 && newSpeed <= 3) {
                virtualservo[i].speed = newSpeed;
                configChanged = true;
            }
        }
        
        if (webServer.hasArg(invertParam)) {
            bool newInvert = webServer.arg(invertParam).toInt() == 1;
            if (newInvert != virtualservo[i].invert) {
                virtualservo[i].invert = newInvert;
                configChanged = true;
            }
        }
    }
    
    if (configChanged) {
        // Mark EEPROM as dirty to save changes
        bootController.isDirty = true;
        putSettings();
        
        Serial.println("All servo configurations updated");
        webServer.send(200, "application/json", "{\"status\":\"success\"}");
    } else {
        webServer.send(200, "application/json", "{\"status\":\"no_changes\"}");
    }
}

void handleFactoryReset() {
    Serial.println("Performing factory reset...");
    
    // Reset WiFi configuration to defaults
    generateDefaultCredentials();
    wifiConfig.mode = DCC_WIFI_AP;
    wifiConfig.enabled = true;
    memset(wifiConfig.stationSSID, 0, WIFI_SSID_MAX_LENGTH);
    memset(wifiConfig.stationPassword, 0, WIFI_PASSWORD_MAX_LENGTH);
    
    // Reset all servos to factory defaults: servo,addr,swing,offset,speed,invert,continuous = 0,0,25,0,0,0
    for (int i = 0; i < TOTAL_PINS; i++) {
        virtualservo[i].pin = pwmPins[i];
        virtualservo[i].address = 0;
        virtualservo[i].swing = 25;
        virtualservo[i].offset = 0;
        virtualservo[i].speed = 0;  // Instant
        virtualservo[i].invert = false;
        virtualservo[i].continuous = false;
    }
    
    // Mark for EEPROM save
    bootController.isDirty = true;
    putSettings();
    
    Serial.println("Factory reset complete. Restarting WiFi...");
    
    // Restart WiFi
    WiFi.disconnect();
    delay(1000);
    initializeWiFi();
    
    String html = "<!DOCTYPE html><html><head><title>Factory Reset</title>";
    html += "<meta http-equiv='refresh' content='5;url=/'>";
    html += "</head><body>";
    html += "<h1>Factory Reset Complete</h1>";
    html += "<p>WiFi settings and servo configurations have been reset to factory defaults.</p>";
    html += "<p>You will be redirected to the home page in 5 seconds...</p>";
    html += "</body></html>";
    
    webServer.send(200, "text/html", html);
}

void handleNotFound() {
    webServer.send(404, "text/plain", "404: Not Found");
}

void handleWiFiScan() {
    Serial.println("WiFi scan requested");
    
    // Ensure WiFi is in a good state for scanning
    if (WiFi.getMode() == WIFI_OFF) {
        WiFi.mode(WIFI_STA);
        delay(100);
    }
    
    // Start WiFi scan
    int numNetworks = WiFi.scanNetworks(false, true); // async=false, show_hidden=true
    
    Serial.printf("WiFi scan found %d networks\n", numNetworks);
    
    String json = "{\"networks\":[";
    int validNetworks = 0;
    
    if (numNetworks > 0) {
        for (int i = 0; i < numNetworks; i++) {
            String ssid = WiFi.SSID(i);
            int rssi = WiFi.RSSI(i);
            wifi_auth_mode_t encryption = WiFi.encryptionType(i);
            int channel = WiFi.channel(i);
            
            // Skip networks with empty SSIDs
            if (ssid.length() == 0) {
                continue;
            }
            
            if (validNetworks > 0) json += ",";
            
            // Escape any quotes in SSID
            ssid.replace("\"", "\\\"");
            
            json += "{";
            json += "\"ssid\":\"" + ssid + "\",";
            json += "\"rssi\":" + String(rssi) + ",";
            json += "\"encryption\":\"" + String(encryption == WIFI_AUTH_OPEN ? "Open" : "Secured") + "\",";
            json += "\"channel\":" + String(channel);
            json += "}";
            
            validNetworks++;
            
            Serial.printf("Network %d: %s (%d dBm, Ch %d, %s)\n", 
                         validNetworks, ssid.c_str(), rssi, channel, 
                         encryption == WIFI_AUTH_OPEN ? "Open" : "Secured");
        }
    }
    
    json += "],\"count\":" + String(validNetworks) + "}";
    
    // Clean up scan results
    WiFi.scanDelete();
    
    // Set proper headers for JSON response
    webServer.sendHeader("Content-Type", "application/json");
    webServer.sendHeader("Cache-Control", "no-cache");
    webServer.send(200, "application/json", json);
    
    Serial.printf("WiFi scan response sent: %s\n", json.c_str());
}

void handleWiFiEvents() {
    // Handle web server requests
    webServer.handleClient();
    
    // Monitor WiFi connection status
    static unsigned long lastStatusCheck = 0;
    if (millis() - lastStatusCheck > 30000) {  // Check every 30 seconds
        lastStatusCheck = millis();
        
        if (wifiConfig.mode == DCC_WIFI_STATION || wifiConfig.mode == DCC_WIFI_AP_STATION) {
            if (WiFi.status() != WL_CONNECTED && strlen(wifiConfig.stationSSID) > 0) {
                Serial.println("WiFi connection lost, attempting to reconnect...");
                WiFi.begin(wifiConfig.stationSSID, wifiConfig.stationPassword);
            }
        }
    }
}

bool needsCredentialUpdate() {
    // Check if current credentials contain uppercase MAC characters
    String currentMac = getLastSixMacChars();
    String lowercaseMac = currentMac;
    for (int i = 0; i < lowercaseMac.length(); i++) {
        lowercaseMac[i] = tolower(lowercaseMac[i]);
    }
    
    // Check if AP SSID contains uppercase MAC
    String currentSSID = String(wifiConfig.apSSID);
    if (currentSSID.indexOf(currentMac) != -1 && currentMac != lowercaseMac) {
        Serial.println("Credentials contain uppercase MAC, updating to lowercase...");
        return true;
    }
    
    return false;
}

void printWiFiStatus() {
    Serial.println("=== WiFi Status ===");
    Serial.printf("Mode: %d\n", wifiConfig.mode);
    Serial.printf("Enabled: %s\n", wifiConfig.enabled ? "Yes" : "No");
    
    if (wifiConfig.mode == DCC_WIFI_AP || wifiConfig.mode == DCC_WIFI_AP_STATION) {
        Serial.printf("AP SSID: %s\n", wifiConfig.apSSID);
        Serial.printf("AP IP: %s\n", WiFi.softAPIP().toString().c_str());
        Serial.printf("AP Clients: %d\n", WiFi.softAPgetStationNum());
    }
    
    if (wifiConfig.mode == DCC_WIFI_STATION || wifiConfig.mode == DCC_WIFI_AP_STATION) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("Station SSID: %s\n", WiFi.SSID().c_str());
            Serial.printf("Station IP: %s\n", WiFi.localIP().toString().c_str());
            Serial.printf("Signal Strength: %d dBm\n", WiFi.RSSI());
        } else {
            Serial.println("Station: Not connected");
        }
    }
    
    Serial.printf("MAC Address: %s\n", getMacAddress().c_str());
    Serial.println("==================");
}

bool isWiFiConnected() {
    return (WiFi.status() == WL_CONNECTED) || (WiFi.softAPgetStationNum() > 0);
}
