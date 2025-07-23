void loopServer(){
    if(client && client.connected())
    {
      if (!client.available()){
        if(millis() - clientLastTime > 15000){
          client.stop();
        }else if(millis() - clientLastTime > 5000)
        {
          client.write("ping");  
        }
        return;
      }

      //最后通信时间
      clientLastTime = millis();

      while(client.available())
      {
          String lenStr = client.readStringUntil(':');
          if(lenStr.length() == 0) {
            Serial.println("[SERVER] recv length fail");
            while(client.available()) client.read();
            return;
          }

          //等待有足够的数据
          int length = lenStr.toInt();
          unsigned long startTime = millis();
          while (client.available() < length && millis() - startTime < 5000) {
              delay(1);
          }

          //读取命令和内容
          if (client.available() >= length) {
            String cmd = client.readStringUntil(':');
            String content = client.readStringUntil('\n'); 
            procCmd(cmd,content);
          }else {
            Serial.println("[SERVER] recv fail");
            while(client.available()) client.read(); // 清空缓冲区
          }
      }
    }
}

void procCmd(String cmd,String content){
  
  if(cmd == "pong") return;

  Serial.printf("[SERVER] recv cmd <%s> content <%s>\n", cmd,content);
  if(cmd == "down") procCmdDown(content);
  else if(cmd == "up") procCmdUp(content);
}

//按下命令
void procCmdDown(String content){

}
//抬起命令
void procCmdUp(String content){

}
