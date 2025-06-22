#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <BluetoothSerial.h>
#include <WiFi.h>
#include <Preferences.h>
#include <freertos/semphr.h>
#include <vector>
#include <string>

#include "wifiManager.h"
#include "weatherScreen.h"
#include "clockScreen.h"
#include "btConfig.h"

#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST   4

#define DOOM_ORANGE 0xFD20 // По-тъмен оранжев за по-добра четимост

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Preferences preferences;
BluetoothSerial SerialBT;
SemaphoreHandle_t dataMutex;

// Обща функция за рисуване на фоновото изображение
void drawBackground(Adafruit_ST7789& tft) {
    // Рисуваме bitmap изображението като фон
    tft.drawRGBBitmap(0, 0, my_image, MY_IMAGE_WIDTH, MY_IMAGE_HEIGHT);
}

enum ScreenType { WEATHER, CLOCK };
ScreenType currentScreen = WEATHER;

unsigned long screenTimer = 0;

// Нишка (Task) за обновяване на времето
void weatherUpdateTask(void *pvParameters) {
    Serial.println("Weather update task started.");
    for (;;) {
        updateWeatherData();
        // Изчакваме 5 минути до следващото обновяване
        vTaskDelay(300000 / portTICK_PERIOD_MS);
    }
}

// Центрира всеки ред поотделно
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

// Показва статус в центъра (мултиредово)
void showStatus(const char* msg, uint16_t color = DOOM_ORANGE, int y = 100) {
    showStatusMultiline(msg, color, y, 2, 8);
}

// Показва SSID и IP при свързване
void showWiFiInfo() {
    drawBackground(tft);  // Използваме фоновото изображение
    
    // Заглавие
    tft.setTextColor(DOOM_ORANGE);
    tft.setTextSize(2);
    tft.setCursor(80, 20);
    tft.print("WiFi Connected");
    
    // SSID
    tft.setTextColor(ST77XX_GREEN);
    tft.setTextSize(2);
    tft.setCursor(20, 60);
    tft.print("SSID: ");
    tft.println(WiFi.SSID());

    // IP адрес
    tft.setCursor(20, 100);
    tft.print("IP: ");
    tft.println(WiFi.localIP());
    
    // Статус
    tft.setTextColor(DOOM_ORANGE);
    tft.setCursor(20, 140);
    tft.print("Signal: ");
    tft.print(WiFi.RSSI());
    tft.print(" dBm");
}

void setup() {
    Serial.begin(115200);
    
    tft.init(170, 320);
    tft.setRotation(1); // Хоризонтален режим
    
    showStatus("DOOM ESP32", DOOM_ORANGE, 60);
    delay(1000);
    showStatus("Booting...", DOOM_ORANGE, 100);
    delay(500);

    dataMutex = xSemaphoreCreateMutex();
    if(dataMutex == NULL){
        showStatus("FATAL ERROR", ST77XX_RED, 60);
        showStatus("Mutex failed", ST77XX_RED, 100);
        while(1); // Stop execution
    }

    connectToWiFi();

    if (!wifiConnected) {
        startBluetooth();
    } else {
        showWiFiInfo();
        delay(3000);
        // Създаваме и стартираме нишката за времето на ядро 0
        xTaskCreatePinnedToCore(
            weatherUpdateTask,    // Функция на нишката
            "WeatherTask",        // Име
            10000,                // Размер на стека
            NULL,                 // Параметри
            1,                    // Приоритет
            NULL,                 // Handle
            0);                   // Ядро 0
    }
}

void loop() {
    if (wifiConnected) {
        if (millis() - screenTimer > 10000) {
            screenTimer = millis();
            currentScreen = (currentScreen == CLOCK) ? WEATHER : CLOCK;
            
            // Force a full redraw when switching screens
            if (currentScreen == CLOCK) {
                showClockScreen(tft, true);
            } else {
                showWeatherScreen(tft, true);
            }
        }

        // The running task handles updates, just draw the current state
        switch (currentScreen) {
            case WEATHER:
                showWeatherScreen(tft);
                break;
            case CLOCK:
                showClockScreen(tft);
                break;
        }
    } else {
        processBluetoothCommands();
    }
    delay(100);
}
