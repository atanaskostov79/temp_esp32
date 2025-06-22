#include <Arduino.h>

#define DHTPIN 5
#define DHTTYPE DHT11

// DHT11 timing constants
#define DHT11_START_SIGNAL_TIME 18000  // 18ms
#define DHT11_RESPONSE_TIMEOUT 100     // 100us timeout
#define DHT11_DATA_TIMEOUT 100         // 100us timeout

void setup() {
  Serial.begin(115200);
  Serial.println("=== DHT11 TEST START ===");
  Serial.printf("Using GPIO %d for DHT11\n", DHTPIN);
  
  // Test the sensor
  float testTemp, testHumidity;
  bool sensorOK = false;
  
  for (int i = 0; i < 5; i++) {
    Serial.printf("Test attempt %d...\n", i + 1);
    if (readDHT11(DHTPIN, &testTemp, &testHumidity)) {
      Serial.printf("SUCCESS! Temp: %.1f°C, Humidity: %.1f%%\n", testTemp, testHumidity);
      sensorOK = true;
      break;
    } else {
      Serial.println("FAILED - No response from sensor");
    }
    delay(2000);
  }
  
  if (!sensorOK) {
    Serial.println("ERROR: Sensor not responding after 5 attempts");
    Serial.println("Check wiring and pull-up resistor!");
  }
  
  Serial.println("=== DHT11 TEST END ===");
}

void loop() {
  // Continuous monitoring
  float temp, humidity;
  if (readDHT11(DHTPIN, &temp, &humidity)) {
    Serial.printf("READING: Temp=%.1f°C, Humidity=%.1f%%\n", temp, humidity);
  } else {
    Serial.println("READING: FAILED");
  }
  delay(3000);
}

// Custom DHT11 implementation
bool readDHT11(uint8_t pin, float* temperature, float* humidity) {
    uint8_t data[5] = {0};
    uint8_t bitIndex = 0;
    uint8_t byteIndex = 0;
    
    // Step 1: Send start signal
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delayMicroseconds(DHT11_START_SIGNAL_TIME);
    digitalWrite(pin, HIGH);
    delayMicroseconds(40);
    
    // Step 2: Switch to input and wait for response
    pinMode(pin, INPUT_PULLUP);
    
    // Wait for DHT11 to pull down (response start)
    unsigned long startTime = micros();
    while (digitalRead(pin) == HIGH) {
        if (micros() - startTime > DHT11_RESPONSE_TIMEOUT) {
            Serial.println("DHT11: No response (timeout 1)");
            return false;
        }
    }
    
    // Wait for DHT11 to pull up (response end)
    startTime = micros();
    while (digitalRead(pin) == LOW) {
        if (micros() - startTime > DHT11_RESPONSE_TIMEOUT) {
            Serial.println("DHT11: No response (timeout 2)");
            return false;
        }
    }
    
    // Wait for DHT11 to pull down (data start)
    startTime = micros();
    while (digitalRead(pin) == HIGH) {
        if (micros() - startTime > DHT11_RESPONSE_TIMEOUT) {
            Serial.println("DHT11: No data start (timeout 3)");
            return false;
        }
    }
    
    // Step 3: Read 40 bits (5 bytes)
    for (int i = 0; i < 40; i++) {
        // Wait for low to high transition (start of bit)
        startTime = micros();
        while (digitalRead(pin) == LOW) {
            if (micros() - startTime > DHT11_DATA_TIMEOUT) {
                Serial.printf("DHT11: Bit %d timeout (low)\n", i);
                return false;
            }
        }
        
        // Measure high pulse width
        startTime = micros();
        while (digitalRead(pin) == HIGH) {
            if (micros() - startTime > DHT11_DATA_TIMEOUT) {
                Serial.printf("DHT11: Bit %d timeout (high)\n", i);
                return false;
            }
        }
        
        unsigned long pulseWidth = micros() - startTime;
        
        // Determine bit value based on pulse width
        // 26-28μs = 0, 70μs = 1
        if (pulseWidth > 50) {  // Threshold between 0 and 1
            data[byteIndex] |= (1 << (7 - bitIndex));
        }
        
        bitIndex++;
        if (bitIndex == 8) {
            bitIndex = 0;
            byteIndex++;
        }
    }
    
    // Step 4: Verify checksum
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        Serial.printf("DHT11: Checksum error! Calculated: %d, Received: %d\n", checksum, data[4]);
        return false;
    }
    
    // Step 5: Convert to temperature and humidity
    *humidity = (float)data[0] + (float)data[1] / 100.0;
    *temperature = (float)data[2] + (float)data[3] / 100.0;
    
    // Validate ranges
    if (*humidity < 0 || *humidity > 100 || *temperature < -40 || *temperature > 80) {
        Serial.printf("DHT11: Invalid values! Temp: %.1f, Humidity: %.1f\n", *temperature, *humidity);
        return false;
    }
    
    return true;
} 