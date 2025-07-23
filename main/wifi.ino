
/*从rom加载配置*/
void loadWifi() {
  int ssidPos = 0;
  int passwordPos = ssidPos + sizeof(ssid);
  int serverPos = ssidPos + passwordPos + sizeof(password);
  int portPos = ssidPos + passwordPos + serverPos + sizeof(serverip);
  int endPos = ssidPos + passwordPos + serverPos + portPos + sizeof(port);

  EEPROM.begin(512);
  EEPROM.get(ssidPos, ssid);
  EEPROM.get(passwordPos, password);
  EEPROM.get(serverPos, serverip);
  EEPROM.get(portPos, port);
  char ok[2 + 1];
  EEPROM.get(endPos, ok);
  EEPROM.end();
  if (String(ok) != String("OK")) {
    ssid[0] = 0;
    password[0] = 0;
    serverip[0] = 0;
    port[0] = 0;
  }
  Serial.println("[WIFI] cache loaded");
  Serial.printf("[WIFI] name:%s\n", ssid);
  Serial.printf("[WIFI] password:%s\n", strlen(password) > 0 ? "********" : "<no password>");
  Serial.printf("[WIFI] server:%s\n", serverip);
  Serial.printf("[WIFI] port:%s\n", port);
  needWifi = strlen(ssid) > 0;
  needServer = strlen(serverip) > 0 && strlen(port) > 0;
}
/*保存wifi配置*/
void saveWifi() {
  int ssidPos = 0;
  int passwordPos = ssidPos + sizeof(ssid);
  int serverPos = ssidPos + passwordPos + sizeof(password);
  int portPos = ssidPos + passwordPos + serverPos + sizeof(serverip);
  int endPos = ssidPos + passwordPos + serverPos + portPos + sizeof(port);

  EEPROM.begin(512);
  EEPROM.put(ssidPos, ssid);
  EEPROM.put(passwordPos, password);
  EEPROM.put(serverPos, serverip);
  EEPROM.put(portPos, port);
  char ok[2 + 1] = "OK";
  EEPROM.put(endPos, ok);
  EEPROM.commit();
  EEPROM.end();
}
/*启动wifi*/
void setupWifi() {
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
void loopWifi() {
  /*需要连接fiwi*/
  if (needWifi) {
    needWifi = false;
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    int result = WiFi.waitForConnectResult();
    Serial.printf("[WIFI] connect to <%s> with password <%s> result<%d>\n", ssid, password, result);
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("[WIFI] changed ");
      Serial.printf("[WIFI] ip:");
      Serial.println(WiFi.localIP());
    } else {
      Serial.printf("[WIFI] connect to <%s> fail\n", ssid);
    }
  }
  if(needServer && !client.connected() && WiFi.status() == WL_CONNECTED)
  {
      needServer = false;   
      String portStr(port);
      int portNum = portStr.toInt();
      if (!client.connect(serverip, portNum)) {
        Serial.printf("[SERVER] connect to <%s>:<%s> failed\n",serverip,port);
      }else{
        Serial.printf("[SERVER] connect to <%s>:<%s> success\n",serverip,port);  
        clientLastTime = millis();
        client.setTimeout(5000);
      }
      needServer = true;   
  }
}
