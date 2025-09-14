/*配置和连接wifi*/

#include <ESP8266WiFi.h>
#include <EEPROM.h>
/*热点ip*/
const IPAddress apIP(192, 168, 18, 1);
const IPAddress apNetMask(255, 255, 255, 0);
/*热点名*/
const char apSSID[] = "snltty8266";
/*热点密码*/
const char apPassword[] = "snltty8266";
/*暂时无用*/
const char myHostname[] = "snltty";

/*保存信息*/
char ssid[32] = "";
char password[32] = "";

/*是否需要连接wifi，当网页上提交数据后，就可以去尝试连接*/
boolean needWifi = false;

//保存wifi信息
void setWifi(String _ssid,String _password)
{
    _ssid.toCharArray(ssid, sizeof(ssid) - 1);
    _password.toCharArray(password, sizeof(password) - 1);
    saveWifi();
    loadWifi();
}

/*从rom加载配置*/
void loadWifi() 
{
  int ssidPos = 0;
  int passwordPos = ssidPos + sizeof(ssid);
  int endPos = ssidPos + passwordPos + sizeof(passwordPos);

  EEPROM.begin(512);
  EEPROM.get(ssidPos, ssid);
  EEPROM.get(passwordPos, password);
  char ok[2 + 1];
  EEPROM.get(endPos, ok);
  EEPROM.end();
  if (String(ok) != String("OK"))
  {
    ssid[0] = 0;
    password[0] = 0;
  }
  Serial.println("[WIFI] cache loaded");
  Serial.printf("[WIFI] name:%s\n", ssid);
  Serial.printf("[WIFI] password:%s\n", strlen(password) > 0 ? "********" : "<no password>");
  needWifi = strlen(ssid) > 0;
}
/*保存wifi配置*/
void saveWifi()
{
  int ssidPos = 0;
  int passwordPos = ssidPos + sizeof(ssid);
  int endPos = ssidPos + passwordPos + sizeof(passwordPos);

  EEPROM.begin(512);
  EEPROM.put(ssidPos, ssid);
  EEPROM.put(passwordPos, password);
  char ok[2 + 1] = "OK";
  EEPROM.put(endPos, ok);
  EEPROM.commit();
  EEPROM.end();
}
/*启动wifi*/
void setupWifi() 
{
  Serial.println("[WIFI] starting");
  Serial.printf("[WIFI] name:<%s> password:<%s>\n", apSSID, apPassword);
  WiFi.softAPConfig(apIP, apIP, apNetMask);
  WiFi.softAP(apSSID, apPassword);
  WiFi.begin();
  Serial.printf("[WIFI] host ip:");
  Serial.println(WiFi.softAPIP());
  Serial.println("[WIFI] started");

  loadWifi();
}
/*wifi死循环*/
void loopWifi() 
{
  /*需要连接fiwi*/
  if (needWifi) {
    needWifi = false;
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    int result = WiFi.waitForConnectResult();
    Serial.printf("[WIFI] connect to <%s> with password <%s> result<%d>\n", ssid, password, result);
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("[WIFI] changed ");
      Serial.printf("[WIFI] ip:");
      Serial.println(WiFi.localIP());
    } 
    else 
    {
      Serial.printf("[WIFI] connect to <%s> fail\n", ssid);
    }
  }
}
