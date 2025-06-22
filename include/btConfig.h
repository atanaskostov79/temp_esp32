#ifndef BT_CONFIG_H
#define BT_CONFIG_H

#include <BluetoothSerial.h>
#include <Preferences.h>
#include "wifiManager.h"

extern BluetoothSerial SerialBT;
extern String btBuffer;

// Global variables
extern String currentSSID;
extern String currentPASS;
extern bool credentialsReceived;
extern Preferences preferences;

// Function declarations
void processBTInput(const String& input);
void handleBluetoothInput();
void startBluetooth();
void saveWiFiCredentials(const String& ssid, const String& pass);
void processBluetoothCommands();

// Function to parse BT data and save credentials
inline void parseBtData(String data) {
    int ssidEnd = data.indexOf(';');
    if (ssidEnd > 0) {
        String ssid = data.substring(0, ssidEnd);
        String pass = data.substring(ssidEnd + 1);
        
        Serial.printf("Got SSID: %s\n", ssid.c_str());
        Serial.printf("Got Pass: %s\n", pass.c_str());
        
        saveCredentials(ssid, pass);
    }
}

#endif
