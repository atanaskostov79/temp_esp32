#ifndef ZEN_QUOTES_SCREEN_H
#define ZEN_QUOTES_SCREEN_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "screenManager.h"

#define DOOM_ORANGE 0xFD20

// Декларации, тъй като функциите са в други файлове
void drawBackground(Adafruit_ST7789& tft);
void drawDoomFrame(Adafruit_ST7789& tft);

// Глобални променливи за цитата и автора
extern String cachedQuote;
extern String cachedAuthor;
extern bool isQuoteValid;

// Функция за изтегляне на цитат
void updateQuoteData();

// Функция за показване на екрана с цитат
void showZenQuoteScreen(Adafruit_ST7789& tft, bool forceRedraw);

#endif 