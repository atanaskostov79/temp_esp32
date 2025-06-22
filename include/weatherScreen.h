#ifndef WEATHER_SCREEN_H
#define WEATHER_SCREEN_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "clockScreen.h"  // За достъп до drawBackground
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

extern SemaphoreHandle_t dataMutex;
void drawBackground(Adafruit_ST7789& tft);

#define DOOM_ORANGE 0xFD20 // По-тъмен оранжев за по-добра четимост

// Кеширани стойности
float cachedTemp = 0;
float cachedWind = 0;
int cachedWeatherCode = 0;
int cachedIsDay = 1;
int cachedWindDirection = 0;
String cachedLastUpdate = "";
bool isWeatherValid = false;
char lastDrawnTime[32] = "";

// Функция за получаване на описание на времето
String getWeatherDescription(int code) {
    if (code == 0) return "Clear";
    else if (code >= 1 && code <= 3) return "Cloudy";
    else if (code >= 4 && code <= 9) return "Fog";
    else if (code >= 10 && code <= 19) return "Rain";
    else if (code >= 20 && code <= 29) return "Snow";
    else if (code >= 30 && code <= 39) return "Storm";
    else if (code >= 40 && code <= 49) return "Fog";
    else if (code >= 50 && code <= 59) return "Drizzle";
    else if (code >= 60 && code <= 69) return "Rain";
    else if (code >= 70 && code <= 79) return "Snow";
    else if (code >= 80 && code <= 89) return "Shower";
    else if (code >= 90 && code <= 99) return "Thunder";
    else return "Unknown";
}

// Функция за получаване на посока на вятъра
String getWindDirection(int degrees) {
    if (degrees >= 315 || degrees < 45) return "N";
    else if (degrees >= 45 && degrees < 135) return "E";
    else if (degrees >= 135 && degrees < 225) return "S";
    else if (degrees >= 225 && degrees < 315) return "W";
    else return "N";
}

void updateWeatherData() {
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        HTTPClient http;
        http.begin("https://api.open-meteo.com/v1/forecast?latitude=42.125&longitude=24.75&current_weather=true");
        
        int httpCode = http.GET();
        bool error = false;
        
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            DynamicJsonDocument doc(1024);
            DeserializationError jsonError = deserializeJson(doc, payload);
            
            if (!jsonError && doc.containsKey("current_weather")) {
                JsonObject weather = doc["current_weather"];
                
                cachedTemp = weather["temperature"] | 0.0;
                cachedWind = weather["windspeed"] | 0.0;
                cachedWeatherCode = weather["weathercode"] | 0;
                cachedIsDay = weather["is_day"] | 1;
                cachedWindDirection = weather["winddirection"] | 0;
                cachedLastUpdate = weather["time"] | "";
                
                isWeatherValid = true;
                Serial.printf("Weather updated: %.1f°C, %.1f km/h, Code: %d, Day: %d, Wind: %d°\n", 
                            cachedTemp, cachedWind, cachedWeatherCode, cachedIsDay, cachedWindDirection);
            } else {
                error = true;
                Serial.println("JSON parsing failed");
            }
        } else {
            error = true;
            Serial.printf("HTTP request failed: %d\n", httpCode);
        }
        
        if (error) {
            isWeatherValid = false;
        }
        
        http.end();
        xSemaphoreGive(dataMutex);
    }
}

void showWeatherScreen(Adafruit_ST7789& tft, bool forceRedraw = false) {
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return;
    }
    
    float currentTemp = cachedTemp;
    float currentWind = cachedWind;
    int currentWeatherCode = cachedWeatherCode;
    int currentIsDay = cachedIsDay;
    int currentWindDirection = cachedWindDirection;
    String currentLastUpdate = cachedLastUpdate;
    bool currentIsValid = isWeatherValid;
    
    xSemaphoreGive(dataMutex);
    
    char currentTime[32];
    strcpy(currentTime, currentLastUpdate.c_str());
    
    if (!forceRedraw && strcmp(currentTime, lastDrawnTime) == 0) {
        return;
    }
    
    strcpy(lastDrawnTime, currentTime);

    // Full screen redraw
    drawBackground(tft);

    // Заглавие
    tft.setTextSize(2);
    tft.setTextColor(DOOM_ORANGE);
    tft.setCursor(70, 15);
    tft.print("PLOVDIV WEATHER");

    if (!currentIsValid) {
        tft.setTextSize(3);
        tft.setTextColor(ST77XX_RED);
        tft.setCursor(90, 80);
        tft.print("No Data");
        return;
    }
    
    // Основна информация
    tft.setTextSize(2);
    tft.setTextColor(DOOM_ORANGE);
    
    // Температура
    char tempStr[16];
    sprintf(tempStr, "Temp: %.1f°C", currentTemp);
    tft.setCursor(20, 50);
    tft.print(tempStr);
    
    // Времето
    String weatherDesc = getWeatherDescription(currentWeatherCode);
    tft.setCursor(20, 75);
    tft.print("Weather: ");
    tft.print(weatherDesc);
    
    // Ден/Нощ
    tft.setCursor(20, 100);
    tft.print("Time: ");
    tft.print(currentIsDay ? "DAY" : "NIGHT");
    
    // Вятър
    char windStr[25];
    String windDir = getWindDirection(currentWindDirection);
    sprintf(windStr, "Wind: %.1f km/h %s", currentWind, windDir.c_str());
    tft.setCursor(20, 125);
    tft.print(windStr);
    
    // Последно обновяване
    tft.setTextSize(1);
    tft.setCursor(20, 150);
    tft.print("Updated: ");
    
    // Конвертираме от UTC към българско време (UTC+2/UTC+3)
    if (currentLastUpdate.length() >= 16) {
        String timeStr = currentLastUpdate.substring(11, 16); // Вземаме час:минути
        int hour = timeStr.substring(0, 2).toInt();
        int minute = timeStr.substring(3, 5).toInt();
        
        // Добавяме 2 часа за българско време (зима)
        // За лято може да добавим 3 часа
        hour += 2;
        if (hour >= 24) hour -= 24;
        
        char localTime[6];
        sprintf(localTime, "%02d:%02d", hour, minute);
        tft.print(localTime);
    } else {
        tft.print(currentLastUpdate.substring(11, 16)); // Fallback
    }
}

#endif
