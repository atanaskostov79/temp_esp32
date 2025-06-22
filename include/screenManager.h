#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "weatherScreen.h"
#include "clockScreen.h"

enum ScreenState {
    WEATHER,
    CLOCK
};

ScreenState currentScreen = WEATHER;
unsigned long lastScreenSwitch = 0;
const unsigned long screenInterval = 5000; // 5 секунди

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

#endif
