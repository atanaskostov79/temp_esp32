# DOOM ESP32 Weather & Clock Display

A multi-screen ESP32 project featuring a DOOM-themed weather and clock display with ST7789 screen.

## 🎮 Features

- **Multi-screen display** with automatic switching between weather and clock
- **DOOM-themed design** with orange text and white corner decorations
- **Real-time weather data** from Open-Meteo API for Plovdiv, Bulgaria
- **Accurate clock** with NTP synchronization
- **WiFi configuration** via Bluetooth terminal
- **Multi-threaded architecture** for smooth UI and network operations
- **Background image** support with custom DOOM-themed graphics

## 📊 Weather Information

- **Temperature** in Celsius
- **Wind speed** and direction (N, E, S, W)
- **Weather description** (Clear, Cloudy, Rain, Snow, Storm, etc.)
- **Day/Night indicator**
- **Last update time** in Bulgarian timezone
- **Auto-refresh** every 5 minutes

## 🕒 Clock Features

- **Real-time clock** with seconds
- **Date display**
- **NTP synchronization**
- **Smooth updates** without flickering
- **DOOM-style frame** with corner decorations

## 🛠️ Hardware Requirements

- **ESP32** development board
- **ST7789** 320x170 TFT display
- **WiFi** connection for weather data and NTP
- **Bluetooth** for WiFi configuration

## 🔧 Pin Configuration

```cpp
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST   4
```

## 📦 Dependencies

- **Adafruit GFX Library** - Graphics library
- **Adafruit ST7735 and ST7789 Library** - Display driver
- **ArduinoJson** - JSON parsing
- **WiFi** - Network connectivity
- **BluetoothSerial** - Bluetooth configuration
- **Preferences** - Credential storage
- **HTTPClient** - API requests
- **FreeRTOS** - Multi-threading

## 🚀 Setup

1. **Clone the repository**
2. **Install PlatformIO** or use Arduino IDE
3. **Connect hardware** according to pin configuration
4. **Upload firmware** to ESP32
5. **Configure WiFi** via Bluetooth terminal

## 📱 Bluetooth Configuration

Connect via Bluetooth terminal and send:
```
SSID:your_wifi_name
PASS:your_wifi_password
```

## 🎨 Design Features

- **DOOM orange color scheme** (`#FD20`)
- **White corner decorations** instead of full frame
- **Background image** support
- **Centered text** with proper spacing
- **Multi-line status messages**
- **Professional layout** for weather data

## 🔄 Multi-threading

- **Core 0**: Weather data fetching (every 5 minutes)
- **Core 1**: UI updates and screen management
- **Mutex protection** for thread-safe data access

## 📡 API Integration

Uses **Open-Meteo API** for weather data:
- **Location**: Plovdiv, Bulgaria (42.125°N, 24.75°E)
- **Data**: Current weather conditions
- **Update frequency**: Every 5 minutes
- **Timezone**: Automatic conversion to Bulgarian time

## 🎯 Future Enhancements

- Weather icons and animations
- Temperature trends and graphs
- UV index and humidity data
- Customizable themes
- More weather locations
- Advanced DOOM-style UI elements

## 📄 License

This project is open source and available under the MIT License.

## 🤝 Contributing

Feel free to contribute with:
- Bug fixes
- New features
- UI improvements
- Documentation updates

---

**Built with ❤️ and DOOM spirit!** 