/*
*******************************************************************************
* Describe: 用于显示Leetcode提交次数的CoreInk Dashboard.
* Date: 2025 Apr.5
* Author: Shou Qiu
*******************************************************************************
  Libraries:
  - [M5Unified](https://github.com/m5stack/M5Unified)
  - [M5GFX](https://github.com/m5stack/M5GFX)
*/

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <M5Unified.h>
#include <Preferences.h>
#include <WiFi.h>
#include <ctime>
#include <esp_sntp.h>
#include "config.h"

const char *monthAbbreviations[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                                    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

Preferences preferences;

// 更新UI界面的函数
void updateUI(JsonDocument& doc, m5::rtc_date_t& date) {
    JsonDocument submissionCalendar;
    deserializeJson(submissionCalendar,
                    doc["data"]["userCalendar"]["submissionCalendar"]);

    // Create a Calendar Matrix.
    int weekday = date.weekDay;
    int days_last_thisweek = 6 - weekday % 7;

    // 构造当天的起始时间（00:00:00）
    struct tm timeinfo = {0};
    timeinfo.tm_year = date.year - 1900; // 年份从 1900 开始计算
    timeinfo.tm_mon = date.month - 1;    // 月份从 0 开始计算
    timeinfo.tm_mday = date.date + days_last_thisweek; // 日
    timeinfo.tm_hour = 9; // 小时, (GMT+0900, Tokyo)
    timeinfo.tm_min = 0;  // 分钟
    timeinfo.tm_sec = 0;  // 秒

    // 转换为时间戳
    time_t timestamp = mktime(&timeinfo);
    uint32_t base_time_int = uint32_t(timestamp);

    char time_string[11];

    LGFX_Sprite canvas(&M5.Display);
    canvas.createSprite(200, 200);
    canvas.fillSprite(lgfx::color888(255, 255, 255));

    int submissionMatrix[5][7] = {0};
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 7; j++) {
            uint32_t time_int = base_time_int - 86400 * (i * 7 + j);
            sprintf(time_string, "%ld", time_int); // 使用 %ld 格式化时间戳
            if (submissionCalendar[time_string].is<int>()) {
                submissionMatrix[i][j] = submissionCalendar[time_string];
            }

            int block_color = max(0, 230 - submissionMatrix[i][j]*100);

            canvas.fillRect(
                175 - 25 * i, 165 - 25 * j, 20, 20,
                lgfx::color888(block_color, block_color, block_color));
        }
    }

    canvas.fillRect(177, 187 - 25 * days_last_thisweek, 16, 2,
                    lgfx::color888(0, 0, 0));

    canvas.setFont(&FreeSansBold12pt7b);
    canvas.setTextSize(1);
    canvas.setTextColor(0x0000, 0xFFFF); // 前景白，背景黑
    const char *month_str = monthAbbreviations[date.month - 1];
    canvas.drawCenterString(month_str, 35, 15);

    // Show date.
    if (date.date / 10) {
        canvas.setFont(&FreeSansBold18pt7b);
    } else {
        canvas.setFont(&FreeSansBold24pt7b);
    }
    char buf[32] = {0};
    sprintf(buf, "%d", date.date);
    canvas.drawCenterString(buf, 35, 40);

    canvas.pushSprite(0, 0);
    canvas.deleteSprite();
    
    // // 等待e-ink屏幕完成刷新
    // delay(1000);
    
    // // 尝试显式刷新显示
    // M5.Display.display();
    // delay(500);
}

