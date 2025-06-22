#include "screenManager.h"

unsigned long lastScreenSwitch = 0;
const unsigned long screenInterval = 5000;

void updateScreen(Adafruit_ST7789& tft) {
    unsigned long now = millis();
    if (now - lastScreenSwitch >= screenInterval) {
        currentScreen = (currentScreen == WEATHER) ? CLOCK : WEATHER;
        lastScreenSwitch = now;
    }
    switch (currentScreen) {
        case WEATHER:
            showWeatherScreen(tft);
            break;
        case CLOCK:
            showClockScreen(tft);
            break;
    }
} 