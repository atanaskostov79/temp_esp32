#include "weatherScreen.h"

float cachedTemp = 0;
float cachedWind = 0;
int cachedWeatherCode = 0;
int cachedIsDay = 1;
int cachedWindDirection = 0;
String cachedLastUpdate = "";
bool isWeatherValid = false;
char lastDrawnTime[32] = "";

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

String getWindDirection(int degrees) {
    if (degrees >= 315 || degrees < 45) return "N";
    else if (degrees >= 45 && degrees < 135) return "E";
    else if (degrees >= 135 && degrees < 225) return "S";
    else if (degrees >= 225 && degrees < 315) return "W";
    else return "N";
}

void updateWeatherData() {
    Serial.println("=== WEATHER UPDATE START ===");
    HTTPClient http;
    http.begin("https://api.open-meteo.com/v1/forecast?latitude=42.125&longitude=24.75&current_weather=true");
    Serial.println("Making HTTP request to weather API...");
    int httpCode = http.GET();
    Serial.printf("Weather API HTTP Code: %d\n", httpCode);
    bool error = false;
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.printf("Weather API Response: %s\n", payload.c_str());
        DynamicJsonDocument doc(2048);
        DeserializationError jsonError = deserializeJson(doc, payload);
        if (!jsonError) {
            Serial.println("JSON parsed successfully");
            if (doc.containsKey("current_weather")) {
                Serial.println("Found current_weather key");
                JsonObject weather = doc["current_weather"];
                cachedTemp = weather["temperature"] | 0.0;
                cachedWind = weather["windspeed"] | 0.0;
                cachedWeatherCode = weather["weathercode"] | 0;
                cachedIsDay = weather["is_day"] | 1;
                cachedWindDirection = weather["winddirection"] | 0;
                cachedLastUpdate = weather["time"] | "";
                isWeatherValid = true;
                Serial.printf("Weather updated successfully: %.1f°C, %.1f km/h, Code: %d, Day: %d, Wind: %d°\n", 
                            cachedTemp, cachedWind, cachedWeatherCode, cachedIsDay, cachedWindDirection);
                Serial.printf("isWeatherValid set to: %s\n", isWeatherValid ? "true" : "false");
            } else {
                error = true;
                Serial.println("ERROR: current_weather key not found in JSON");
            }
        } else {
            error = true;
            Serial.printf("JSON parsing failed: %s\n", jsonError.c_str());
        }
    } else {
        error = true;
        Serial.printf("HTTP request failed: %d\n", httpCode);
    }
    if (error) {
        isWeatherValid = false;
        Serial.println("ERROR: Setting isWeatherValid to false");
    }
    http.end();
    Serial.println("=== WEATHER UPDATE END ===");
}

const uint8_t SUN_ICON[] = {0};
const uint8_t MOON_ICON[] = {0};
const uint8_t CLOUD_ICON[] = {0};
const uint8_t RAIN_ICON[] = {0};
const uint8_t SNOW_ICON[] = {0};
const uint8_t STORM_ICON[] = {0};

void drawWeatherIcon(Adafruit_ST7789& tft, int x, int y, int code, bool isDay) {
    if (isDay) {
        tft.fillCircle(x, y, 10, DOOM_ORANGE);
    } else {
        tft.fillCircle(x, y, 10, ST77XX_WHITE);
        tft.fillCircle(x - 3, y - 3, 8, ST77XX_BLACK);
    }
}

uint16_t getTempColor(float temp) {
    if (temp < 0) return ST77XX_CYAN;
    else if (temp < 10) return ST77XX_BLUE;
    else if (temp < 20) return ST77XX_YELLOW;
    else if (temp < 30) return DOOM_ORANGE;
    else return ST77XX_RED;
}

float lastTemp = 0;
int lastTrend = 0;

