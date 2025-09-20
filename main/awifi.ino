/*配置和连接wifi*/

#include <ESP8266WiFi.h>
#include <Preferences.h>
#include <tuple>

/*热点ip*/
const IPAddress apIP(192, 168, 18, 1);
const IPAddress apNetMask(255, 255, 255, 0);
/*热点名*/
const char apSSID[] = "snltty8266";
/*热点密码*/
const char apPassword[] = "snltty8266";
/*暂时无用*/
const char myHostname[] = "snltty";

/*是否需要连接wifi，当网页上提交数据后，就可以去尝试连接*/
boolean needWifi = false;
Preferences preferences;

void setWifi(String ssid,String password)
{
    preferences.begin("wifi", false);
    preferences.putString("ssid",ssid);
    preferences.putString("password",password);
    preferences.end();
    needWifi = ssid.length() > 0;
    WiFi.disconnect();
}
std::tuple<String, String> loadWifi() 
{
    preferences.begin("wifi", false);
    String ssid =  preferences.getString("ssid","");
    String password =  preferences.getString("password","");
    preferences.end();
    Serial.println("[WIFI] cache loaded");
    Serial.printf("[WIFI] name:%s\n", ssid);
    Serial.printf("[WIFI] password:%s\n", password.length() > 0 ? "********" : "<no password>");
    needWifi = ssid.length() > 0;

    return std::make_tuple(ssid, password);
}

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
void loopWifi() 
{
    if (needWifi && WiFi.status() != WL_CONNECTED) {
        preferences.begin("wifi", false);
        String ssid =  preferences.getString("ssid","");
        String password =  preferences.getString("password","");
        preferences.end();

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
