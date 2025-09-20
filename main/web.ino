/*网页服务，用于配网*/

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <tuple>

ESP8266WebServer server(80);
/*首页*/
void handleHome()
{
    server.sendHeader("Cache-Control", "no-cache");

    auto [ssid,password] = loadWifi();
    auto [serverHost,serverPort,remotePort,localHost,localPort] = loadForward();

    String wifiHtml = "<select name='n' class='w-200 h-32'>";
    int n = WiFi.scanNetworks();
    if (n > 0)
    {
        for (int i = 0; i < n; i++)
        {
            wifiHtml += "<option value='" + WiFi.SSID(i) + "' "+(WiFi.SSID(i) == ssid ?"selected":"")+">" + WiFi.SSID(i) + "</option>";
        }
    }
    wifiHtml += "</select>";

    String html = "<!DOCTYPE html><html>";
    html += "<head><meta charset='utf-8'/><meta name='viewport' content='width=device-width,initial-scale=1.0'/><title>snltty esp8266 wifi config</title>";
    html += "   <meta charset='utf-8'>";
    html += "   <meta name='viewport' content='width=device-width,initial-scale=1.0'>";
    html += "   <title>snltty esp8266 wifi config</title>";
    html += "   <style>*{margin:0;padding:0;list-style: none;text-align:center;} body{font-size:12px;}.green{color:green;}.red{color:red;}.mt-40{margin-top:40px;} .mt-20{margin-top:20px;} .mt-10{margin-top:10px;} .w-200{width:180px;} .h-32{height:32px; line-height:32px;} .label{display:inline-block; width:100px;line-height:32px; text-align:left;} .submit{padding:10px 20px;}</style>";
    html += "</head>";
    html += "<body>";
    html += "   <h1 class='mt-40'>snltty esp8266 wifi config</h1>";
    if (WiFi.status() == WL_CONNECTED)
    {
        html += "   <h2 class='mt-20 green'>wifi ["+WiFi.localIP().toString()+"] online</h2>";
    } 
    else 
    {
        html += "   <h2 class='mt-20 red'>wifi offline</h2>";
    }
    html += "   <form method='POST' action='save' class='mt-20'>";
    html += "     <div><span class='label'>wifi:</span>" + wifiHtml + "</div>";
    html += "     <div class='mt-10'><span class='label'>password:</span><input type='text' value='"+password+"' placeholder='password' name='p' class='w-200 h-32'/></div>";
    html += "     <div class='mt-10'><span class='label'>server host:</span><input type='text' value='"+serverHost+"' placeholder='forward server host' name='s1' class='w-200 h-32'/></div>";
    html += "     <div class='mt-10'><span class='label'>server port:</span><input type='text' value='"+String(serverPort)+"' placeholder='forward server port' name='p1' class='w-200 h-32'/></div>";
    html += "     <div class='mt-10'><span class='label'>remote port:</span><input type='text' value='"+String(remotePort)+"' placeholder='forward remote port' name='p2' class='w-200 h-32'/></div>";
    html += "     <div class='mt-10'><span class='label'>local host:</span><input type='text' value='"+localHost+"' placeholder='forward local host' name='s3' class='w-200 h-32'/></div>";
    html += "     <div class='mt-10'><span class='label'>local port:</span><input type='text' value='"+String(localPort)+"' placeholder='forward local port' name='p3' class='w-200 h-32'/></div>";
    html += "     <div class='mt-10'><input type='submit' value='Connect' class='submit'/></div>";
    html += "   </form>";
    html += "</body>";
    html += "</html>";

    server.send(200, "text/html; charset=utf-8", html);
    server.client().stop();
}
/*接收提交，保存数据*/
void handleWifiSave()
{
    Serial.println("[WIFI] save to cache");

    setWifi(server.arg("n"),server.arg("p"));
    setForward(server.arg("s1"),server.arg("p1").toInt(),server.arg("p2").toInt(),server.arg("s3"),server.arg("p3").toInt());

    server.sendHeader("Location", "/", true);
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.send(302, "text/plain", "");
    server.client().stop();
}
/*默认404*/
void handleNotFound()
{
    server.send(404);
}
/*启动网页*/
void setupWeb()
{
    Serial.println("[WEB] starting");
    server.on("/", HTTP_GET, handleHome);
    server.on("/save", HTTP_POST, handleWifiSave);
    server.onNotFound(handleNotFound);
    server.enableCORS(true);
    server.enableETag(true);
    server.begin();
    Serial.println("[WEB] started");
}
/*网页死循环*/
void loopWeb()
{
    server.handleClient();
}