// 获取LeetCode提交数据，带重试和超时机制
String fetch_leetcode_submissions(String username, int maxRetries = 3) {
    for (int attempt = 0; attempt < maxRetries; attempt++) {
        // 初始化HTTP客户端
        HTTPClient http;

        // 设置请求URL
        http.begin("https://leetcode.cn/graphql/noj-go/");
        
        // 设置超时时间（连接超时10秒，读取超时15秒）
        http.setTimeout(15000);
        http.setConnectTimeout(10000);

        // 设置请求头
        http.addHeader("authority", "leetcode.cn");
        http.addHeader("content-type", "application/json");
        http.addHeader("origin", "https://leetcode.cn");
        http.addHeader("referer", "https://leetcode.cn/u/" + username + "/");
        http.addHeader(
            "user-agent",
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64) Chrome/134.0.0.0 ");

        // 构建查询对象
        JsonDocument queryDoc;
        queryDoc["operationName"] = "userProfileCalendar";
        queryDoc["variables"]["userSlug"] = username;
        queryDoc["query"] = R"(query userProfileCalendar($userSlug: String!) {
          userCalendar(userSlug: $userSlug) {
              submissionCalendar    
              streak
              totalActiveDays
              activeYears
          }
      })";

        // 将JSON对象序列化为字符串
        String query;
        serializeJson(queryDoc, query);

        // 发送POST请求
        int httpResponseCode = http.POST(query);
        String payload = "";

        // 处理响应
        if (httpResponseCode == 200) {
            payload = http.getString();
            http.end();
            
            // 验证响应是否有效
            if (payload.length() > 50 && payload.indexOf("submissionCalendar") > 0) {
                return payload;
            }
        }
        
        http.end();
        
        // 如果不是最后一次尝试，等待一段时间再重试
        if (attempt < maxRetries - 1) {
            delay(2000 * (attempt + 1)); // 递增延迟：2s, 4s, 6s
        }
    }
    
    return "Error";
}

