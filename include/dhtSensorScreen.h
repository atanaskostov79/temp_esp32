#ifndef DHT_SENSOR_SCREEN_H
#define DHT_SENSOR_SCREEN_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Arduino.h>

#define DHTPIN 5
#define DHTTYPE DHT11

// DHT11 timing constants
#define DHT11_START_SIGNAL_TIME 18000  // 18ms
#define DHT11_RESPONSE_TIMEOUT 100     // 100us timeout
#define DHT11_DATA_TIMEOUT 100         // 100us timeout

extern float dhtTemperature;
extern float dhtHumidity;
extern bool dhtValid;

// Custom DHT11 functions
bool readDHT11(uint8_t pin, float* temperature, float* humidity);
void setupDht();
void updateDhtData();
void showDhtSensorScreen(Adafruit_ST7789& tft, bool forceRedraw);

#endif 