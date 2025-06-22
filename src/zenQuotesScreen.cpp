#include "zenQuotesScreen.h"
#include "my_image.h"

extern void drawBackground(Adafruit_ST7789& tft);
extern void drawDoomFrame(Adafruit_ST7789& tft);

String cachedQuote = "Loading quotes...";
String cachedAuthor = "Please wait";
bool isQuoteValid = false;

void updateQuoteData() {
    HTTPClient http;
    http.begin("https://zenquotes.io/api/today");
    http.setTimeout(10000);
    
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
            if (doc.is<JsonArray>()) {
                JsonArray array = doc.as<JsonArray>();
                if (array.size() > 0) {
                    JsonObject quoteObj = array[0];
                    if (quoteObj.containsKey("q") && quoteObj.containsKey("a")) {
                        cachedQuote = quoteObj["q"].as<String>();
                        cachedAuthor = quoteObj["a"].as<String>();
                        isQuoteValid = true;
                    } else {
                        isQuoteValid = false;
                    }
                } else {
                    isQuoteValid = false;
                }
            } else {
                isQuoteValid = false;
            }
        } else {
            isQuoteValid = false;
        }
    } else {
        isQuoteValid = false;
    }
    
    http.end();
}

void showZenQuoteScreen(Adafruit_ST7789& tft, bool forceRedraw) {
    if (forceRedraw) {
        drawBackground(tft);
        drawDoomFrame(tft);
        
        tft.setTextSize(2);
        tft.setTextColor(DOOM_ORANGE);
        tft.setCursor(80, 15);
        tft.print("Quote of the Day");
    }
    
    if (isQuoteValid) {
        tft.setTextSize(2);
        tft.setTextColor(DOOM_ORANGE);
        tft.setCursor(25, 50);
        
        // Разделяме цитата на редове ако е твърде дълга
        if (cachedQuote.length() > 25) {
            String firstLine = cachedQuote.substring(0, 25);
            String secondLine = cachedQuote.substring(25, 50);
            tft.print(firstLine);
            tft.setCursor(25, 80);
            tft.print(secondLine);
        } else {
            tft.print(cachedQuote);
        }
        
        tft.setTextSize(2);
        tft.setTextColor(DOOM_ORANGE);
        tft.setCursor(180, 140);
        tft.print("- " + cachedAuthor);
    } else {
        tft.setTextSize(2);
        tft.setTextColor(ST77XX_RED);
        tft.setCursor(100, 80);
        tft.print("No Quote Data");
        
        tft.setTextSize(1);
        tft.setTextColor(ST77XX_YELLOW);
        tft.setCursor(50, 120);
        tft.print("Check WiFi connection");
    }
} 