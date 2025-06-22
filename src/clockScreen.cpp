#include "clockScreen.h"

void drawDoomFrame(Adafruit_ST7789& tft) {
    tft.fillRect(10, 10, 20, 3, ST77XX_WHITE);
    tft.fillRect(290, 10, 20, 3, ST77XX_WHITE);
    tft.fillRect(10, 157, 20, 3, ST77XX_WHITE);
    tft.fillRect(290, 157, 20, 3, ST77XX_WHITE);
    tft.fillRect(10, 10, 3, 20, ST77XX_WHITE);
    tft.fillRect(307, 10, 3, 20, ST77XX_WHITE);
    tft.fillRect(10, 140, 3, 20, ST77XX_WHITE);
    tft.fillRect(307, 140, 3, 20, ST77XX_WHITE);
}

void drawTextWithBg(Adafruit_ST7789& tft, const String& text, int16_t x, int16_t y, uint8_t size, uint16_t textColor, uint16_t bgColor) {
    int16_t x1, y1;
    uint16_t w, h;
    tft.setTextSize(size);
    tft.getTextBounds(text.c_str(), x, y, &x1, &y1, &w, &h);
    int16_t bg_x = x1 - 4;
    int16_t bg_y = y1 - 4;
    uint16_t bg_w = w + 8;
    uint16_t bg_h = h + 8;
    if (bg_x < 0) bg_x = 0;
    if (bg_y < 0) bg_y = 0;
    tft.fillRect(bg_x, bg_y, bg_w, bg_h, bgColor);
    tft.setCursor(x, y);
    tft.setTextColor(textColor);
    tft.print(text);
}

void showClockScreen(Adafruit_ST7789& tft, bool forceRedraw) {
    static bool ntpInitialized = false;
    static char lastTimeStr[16] = "";
    static char lastDateStr[32] = "";
    static bool screenHasBeenDrawn = false;
    if (forceRedraw) {
        screenHasBeenDrawn = false;
    }
    if (!ntpInitialized) {
        configTime(7200, 3600, "pool.ntp.org", "time.nist.gov");
        ntpInitialized = true;
    }
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to get time");
        return;
    }
    char timeStr[16];
    char dateStr[32];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);
    bool timeChanged = (strcmp(timeStr, lastTimeStr) != 0);
    bool dateChanged = (strcmp(dateStr, lastDateStr) != 0);
    if (!timeChanged && !dateChanged && screenHasBeenDrawn) {
        return;
    }
    if (!screenHasBeenDrawn || dateChanged || forceRedraw) {
        drawBackground(tft);
        drawDoomFrame(tft);
        tft.setTextSize(2);
        tft.setTextColor(DOOM_ORANGE);
        int16_t x1_date, y1_date;
        uint16_t w_date, h_date;
        tft.getTextBounds(dateStr, 0, 0, &x1_date, &y1_date, &w_date, &h_date);
        int16_t x_date = (320 - w_date) / 2;
        int16_t y_date = 130;
        tft.setCursor(x_date, y_date);
        tft.print(dateStr);
    }
    if (timeChanged) {
        tft.setTextSize(4);
        int16_t x1, y1;
        uint16_t w, h;
        tft.getTextBounds("00:00:00", 0, 0, &x1, &y1, &w, &h);
        int16_t x = (320 - w) / 2;
        int16_t y = 80;
        tft.fillRect(x - 5, y - 5, w + 10, h + 10, ST77XX_BLACK);
    }
    tft.setTextSize(4);
    tft.setTextColor(DOOM_ORANGE);
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);
    int16_t x = (320 - w) / 2;
    int16_t y = 80;
    tft.setCursor(x, y);
    tft.print(timeStr);
    strcpy(lastTimeStr, timeStr);
    strcpy(lastDateStr, dateStr);
    screenHasBeenDrawn = true;
    Serial.print("🕒 ");
    Serial.println(timeStr);
} 