void showWeatherScreen(Adafruit_ST7789& tft, bool forceRedraw) {
    float currentTemp = cachedTemp;
    float currentWind = cachedWind;
    int currentWeatherCode = cachedWeatherCode;
    int currentIsDay = cachedIsDay;
    int currentWindDirection = cachedWindDirection;
    String currentLastUpdate = cachedLastUpdate;
    bool currentIsValid = isWeatherValid;
    
    Serial.printf("Weather Screen - Temp: %.1f, Wind: %.1f, Code: %d, Day: %d, Valid: %s\n", 
                  currentTemp, currentWind, currentWeatherCode, currentIsDay, currentIsValid ? "true" : "false");
    
    char currentTime[32];
    strcpy(currentTime, currentLastUpdate.c_str());
    if (!forceRedraw && strcmp(currentTime, lastDrawnTime) == 0) {
        Serial.println("Weather Screen - Skipping redraw (same data)");
        return;
    }
    strcpy(lastDrawnTime, currentTime);
    
    Serial.println("Weather Screen - Drawing screen...");
    
    // Рисуваме фона
    drawBackground(tft);
    taskYIELD();
    
    // Рисуваме рамката
    tft.fillRect(10, 10, 20, 3, ST77XX_WHITE);
    tft.fillRect(290, 10, 20, 3, ST77XX_WHITE);
    tft.fillRect(10, 157, 20, 3, ST77XX_WHITE);
    tft.fillRect(290, 157, 20, 3, ST77XX_WHITE);
    tft.fillRect(10, 10, 3, 20, ST77XX_WHITE);
    tft.fillRect(307, 10, 3, 20, ST77XX_WHITE);
    tft.fillRect(10, 140, 3, 20, ST77XX_WHITE);
    tft.fillRect(307, 140, 3, 20, ST77XX_WHITE);
    taskYIELD();
    
    // Заглавие
    tft.setTextSize(2);
    tft.setTextColor(DOOM_ORANGE);
    tft.setCursor(70, 15);
    tft.print("PLOVDIV WEATHER");
    
    if (!currentIsValid) {
        Serial.println("Weather Screen - Showing 'No Data' message");
        tft.setTextSize(3);
        tft.setTextColor(ST77XX_RED);
        tft.setCursor(90, 80);
        tft.print("No Data");
        return;
    }
    
    Serial.println("Weather Screen - Drawing weather data...");
    
    // Икона и ден/нощ
    drawWeatherIcon(tft, 35, 60, currentWeatherCode, currentIsDay);
    tft.setTextSize(2);
    tft.setCursor(25, 85);
    tft.setTextColor(currentIsDay ? ST77XX_YELLOW : ST77XX_WHITE);
    tft.print(currentIsDay ? "DAY" : "NIGHT");
    taskYIELD();
    
    // Температура
    tft.setTextSize(3);
    tft.setCursor(100, 50);
    tft.setTextColor(getTempColor(currentTemp));
    char tempStr[16];
    sprintf(tempStr, "%.1f", currentTemp);
    tft.print(tempStr);
    
    int16_t tempX = tft.getCursorX();
    int16_t tempY = tft.getCursorY();
    tft.drawCircle(tempX + 8, tempY + 4, 3, getTempColor(currentTemp));
    tft.setCursor(tempX + 15, tempY);
    tft.print("C");
    
    // Тренд
    if (currentTemp > lastTemp) { lastTrend = 1; }
    else if (currentTemp < lastTemp) { lastTrend = -1; }
    else { lastTrend = 0; }
    lastTemp = currentTemp;
    
    tft.setTextColor(lastTrend == 1 ? ST77XX_GREEN : (lastTrend == -1 ? ST77XX_RED : ST77XX_WHITE));
    tft.setTextSize(3);
    tft.print(lastTrend == 1 ? " ^" : (lastTrend == -1 ? " v" : ""));
    taskYIELD();
    
    // Описание на времето
    tft.setTextColor(DOOM_ORANGE);
    tft.setTextSize(2);
    String weatherDesc = getWeatherDescription(currentWeatherCode);
    tft.setCursor(100, 85);
    tft.print(weatherDesc);
    
    // Вятър
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(100, 110);
    char windStr[30];
    String windDir = getWindDirection(currentWindDirection);
    tft.setTextSize(2);
    const char* arrow;
    if (windDir == "N") arrow = "^";
    else if (windDir == "E") arrow = ">";
    else if (windDir == "S") arrow = "v";
    else if (windDir == "W") arrow = "<";
    else arrow = "";
    sprintf(windStr, "%.1f km/h %s %s", currentWind, windDir.c_str(), arrow);
    tft.print(windStr);
    taskYIELD();
    
    // Време на обновяване
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(220, 155);
    tft.print("Updated: ");
    if (currentLastUpdate.length() >= 16) {
        String timeStr = currentLastUpdate.substring(11, 16);
        int hour = timeStr.substring(0, 2).toInt();
        int minute = timeStr.substring(3, 5).toInt();
        hour += 2;
        if (hour >= 24) hour -= 24;
        char localTime[6];
        sprintf(localTime, "%02d:%02d", hour, minute);
        tft.print(localTime);
    } else {
        tft.print(currentLastUpdate.substring(11, 16));
    }
    
    Serial.println("Weather Screen - Drawing completed");
} 