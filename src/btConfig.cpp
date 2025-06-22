#include "btConfig.h"
#include "wifiManager.h"
#include <Preferences.h>
#include <BluetoothSerial.h>

// Global variable definitions
extern BluetoothSerial SerialBT;
extern void showStatus(const char* msg, uint16_t color, int y);
String btBuffer = "";
String currentSSID = "";
String currentPASS = "";
bool credentialsReceived = false;

// This file is almost empty because most logic is now inlined in the header
// or handled directly in other managers. It's kept for potential future expansion
// of Bluetooth functionalities.

void processBTInput(const String& input) {
    if (input.startsWith("SSID:")) {
        currentSSID = input.substring(5);
        Serial.println("📶 SSID set to: " + currentSSID);
    } else if (input.startsWith("PASS:")) {
        currentPASS = input.substring(5);
        Serial.println("🔑 PASS set to: " + currentPASS);
    } else if (input == "CLEAR") {
        preferences.begin("wifi", false);
        preferences.clear();
        preferences.end();
        Serial.println("🗑️ Wi-Fi credentials cleared.");
        currentSSID = "";
        currentPASS = "";
        credentialsReceived = false;
    }

    if (!currentSSID.isEmpty() && !currentPASS.isEmpty()) {
        credentialsReceived = true;
        saveWiFiCredentials(currentSSID, currentPASS);
    }
}

void handleBluetoothInput() {
    while (SerialBT.available()) {
        char c = SerialBT.read();
        if (c == '\n') {
            btBuffer.trim();
            processBTInput(btBuffer);
            btBuffer = "";
        } else {
            btBuffer += c;
        }
    }
}

static String temp_ssid = "";
static String temp_pass = "";

void startBluetooth() {
    temp_ssid = "";
    temp_pass = "";
    SerialBT.begin("DOOM_Clock_Config");
    Serial.println("Bluetooth device is ready to pair.");
    showStatus("Send SSID & PASS\nvia Bluetooth", 0xFFFF, 60);
    SerialBT.println("Send in format:\nSSID:your_ssid\nPASS:your_password");
}

void processBluetoothCommands() {
    if (SerialBT.available()) {
        String input = SerialBT.readStringUntil('\n');
        input.trim();
        Serial.printf("Received BT line: '%s'\n", input.c_str());

        if (input.startsWith("SSID:")) {
            temp_ssid = input.substring(5);
            Serial.printf("SSID stored: '%s'\n", temp_ssid.c_str());
            SerialBT.println("OK. SSID received. Send PASS.");
            showStatus("SSID OK. Send PASS", 0xFFFF, 80);

        } else if (input.startsWith("PASS:")) {
            temp_pass = input.substring(5);
            Serial.printf("PASS stored: '%s'\n", temp_pass.c_str());
            SerialBT.println("OK. PASS received.");
            showStatus("PASS OK.", 0xFFFF, 80);
        
        } else {
            Serial.println("Invalid format received.");
            SerialBT.println("Invalid format.");
            showStatus("Invalid Format!", 0xF800, 80);
            delay(1500);
            showStatus("Send SSID & PASS\nvia Bluetooth", 0xFFFF, 60);
        }

        if (temp_ssid.length() > 0 && temp_pass.length() > 0) {
            Serial.println("Both SSID and PASS received. Saving...");
            saveCredentials(temp_ssid, temp_pass);

            showStatus("Credentials Saved!\nRebooting...", 0x07E0, 80);
            delay(2000);
            ESP.restart();
        }
    }
}

void saveWiFiCredentials(const String& ssid, const String& pass) {
    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("pass", pass);
    preferences.end();
    Serial.println("💾 WiFi credentials saved");
} 