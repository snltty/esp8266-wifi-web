#include <Arduino.h>
#include <Arduino_JSON.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <EEPROM.h>

/*热点ip*/
IPAddress apIP(192, 168, 18, 1);
IPAddress apNetMask(255, 255, 255, 0);
/*热点名*/
const char apSSID[] = "snltty8266";
/*热点密码*/
const char apPassword[] = "snltty8266";
/*暂时无用*/
const char myHostname[] = "snltty";
/*保存信息*/
char ssid[32] = "";
char password[32] = "";
char serverip[32] = "";
char port[32] = "";
/*是否需要连接wifi，当网页上提交数据后，就可以去尝试连接*/
boolean needConnect = false;


/*启动主入口*/
void setup() {
  Serial.begin(115200);
  Serial.println();
  setupWifi();
  setupWeb();
}
/*循环主入口*/
void loop() {
  loopWifi();
  loopWeb();
}
