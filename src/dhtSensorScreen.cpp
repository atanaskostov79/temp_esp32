#include "dhtSensorScreen.h"
#include "my_image.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

extern void drawBackground(Adafruit_ST7789& tft);
#define DOOM_ORANGE 0xFD20

float dhtTemperature = 0;
float dhtHumidity = 0;
bool dhtValid = false;

// DHT11 task handle
TaskHandle_t dhtTaskHandle = NULL;

// Custom DHT11 implementation
bool readDHT11(uint8_t pin, float* temperature, float* humidity) {
    uint8_t data[5] = {0};
    uint8_t bitIndex = 0;
    uint8_t byteIndex = 0;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delayMicroseconds(DHT11_START_SIGNAL_TIME);
    digitalWrite(pin, HIGH);
    delayMicroseconds(40);
    pinMode(pin, INPUT_PULLUP);
    unsigned long startTime = micros();
    while (digitalRead(pin) == HIGH) {
        if (micros() - startTime > DHT11_RESPONSE_TIMEOUT) return false;
    }
    startTime = micros();
    while (digitalRead(pin) == LOW) {
        if (micros() - startTime > DHT11_RESPONSE_TIMEOUT) return false;
    }
    startTime = micros();
    while (digitalRead(pin) == HIGH) {
        if (micros() - startTime > DHT11_RESPONSE_TIMEOUT) return false;
    }
    for (int i = 0; i < 40; i++) {
        startTime = micros();
        while (digitalRead(pin) == LOW) {
            if (micros() - startTime > DHT11_DATA_TIMEOUT) return false;
        }
        startTime = micros();
        while (digitalRead(pin) == HIGH) {
            if (micros() - startTime > DHT11_DATA_TIMEOUT) return false;
        }
        unsigned long pulseWidth = micros() - startTime;
        if (pulseWidth > 50) {
            data[byteIndex] |= (1 << (7 - bitIndex));
        }
        bitIndex++;
        if (bitIndex == 8) {
            bitIndex = 0;
            byteIndex++;
        }
    }
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) return false;
    *humidity = (float)data[0] + (float)data[1] / 100.0;
    *temperature = (float)data[2] + (float)data[3] / 100.0;
    if (*humidity < 0 || *humidity > 100 || *temperature < -40 || *temperature > 80) return false;
    return true;
}

void dht11Task(void* pvParameters) {
    for (;;) {
        float temp, hum;
        bool ok = readDHT11(DHTPIN, &temp, &hum);
        if (ok) {
            dhtTemperature = temp;
            dhtHumidity = hum;
            dhtValid = true;
        } else {
            dhtValid = false;
        }
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void setupDht() {
    if (dhtTaskHandle == NULL) {
        xTaskCreatePinnedToCore(dht11Task, "DHT11Task", 2048, NULL, 1, &dhtTaskHandle, 1);
    }
}

void updateDhtData() { /* вече не се ползва, но оставям за съвместимост */ }

void showDhtSensorScreen(Adafruit_ST7789& tft, bool forceRedraw) {
    static float lastTemp = -999;
    static float lastHumidity = -999;
    static bool lastValid = false;
    bool newData = false;
    float temp = 0, hum = 0;
    bool ok = false;

    if (forceRedraw) {
        drawBackground(tft);
        tft.fillRect(10, 10, 20, 3, ST77XX_WHITE);
        tft.fillRect(290, 10, 20, 3, ST77XX_WHITE);
        tft.fillRect(10, 157, 20, 3, ST77XX_WHITE);
        tft.fillRect(290, 157, 20, 3, ST77XX_WHITE);
        tft.fillRect(10, 10, 3, 20, ST77XX_WHITE);
        tft.fillRect(307, 10, 3, 20, ST77XX_WHITE);
        tft.fillRect(10, 140, 3, 20, ST77XX_WHITE);
        tft.fillRect(307, 140, 3, 20, ST77XX_WHITE);
        tft.setTextSize(2);
        tft.setTextColor(DOOM_ORANGE);
        tft.setCursor(60, 20);
        tft.print("DHT11 SENSOR");

        for (int i = 0; i < 3; ++i) {
            ok = readDHT11(DHTPIN, &temp, &hum);
            if (ok) {
                break;
            }
            delay(200);
        }
        if (ok) {
            dhtTemperature = temp;
            dhtHumidity = hum;
            dhtValid = true;
            lastTemp = temp;
            lastHumidity = hum;
            lastValid = true;
            newData = true;
        } else {
            dhtValid = false;
            newData = false;
        }
    } else if (!lastValid) {
        for (int i = 0; i < 2; ++i) {
            ok = readDHT11(DHTPIN, &temp, &hum);
            if (ok) {
                break;
            }
            delay(100);
        }
        if (ok) {
            dhtTemperature = temp;
            dhtHumidity = hum;
            dhtValid = true;
            lastTemp = temp;
            lastHumidity = hum;
            lastValid = true;
            newData = true;
        }
    }

    // Показваме данни само ако има нови или има последно валидни
    if (newData || lastValid) {
        tft.setTextSize(2);
        tft.setTextColor(DOOM_ORANGE);
        tft.setCursor(40, 70);
        tft.print("Temp: ");
        tft.print(lastTemp, 1);
        tft.print(" C");
        tft.setCursor(40, 110);
        tft.print("Humidity: ");
        tft.print(lastHumidity, 1);
        tft.print(" %");
    }
} 