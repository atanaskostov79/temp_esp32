#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <Adafruit_ST7789.h>

// Глобални променливи
extern SemaphoreHandle_t dataMutex;

// Enum за състоянията на екрана
enum ScreenState {
    CLOCK,
    WEATHER,
    ZEN_QUOTE,
    DHT_SENSOR
};
extern ScreenState currentScreen;

// Декларации на функциите за показване на екрани
void showClockScreen(Adafruit_ST7789& tft, bool forceRedraw = false);
void showWeatherScreen(Adafruit_ST7789& tft, bool forceRedraw = false);
void showZenQuoteScreen(Adafruit_ST7789& tft, bool forceRedraw = false);
void showDhtSensorScreen(Adafruit_ST7789& tft, bool forceRedraw = false);

extern unsigned long lastScreenSwitch;
extern const unsigned long screenInterval;

void updateScreen(Adafruit_ST7789& tft);

#endif
