#include <ESP8266WebServer.h>
/*网页服务，用于配网*/
ESP8266WebServer server(80);
/*首页*/
void handleHome() {
  server.sendHeader("Cache-Control", "no-cache");

  String wifiHtml = "<select name='n' class='w-200 h-32'>";
  int n = WiFi.scanNetworks();
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      wifiHtml += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + "</option>";
    }
  }
  wifiHtml += "</select>";

  String html = "<!DOCTYPE html><html>";
  html += "<head><meta charset='utf-8'/><meta name='viewport' content='width=device-width,initial-scale=1.0'/><title>snltty esp8266 wifi config</title>";
  html += "   <meta charset='utf-8'>";
  html += "   <meta name='viewport' content='width=device-width,initial-scale=1.0'>";
  html += "   <title>snltty esp8266 wifi config</title>";
  html += "   <style>*{margin:0;padding:0;list-style: none;text-align:center;} body{font-size:12px;}.mt-40{margin-top:40px;} .mt-20{margin-top:20px;} .mt-10{margin-top:10px;} .w-200{width:180px;} .h-32{height:32px; line-height:32px;} .label{display:inline-block; width:60px;line-height:32px; text-align:left;} .submit{padding:10px 20px;}</style>";
  html += "</head>";
  html += "<body>";
  html += "   <h1 class='mt-40'>snltty esp8266 wifi config</h1>";
  html += "   <form method='POST' action='save' class='mt-20'>";
  html += "     <div><span class='label'>wifi:</span>" + wifiHtml + "</div>";
  html += "     <div class='mt-10'><span class='label'>passd:</span><input type='text' placeholder='password' name='p' class='w-200 h-32'/></div>";
  html += "     <div class='mt-10'><span class='label'>server:</span><input type='text' placeholder='server ip' name='s' class='w-200 h-32'/></div>";
  html += "     <div class='mt-10'><span class='label'>port:</span><input type='text' placeholder='server port' name='o' class='w-200 h-32'/></div>";
  html += "     <div class='mt-10'><input type='submit' value='Connect' class='submit'/></div>";
  html += "   </form>";
  html += "</body>";
  html += "</html>";

  server.send(200, "text/html; charset=utf-8", html);
  server.client().stop();
}
/*接收提交，保存数据*/
void handleWifiSave() {
  Serial.println("[WIFI] save to cache");
  server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
  server.arg("p").toCharArray(password, sizeof(password) - 1);
  server.arg("s").toCharArray(serverip, sizeof(serverip) - 1);
  server.arg("o").toCharArray(port, sizeof(port) - 1);
  server.sendHeader("Location", "/", true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(302, "text/plain", "");
  server.client().stop();
  saveWifi();
  loadWifi();
}
/*默认404*/
void handleNotFound() {
  server.send(404);
}
/*启动网页*/
void setupWeb() {
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
void loopWeb() {
  server.handleClient();
}