void setup(void) {
    auto cfg = M5.config();

    cfg.external_rtc = true;
    cfg.clear_display = false;

    M5.begin(cfg);

    // 初始化Preferences
    preferences.begin("leetcode", false);

    // 只有在第一次开机或断电重启时才同步时间
    if (!M5.Rtc.getIRQstatus()) {
        // 不是闹钟唤醒（第一次开机或断电重启）
        WiFi.setAutoReconnect(false);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        
        // 减少WiFi连接等待时间，避免长时间高功耗
        unsigned long wifiStartTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - wifiStartTime < 15000) {
            delay(5000);
        }
        
        // 如果WiFi连接失败，直接进入睡眠模式
        if (WiFi.status() != WL_CONNECTED) {
            M5.Display.sleep();
            M5.Rtc.clearIRQ();
            M5.Rtc.setAlarmIRQ(1800); // 30分钟后重试
            M5.Power.powerOff();
            return;
        }

        configTzTime(NTP_TIMEZONE, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);

        // 减少NTP同步等待时间
        unsigned long ntpStartTime = millis();
        while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED && millis() - ntpStartTime < 10000) {
            delay(5000);
        }

        time_t t = time(nullptr) + 1;
        while (t > time(nullptr))
            ;

        M5.Rtc.setDateTime(localtime(&t));
        
        // 断开WiFi连接以节省电力
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
    }

    // 获取当前日期和时间
    auto date = M5.Rtc.getDate();
    auto time = M5.Rtc.getTime();
    
    // 检查是否在夜间时段（24:00-10:00），如果是则延长睡眠时间
    if (time.hours >= 0 && time.hours < 10) {
        // 夜间时段，计算到早上10点的时间
        int hoursUntil10AM = 10 - time.hours;
        int secondsUntil10AM = hoursUntil10AM * 3600 - time.minutes * 60 - time.seconds;
        
        M5.Display.sleep();
        M5.Rtc.clearIRQ();
        M5.Rtc.setAlarmIRQ(secondsUntil10AM); // 睡眠到早上10点
        M5.Power.powerOff();
        return;
    }

    // 获取LeetCode提交数据
    JsonDocument doc;
    String response;
    bool dataFetchSuccess = false;
    
    // 连接WiFi获取数据
    WiFi.setAutoReconnect(false);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    unsigned long wifiStartTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - wifiStartTime < 15000) {
        delay(1000);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        response = fetch_leetcode_submissions(USERNAME);
        
        // 立即断开WiFi连接以节省电力
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        
        if (response != "Error") {
            deserializeJson(doc, response);
            
            // 验证数据有效性
            if (!doc["data"]["userCalendar"]["submissionCalendar"].isNull()) {
                dataFetchSuccess = true;
            } else {
                // 数据无效，尝试重新获取一次
                WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
                wifiStartTime = millis();
                while (WiFi.status() != WL_CONNECTED && millis() - wifiStartTime < 10000) {
                    delay(1000);
                }
                
                if (WiFi.status() == WL_CONNECTED) {
                    response = fetch_leetcode_submissions(USERNAME);
                    WiFi.disconnect(true);
                    WiFi.mode(WIFI_OFF);
                    
                    if (response != "Error") {
                        deserializeJson(doc, response);
                        if (!doc["data"]["userCalendar"]["submissionCalendar"].isNull()) {
                            dataFetchSuccess = true;
                        }
                    }
                }
            }
        }
    } else {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
    }
    
    // 如果网络请求失败，使用缓存数据或直接睡眠
    if (!dataFetchSuccess) {
        // 尝试使用上次缓存的数据
        String cachedData = preferences.getString("cachedData", "");
        if (cachedData.length() > 0) {
            deserializeJson(doc, cachedData);
            if (!doc["data"]["userCalendar"]["submissionCalendar"].isNull()) {
                dataFetchSuccess = true;
                // 使用缓存数据，但不更新时间戳，确保下次还会尝试获取新数据
            }
        }
        
        // if (!dataFetchSuccess) {
        //     // 完全无法获取数据，延长睡眠时间后重试
        //     M5.Display.sleep();
        //     M5.Rtc.clearIRQ();
        //     M5.Rtc.setAlarmIRQ(1800); // 30分钟后重试
        //     M5.Power.powerOff();
        //     return;
        // }
    } else {
        // 成功获取数据，缓存起来备用
        preferences.putString("cachedData", response);
    }
    
    
    // 获取之前保存的最后时间戳
    String lastTimestamp = preferences.getString("lastTimestamp", "");
    
    // 获取当前数据中的最后时间戳
    String currentTimestamp = "";
    JsonObject submissionCalendar = doc["data"]["userCalendar"]["submissionCalendar"];
    
    // 找到最大的时间戳
    for (JsonPair kv : submissionCalendar) {
        if (String(kv.key().c_str()) > currentTimestamp) {
            currentTimestamp = String(kv.key().c_str());
        }
    }
    
    // 比较时间戳判断是否需要更新
    bool shouldUpdate = false;
    if (lastTimestamp.length() == 0 || currentTimestamp > lastTimestamp) {
        shouldUpdate = true;
    }

    if (shouldUpdate) {
        // 数据有变化，更新UI
        updateUI(doc, date);
        
        // 保存新的最后时间戳
        preferences.putString("lastTimestamp", currentTimestamp);
        
        // 重置失败计数器
        preferences.putInt("failureCount", 0);
    }
    
    // 如果使用了缓存数据但没有新数据，记录失败次数
    if (!dataFetchSuccess || (dataFetchSuccess && preferences.getString("cachedData", "") == response)) {
        int failureCount = preferences.getInt("failureCount", 0);
        preferences.putInt("failureCount", failureCount + 1);
    }

    M5.Display.sleep();
    M5.Rtc.clearIRQ();
    
    // 检查当前时间，决定下次唤醒时间
    auto currentTime = M5.Rtc.getTime();
    int sleepDuration;
    
    // 获取失败计数，用于调整重试策略
    int failureCount = preferences.getInt("failureCount", 0);
    
    if (currentTime.hours >= 22) {
        // 晚上22点后，睡眠到第二天早上10点
        int hoursUntil8AM = (24 - currentTime.hours) + 8;
        sleepDuration = hoursUntil8AM * 3600 - currentTime.minutes * 60 - currentTime.seconds;
    } else {
        // 正常时间，4小时后唤醒
        sleepDuration = 4 * 3600;
    }
    
    M5.Rtc.setAlarmIRQ(sleepDuration);
    
    M5.Power.powerOff();
}

void loop(void) { delay(100); }