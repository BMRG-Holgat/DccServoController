#ifndef WIFI_CONTROLLER_H
#define WIFI_CONTROLLER_H

#include <WiFi.h>
#include <WiFiAP.h>
#include <WebServer.h>
#include "config.h"

// WiFi configuration constants
#define WIFI_SSID_MAX_LENGTH 32
#define WIFI_PASSWORD_MAX_LENGTH 64
#define WIFI_TIMEOUT_MS 10000
#define WIFI_AP_CHANNEL 1
#define WIFI_AP_MAX_CONNECTIONS 4

// WiFi modes (using custom names to avoid ESP32 conflicts)
enum DccWiFiMode {
    DCC_WIFI_OFF = 0,
    DCC_WIFI_AP = 1,
    DCC_WIFI_STATION = 2,
    DCC_WIFI_AP_STATION = 3
};

// WiFi configuration structure for EEPROM
struct WiFiConfig {
    bool enabled = true;
    DccWiFiMode mode = DCC_WIFI_AP;
    char stationSSID[WIFI_SSID_MAX_LENGTH] = "";
    char stationPassword[WIFI_PASSWORD_MAX_LENGTH] = "";
    char apSSID[WIFI_SSID_MAX_LENGTH] = "";
    char apPassword[WIFI_PASSWORD_MAX_LENGTH] = "";
    bool useStaticIP = false;
    IPAddress staticIP = IPAddress(192, 168, 1, 100);
    IPAddress gateway = IPAddress(192, 168, 1, 1);
    IPAddress subnet = IPAddress(255, 255, 255, 0);
    IPAddress dns1 = IPAddress(8, 8, 8, 8);
    IPAddress dns2 = IPAddress(8, 8, 4, 4);
};

// Global WiFi configuration
extern WiFiConfig wifiConfig;
extern WebServer webServer;

// Function declarations
void initializeWiFi();
void setupAccessPoint();
void setupStation();
void handleWiFiEvents();
void generateDefaultCredentials();
bool needsCredentialUpdate();
String getMacAddress();
String getLastSixMacChars();
String getMDNSHostname();
void setupMDNS();
void startWebServer();
void handleRoot();
void handleConfig();
void handleServoControl();
void handleServoConfig();
void updateServoConfig();
void handleFactoryReset();
void handleNotFound();
void handleWiFiScan();
void updateWiFiConfig();
void printWiFiStatus();
bool isWiFiConnected();
void processWiFiCommands();

#endif // WIFI_CONTROLLER_H
