## **Leetcode Calendar for CoreInk**

This project uses the M5Stack CoreInk to show daily LeetCode submission activity on an e-ink screen.

## **Introduction**

This project is designed for the [M5Stack CoreInk](https://docs.m5stack.com/en/core/coreink).
I enjoy tracking my working progress — it's motivating to see the work accumulate over time. That lead me to build a dashboard on my desk to check my LeetCode progress. Since LeetCode submission data doesn't change frequently, updating once a day is more than enough. The low-power, always-visible nature of an e-ink display makes it a great fit for this kind of passive tracking.

The current features includes:
1. Automatically fetching recent submission records from LeetCode CN (No account login required).
2. Smart scheduling: Updates every hour during work hours (10:00-22:00), sleeps during night hours (22:00-10:00).
3. Intelligent screen updates: Only refreshes when new submissions are detected.
4. Low power consumption with optimized WiFi usage.

## **Setup Instructions**

### **1. Configuration**

Before compiling, you need to create a configuration file:

1. Copy `src/config.h.example` to `src/config.h`
2. Edit `src/config.h` with your actual settings:
   ```cpp
   #define WIFI_SSID "Your_WiFi_SSID"
   #define WIFI_PASSWORD "Your_WiFi_Password"
   #define USERNAME "your-leetcode-username"
   ```

### **2. Compilation**

Use PlatformIO to compile and upload the code to your M5Stack CoreInk.

### **Screenshot:**

![./src/figure.jpg](./src/figure.jpg "screenshot1")lendar for CoreInk**

This project uses the M5Stack CoreInk to show daily LeetCode submission activity on an e-ink screen.

## **Introduction**

This project is designed for the [M5Stack CoreInk](https://docs.m5stack.com/en/core/coreink).
I enjoy tracking my working progress — it’s motivating to see the work accumulate over time. That lead me to build a dashboard on my desk to check my LeetCode progress. Since LeetCode submission data doesn’t change frequently, updating once a day is more than enough. The low-power, always-visible nature of an e-ink display makes it a great fit for this kind of passive tracking.

The current features includes:
1. Automatically fetching recent submission records from LeetCode CN (No account login required).
2. Update every 4 hours. (The maximum sleep interval allowed by CoreInk's API).

### **Screenshot:**

![./src/figure.jpg](./src/figure.jpg "screenshot1")

### **Libraries and Recommended Reads:**

1. [m5stack/M5GFX](https://github.com/m5stack/M5GFX)
2. [m5stack/M5Unified](https://github.com/m5stack/M5Unified)
3. [m5stack/M5Core-Ink](https://github.com/m5stack/M5Core-Ink)(Conflicts with M5Unified.)
4. [bblanchon/ArduinoJson](https://github.com/bblanchon/ArduinoJson)
5. [如何在不登录的情况下获取LeetCode-CN用户提交记录](https://blog.csdn.net/qq_32424059/article/details/106071201)

## **Installation and Usage**

Recommend to use [PlatformIO](https://platformio.org/) for this project.

1. Create a new project in PlatformIO.

2. Add the following libraries:
    1. m5stack/M5GFX@^0.2.6
	2. m5stack/M5Unified@^0.2.5
	3. bblanchon/ArduinoJson@^7.3.1

3. Copy all the codes in [main.cpp](./src/main.cpp) to yours.(This project only has one file to edit.)

4. Build and upload to your CoreInk.

## **FAQ**

**Q:**
> How to configure my own infos?

**A:**
> You can change the configures at the beginning of the main.cpp.  
> #define WIFI_SSID "\*\*\*\*\*\*"  
> #define WIFI_PASSWORD "\*\*\*\*\*\*"  
> #define USERNAME "\*\*\*\*\*\*"  // Your Leetcode Username.  
> If necessary, you will need to handle the issue of **plaintext passwords** yourself.

## **Change log**

- **0.1.0(Arp.5 2024):** Initial release

## **License**

This is released under the MIT License.

## **Contact**

If you have any questions or comments about this project, please contact Shou Qiu (<qiusots@gmail.com>).