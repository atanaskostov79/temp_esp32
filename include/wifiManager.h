#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <Preferences.h>

extern bool wifiConnected;

void saveCredentials(const String& ssid, const String& pass);
bool loadCredentials(String& ssid, String& pass);
void clearCredentials();
void connectToWiFi();

#endif
