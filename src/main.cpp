#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <BluetoothSerial.h>
#include <WiFi.h>
#include <Preferences.h>
#include <freertos/semphr.h>
#include <vector>
#include <string>

#include "screenManager.h"
#include "wifiManager.h"
#include "btConfig.h"
#include "weatherScreen.h"
#include "clockScreen.h"
#include "zenQuotesScreen.h"
#include "dhtSensorScreen.h"

#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST   4

#define DOOM_ORANGE 0xFD20

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Preferences preferences;
BluetoothSerial SerialBT;

ScreenState currentScreen;
SemaphoreHandle_t dataMutex;

void drawBackground(Adafruit_ST7789& tft) {
    tft.drawRGBBitmap(0, 0, my_image, MY_IMAGE_WIDTH, MY_IMAGE_HEIGHT);
}

void weatherUpdateTask(void *pvParameters) {
    for (;;) {
        updateWeatherData();
        vTaskDelay(pdMS_TO_TICKS(5 * 60 * 1000));
        taskYIELD();
    }
}

void quoteUpdateTask(void *pvParameters) {
    // Първоначално зареждане на цитатите веднага
    updateQuoteData();
    
    for (;;) {
        updateQuoteData();
        vTaskDelay(pdMS_TO_TICKS(15 * 60 * 1000));
        taskYIELD();
    }
}

void showStatusMultiline(const char* msg, uint16_t color = DOOM_ORANGE, int y = 100, uint8_t size = 2, int lineSpacing = 8) {
    drawBackground(tft);
    tft.setTextColor(color);
    tft.setTextSize(size);
    
    std::string s(msg);
    std::vector<std::string> lines;
    size_t pos = 0, prev = 0;
    while ((pos = s.find('\n', prev)) != std::string::npos) {
        lines.push_back(s.substr(prev, pos - prev));
        prev = pos + 1;
    }
    lines.push_back(s.substr(prev));
    
    int yPos = y;
    for (const auto& line : lines) {
        int16_t x1, y1;
        uint16_t w, h;
        tft.getTextBounds(line.c_str(), 0, 0, &x1, &y1, &w, &h);
        int16_t x = (320 - w) / 2;
        tft.setCursor(x, yPos);
        tft.print(line.c_str());
        yPos += h + lineSpacing;
    }
}

void showStatus(const char* msg, uint16_t color = DOOM_ORANGE, int y = 100) {
    showStatusMultiline(msg, color, y, 2, 8);
}

void showWiFiInfo() {
    drawBackground(tft);
    tft.fillRect(10, 10, 20, 3, ST77XX_WHITE);
    tft.fillRect(290, 10, 20, 3, ST77XX_WHITE);
    tft.fillRect(10, 157, 20, 3, ST77XX_WHITE);
    tft.fillRect(290, 157, 20, 3, ST77XX_WHITE);
    tft.fillRect(10, 10, 3, 20, ST77XX_WHITE);
    tft.fillRect(307, 10, 3, 20, ST77XX_WHITE);
    tft.fillRect(10, 140, 3, 20, ST77XX_WHITE);
    tft.fillRect(307, 140, 3, 20, ST77XX_WHITE);
    
    tft.setTextColor(DOOM_ORANGE);
    tft.setTextSize(2);
    tft.setCursor(80, 20);
    tft.print("WiFi Connected");
    
    tft.setTextColor(ST77XX_GREEN);
    tft.setTextSize(2);
    tft.setCursor(20, 60);
    tft.print("SSID: ");
    tft.println(WiFi.SSID());

    tft.setCursor(20, 100);
    tft.print("IP: ");
    tft.println(WiFi.localIP());
    
    tft.setTextColor(DOOM_ORANGE);
    tft.setCursor(20, 140);
    tft.print("Signal: ");
    tft.print(WiFi.RSSI());
    tft.print(" dBm");
}

void setup() {
    Serial.begin(115200);
    
    tft.init(170, 320);
    tft.setRotation(1);
    
    showStatus("DOOM ESP32", DOOM_ORANGE, 60);
    delay(1000);
    showStatus("Booting...", DOOM_ORANGE, 100);
    delay(500);

    dataMutex = xSemaphoreCreateMutex();
    if(dataMutex == NULL){
        showStatus("FATAL ERROR", ST77XX_RED, 60);
        showStatus("Mutex failed", ST77XX_RED, 100);
        while(1);
    }

    connectToWiFi();

    if (!wifiConnected) {
        startBluetooth();
    } else {
        showWiFiInfo();
        delay(2000);
        tft.fillScreen(ST77XX_BLACK);
    }

    if (WiFi.status() == WL_CONNECTED) {
        xTaskCreatePinnedToCore(
            weatherUpdateTask,
            "WeatherTask",
            4096,
            NULL,
            1,
            NULL,
            0);
            
        xTaskCreatePinnedToCore(
            quoteUpdateTask,
            "QuoteTask",
            4096,
            NULL,
            1,
            NULL,
            1);
    }

    setupDht();
    currentScreen = CLOCK;
}

void loop() {
    static unsigned long lastSwitchTime = 0;
    static bool forceRedraw = true;
    static unsigned long lastDhtRead = 0;
    static unsigned long loopCounter = 0;
    
    loopCounter++;
    
    // Смяна на екраните на всеки 10 секунди
    unsigned long currentTime = millis();
    if (currentTime - lastSwitchTime > 10000) {
        lastSwitchTime = currentTime;
        currentScreen = (ScreenState)((currentScreen + 1) % 4);
        forceRedraw = true;
    }

    // Опитваме да вземем mutex с по-кратко време за изчакване
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        switch (currentScreen) {
            case CLOCK:
                showClockScreen(tft, forceRedraw);
                break;
            case WEATHER:
                showWeatherScreen(tft, forceRedraw);
                break;
            case ZEN_QUOTE:
                showZenQuoteScreen(tft, forceRedraw);
                break;
            case DHT_SENSOR:
                showDhtSensorScreen(tft, forceRedraw);
                break;
        }
        xSemaphoreGive(dataMutex);
    }

    if (forceRedraw) {
        forceRedraw = false;
    }

    delay(200);
    taskYIELD();
}
