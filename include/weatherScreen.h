#ifndef WEATHER_SCREEN_H
#define WEATHER_SCREEN_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "clockScreen.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

extern SemaphoreHandle_t dataMutex;
void drawBackground(Adafruit_ST7789& tft);

#define DOOM_ORANGE 0xFD20

extern float cachedTemp;
extern float cachedWind;
extern int cachedWeatherCode;
extern int cachedIsDay;
extern int cachedWindDirection;
extern String cachedLastUpdate;
extern bool isWeatherValid;
extern char lastDrawnTime[32];

String getWeatherDescription(int code);
String getWindDirection(int degrees);
void updateWeatherData();
void drawWeatherIcon(Adafruit_ST7789& tft, int x, int y, int code, bool isDay);
uint16_t getTempColor(float temp);
void showWeatherScreen(Adafruit_ST7789& tft, bool forceRedraw);

#endif 