#ifndef CLOCK_SCREEN_H
#define CLOCK_SCREEN_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <time.h>
#include <WiFi.h>
#include "my_image.h"

#define ST77XX_SILVER 0xC618
#define DOOM_ORANGE 0xFD20

void drawBackground(Adafruit_ST7789& tft);
void drawDoomFrame(Adafruit_ST7789& tft);
void drawTextWithBg(Adafruit_ST7789& tft, const String& text, int16_t x, int16_t y, uint8_t size, uint16_t textColor, uint16_t bgColor);
void showClockScreen(Adafruit_ST7789& tft, bool forceRedraw);

#endif 