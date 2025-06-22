#include <WiFi.h>
#include <Preferences.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "wifiManager.h"

bool wifiConnected = false;
// The 'preferences' object will be declared in one place only (e.g., main.cpp or a specific config file) 
// to avoid multiple definitions. For now, we assume it's accessible.
extern Preferences preferences;
extern void showStatus(const char* msg, uint16_t color, int y);

#define DOOM_ORANGE 0xFD20 // По-тъмен оранжев за по-добра четимост

void saveCredentials(const String& ssid, const String& pass) {
    preferences.begin("wifi-creds", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", pass);
    preferences.end();
    Serial.println("Credentials saved.");
}

bool loadCredentials(String& ssid, String& pass) {
    preferences.begin("wifi-creds", true);
    ssid = preferences.getString("ssid", "");
    pass = preferences.getString("password", "");
    preferences.end();
    return ssid.length() > 0;
}

void clearCredentials() {
    preferences.begin("wifi-creds", false);
    preferences.clear();
    preferences.end();
    Serial.println("Invalid credentials cleared.");
}

void connectToWiFi() {
    String ssid, pass;
    if (loadCredentials(ssid, pass)) {
        int attempts = 0;
        while (attempts < 3) {
            attempts++;
            char statusMsg[64];
            sprintf(statusMsg, "Connecting...\nAttempt %d of 3", attempts);
            showStatus(statusMsg, DOOM_ORANGE, 80);

            Serial.printf("Attempt %d to connect to WiFi '%s'...", attempts, ssid.c_str());
            WiFi.begin(ssid.c_str(), pass.c_str());

            int timeout_counter = 0;
            while (WiFi.status() != WL_CONNECTED && timeout_counter < 20) { // 10 sec timeout
                delay(500);
                Serial.print(".");
                timeout_counter++;
            }

            if (WiFi.status() == WL_CONNECTED) {
                Serial.println("\nConnected!");
                wifiConnected = true;
                return; 
            } else {
                Serial.println("\nFailed to connect.");
                WiFi.disconnect(true);
                delay(1000);
            }
        }

        Serial.println("All WiFi connection attempts failed. Clearing credentials.");
        showStatus("Connection Failed.\nClearing credentials.", ST77XX_RED, 60);
        delay(2000);
        clearCredentials();
        wifiConnected = false;

    } else {
        Serial.println("No WiFi credentials saved.");
        wifiConnected = false;
    }
}
