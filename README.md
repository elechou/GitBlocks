## LeetCode Calendar for CoreInk

This project displays your daily LeetCode submission activity on an e-ink screen using the M5Stack CoreInk.

## Introduction

This project targets the [M5Stack CoreInk](https://docs.m5stack.com/en/core/coreink).
I enjoy tracking my progress — it’s motivating to see the work accumulate over time. That led me to build a small desktop dashboard to visualize my LeetCode activity. Since LeetCode submission data doesn't change frequently, updating a few times per day is enough. The low‑power, always‑visible nature of an e‑ink display makes it a great fit for passive tracking.

Current features include:
1. Automatically fetch recent submission records from LeetCode (CN site) — no account login required.
2. Smart scheduling: active during the day, sleeps at night (sleeps until 10:00).
3. Intelligent updates: refresh only when new submissions are detected.
4. Low power consumption via optimized Wi‑Fi usage.

### Screenshot

![Screenshot](./src/figure.jpg)

## Setup Instructions

### 1) Configuration

Before compiling, you need to create a configuration file:

1. Copy `src/config.h.example` to `src/config.h`.
2. Edit `src/config.h` with your settings:
   ```cpp
   #define WIFI_SSID "Your_WiFi_SSID"
   #define WIFI_PASSWORD "Your_WiFi_Password"
   #define USERNAME "your-leetcode-username"
   // Optional: timezone and NTP servers
   #define NTP_TIMEZONE "JST-9"
   #define NTP_SERVER1 "ntp.nict.jp"
   #define NTP_SERVER2 "ntp.aliyun.com"
   #define NTP_SERVER3 "ntp1.aliyun.com"
   ```

### 2) Build and Upload
Use PlatformIO to compile and upload the code to your M5Stack CoreInk.

### Libraries and References

 1. [m5stack/M5GFX](https://github.com/m5stack/M5GFX)
 2. [m5stack/M5Unified](https://github.com/m5stack/M5Unified)
 3. [m5stack/M5Core-Ink](https://github.com/m5stack/M5Core-Ink) (conflicts with M5Unified)
 4. [bblanchon/ArduinoJson](https://github.com/bblanchon/ArduinoJson)
 5. [如何在不登录的情况下获取LeetCode-CN用户提交记录](https://blog.csdn.net/qq_32424059/article/details/106071201)

## Installation and Usage

Use [PlatformIO](https://platformio.org/) for this project.

 1. Create a new project in PlatformIO.

 2. Add the following libraries:
    - m5stack/M5GFX@^0.2.6
    - m5stack/M5Unified@^0.2.5
    - bblanchon/ArduinoJson@^7.3.1

 3. Copy the code from [`src/main.cpp`](./src/main.cpp) into your project (this repo uses a single editable file) and edit `src/config.h`.

 4. Build and upload to your CoreInk.

## FAQ

Q:
> How do I configure my own settings?

A:
```cpp
// Edit src/config.h
#define WIFI_SSID "******"
#define WIFI_PASSWORD "******"
#define USERNAME "******" // Your LeetCode username
```
If necessary, handle any concerns around storing plaintext passwords yourself.

## Changelog

- 0.2.0 (Jul 15, 2025): Handled timeouts for LeetCode API requests.
- 0.1.0 (Apr 05, 2025): Initial release.

## License

MIT License.

## Contact

Questions or feedback are welcome: Shou Qiu <qiusots@gmail.com>.