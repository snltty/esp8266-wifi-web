#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>


/*保存信息*/
char ssid[32] = "";
char password[32] = "";
char serverip[32] = "";
char port[32] = "";

/*启动主入口*/
void setup() {
  Serial.begin(115200);
  Serial.println();
  setupWifi();
  setupWeb();
  setupForward();
}
/*循环主入口*/
void loop() {
  loopWifi();
  loopWeb();
  loopServer();
  loopForward();
}
