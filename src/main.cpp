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
#include <WiFi.h>
#include <esp_sntp.h>
#include <ctime>

#define WIFI_SSID "Chiba-lab-609-2G"
#define WIFI_PASSWORD \
  "bearingless__"  // 虽然是明文不过应该没人跑过来蹭WIFI吧。。
#define USERNAME "shou-66"
#define NTP_TIMEZONE "JST-9"
#define NTP_SERVER1 "ntp.nict.jp"
#define NTP_SERVER2 "ntp.aliyun.com"
#define NTP_SERVER3 "ntp1.aliyun.com"

const char* monthAbbreviations[] = {
  "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
  "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};

// 获取LeetCode提交数据
String fetch_leetcode_submissions(String username) {
  // 初始化HTTP客户端
  HTTPClient http;

  // 设置请求URL
  http.begin("https://leetcode.cn/graphql/noj-go/");

  // 设置请求头
  http.addHeader("authority", "leetcode.cn");
  http.addHeader("content-type", "application/json");
  http.addHeader("origin", "https://leetcode.cn");
  http.addHeader("referer", "https://leetcode.cn/u/" + username + "/");
  http.addHeader("user-agent",
                 "Mozilla/5.0 (Windows NT 10.0; Win64; x64) Chrome/134.0.0.0 ");

  // 构建GraphQL查询
  JsonDocument queryDoc; // 调整大小以适应实际需求

  // 构建查询对象
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
  if (httpResponseCode > 0) {
    payload = http.getString();
  } else {
    payload = "Error";
  }

  http.end();

  return payload;
}

void setup(void) {
  auto cfg = M5.config();

  cfg.external_rtc = true;
  cfg.clear_display = false;

  M5.begin(cfg);

  if (!M5.Rtc.getIRQstatus()) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    configTzTime(NTP_TIMEZONE, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }

    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED) {
      delay(500);
    }

    time_t t = time(nullptr) + 1;
    while (t > time(nullptr));

    M5.Rtc.setDateTime(localtime(&t));
  }

  // 获取当前日期
  auto date = M5.Rtc.getDate();

  // 获取LeetCode提交数据
  JsonDocument doc;
  String response = fetch_leetcode_submissions(USERNAME);
  // 测试用数据
  // String response = "{\"data\":{\"userCalendar\":{\"streak\":6,\"totalActiveDays\":40,\"submissionCalendar\":\'{\"1728950400\":4,\"1729036800\":5,\"1729296000\":1,\"1729382400\":4,\"1729468800\":1,\"1729555200\":1,\"1729728000\":3,\"1729814400\":13,\"1729900800\":2,\"1729987200\":2,\"1730073600\":6,\"1730160000\":2,\"1730332800\":4,\"1730764800\":4,\"1731628800\":1,\"1731715200\":2,\"1731974400\":5,\"1732060800\":3,\"1733270400\":7,\"1739664000\":4,\"1739750400\":3,\"1740355200\":3,\"1740441600\":3,\"1740528000\":1,\"1740700800\":2,\"1740787200\":2,\"1740873600\":3,\"1740960000\":4,\"1741132800\":1,\"1741219200\":3,\"1741305600\":3,\"1741392000\":1,\"1741651200\":2,\"1741737600\":2,\"1741824000\":1,\"1741910400\":2,\"1742083200\":2,\"1742169600\":2,\"1742256000\":5,\"1742428800\":13}\'}}}";
  deserializeJson(doc, response);

  if (doc["data"]["userCalendar"]["submissionCalendar"].isNull()) {
    delay(1000);
    response = fetch_leetcode_submissions(USERNAME);
    deserializeJson(doc, response);
  }

  // int streak = doc["data"]["userCalendar"]["streak"];
  // int totalDays = doc["data"]["userCalendar"]["totalActiveDays"];
  JsonDocument submissionCalendar;
  deserializeJson(submissionCalendar, doc["data"]["userCalendar"]["submissionCalendar"]);;

  // Create a Calendar Matrix.
  int weekday = date.weekDay;
  int days_last_thisweek = 6 - weekday % 7;

  // 构造当天的起始时间（00:00:00）
  struct tm timeinfo = {0};
  timeinfo.tm_year = date.year - 1900;                // 年份从 1900 开始计算
  timeinfo.tm_mon = date.month - 1;                   // 月份从 0 开始计算
  timeinfo.tm_mday = date.date + days_last_thisweek;  // 日
  timeinfo.tm_hour = 9;                               // 小时, (GMT+0900, Tokyo)
  timeinfo.tm_min = 0;                                // 分钟
  timeinfo.tm_sec = 0;                                // 秒

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
      uint32_t time_int = base_time_int - 86400*(i*7 + j);
      sprintf(time_string, "%ld", time_int); // 使用 %ld 格式化时间戳
      if(submissionCalendar[time_string].is<int>()){
        submissionMatrix[i][j] = submissionCalendar[time_string];
      }

      int block_color = max(0, 230 - 100*submissionMatrix[i][j]);
      canvas.fillRect( 175 - 25*i, 165 - 25*j, 20, 20, lgfx::color888(block_color, block_color, block_color));
    }
  }

  canvas.fillRect(177, 187 - 25 * days_last_thisweek, 16, 2, lgfx::color888(0, 0, 0));

  // // 如果需要一些文字。
  // char buf[32] = {0};
  // canvas.setFont(&DejaVu24);
  // canvas.setTextSize(1);
  // canvas.setTextColor(0x0000, 0xFFFF); // 前景白，背景黑
  // sprintf(buf, "%d", time_string);
  // canvas.drawCenterString(buf, 100, 10);

  canvas.setFont(&FreeSansBold12pt7b);
  canvas.setTextSize(1);
  canvas.setTextColor(0x0000, 0xFFFF); // 前景白，背景黑
  const char* month_str = monthAbbreviations[date.month - 1];
  canvas.drawCenterString(month_str, 35, 15);

  // Show date.
  if (date.date/10) {
    canvas.setFont(&FreeSansBold18pt7b);
  } else {
    canvas.setFont(&FreeSansBold24pt7b);
  }
  char buf[32] = {0};
  sprintf(buf, "%d", date.date);
  canvas.drawCenterString(buf, 35, 40);

  // Test Leetcode response;
  // canvas.setFont(&FreeSansBold9pt7b);
  // String testOuput;
  // serializeJson(doc["data"]["userCalendar"], testOuput);
  // canvas.drawString(testOuput, 0, 40);

  canvas.pushSprite(0, 0);
  canvas.deleteSprite();

  M5.Display.sleep();
  M5.Rtc.clearIRQ();
  M5.Rtc.setAlarmIRQ(14400); // 4h*60min*60s
  M5.Power.powerOff();
}

void loop(void) { delay(100); }