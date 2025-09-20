/*内网穿透*/
#include <vector>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Preferences.h>
#include <tuple>

//开启穿透标识
const char* forwardOpenFlag = "snltty_open";
//回复穿透标识
const char* forwardReplyFlag = "snltty_reply";

//分隔符
const char delimiterChar = '|';
const String delimiterString = "|";
//结束符
const char endChar = ',';

//连接服务器的隧道
WiFiClient tunnel;
//用户交换数据的状态缓存
struct ForwardSession 
{
  WiFiClient source;
  WiFiClient target;
};
//交换数据列表
std::vector<ForwardSession> forwardSessions;

//保存信息，和是否需要穿透
boolean needForward = false;
Preferences preferencesForward;
void setForward(String serverHost,int serverPort,int remotePort,String localHost,int localPort)
{
    preferencesForward.begin("forward", false);
    preferencesForward.putString("serverHost",serverHost);
    preferencesForward.putInt("serverPort",serverPort);
    preferencesForward.putInt("remotePort",remotePort);
    preferencesForward.putString("localHost",localHost);
    preferencesForward.putInt("localPort",localPort);
    preferencesForward.end();
    needForward = serverHost.length() > 0 && serverPort > 0 && remotePort > 0 && localHost.length()>0 && localPort > 0;
}
std::tuple<String, int, int, String,int> loadForward() 
{
    preferencesForward.begin("forward", false);
    String serverHost = preferencesForward.getString("serverHost","");
    int serverPort = preferencesForward.getInt("serverPort",0);
    int remotePort = preferencesForward.getInt("remotePort",0);
    String localHost = preferencesForward.getString("localHost","");
    int localPort = preferencesForward.getInt("localPort",0);
    preferencesForward.end();
    Serial.println("[FORWARD] cache loaded");
    Serial.printf("[FORWARD] serverHost:%s\n", serverHost);
    Serial.printf("[FORWARD] serverPort:%d\n", serverPort);
    Serial.printf("[FORWARD] remotePort:%d\n", remotePort);
    Serial.printf("[FORWARD] localHost:%s\n", localHost);
    Serial.printf("[FORWARD] localPort:%d\n", localPort);
    needForward = serverHost.length() > 0 && serverPort > 0 && remotePort > 0 && localHost.length()>0 && localPort > 0;

    return std::make_tuple(serverHost, serverPort, remotePort, localHost,localPort);
}


void forward(){
    if(needForward == false) return;
    //连接服务器
    if(!tunnel.connected() && WiFi.status() == WL_CONNECTED)
    {
        preferencesForward.begin("forward", false);
        String serverHost = preferencesForward.getString("serverHost","");
        int serverPort = preferencesForward.getInt("serverPort",0);
        int remotePort = preferencesForward.getInt("remotePort",0);
        String localHost = preferencesForward.getString("localHost","");
        int localPort = preferencesForward.getInt("localPort",0);
        preferencesForward.end();
        tunnel.connect(serverHost, serverPort);
        //开启一个穿透
        tunnel.print(forwardOpenFlag+delimiterString+remotePort+delimiterString+localHost+delimiterString+localPort);
    }
    else if(tunnel.available())
    {
        //从服务器接收穿透信息
        String serverIP = tunnel.remoteIP().toString();
        int serverPort = tunnel.readStringUntil(delimiterChar).toInt();             
        String serverId = tunnel.readStringUntil(delimiterChar);             
        String targetIP = tunnel.readStringUntil(delimiterChar);             
        int targetPort= tunnel.readStringUntil(endChar).toInt();  

        WiFiClient source;
        WiFiClient target;
        //连接服务器和本地服务
        if (source.connect(serverIP,serverPort) && target.connect(targetIP, targetPort))
        {
            //回复穿透
            source.print(forwardReplyFlag+delimiterString+serverId);
            forwardSessions.push_back({source, target});
        }
    }

    //处理所有连接
    for(auto& session : forwardSessions)
    {
        while (session.source.available())
        {
          size_t bytesAvailable = session.source.available();
          uint8_t buf[bytesAvailable];
          session.source.read(buf, bytesAvailable);
          session.target.write(buf, bytesAvailable);
        }
        while (session.target.available()) 
        {
          size_t bytesAvailable = session.target.available();
          uint8_t buf[bytesAvailable];
          session.target.read(buf, bytesAvailable);
          session.source.write(buf, bytesAvailable);
        }
    }
    //清理无效连接
    forwardSessions.erase(
    std::remove_if(forwardSessions.begin(), forwardSessions.end(), 
      [](ForwardSession& s) { return !s.source.connected() || !s.target.connected(); }),
    forwardSessions.end());
}

void setupForward()
{
  auto data = loadForward();
}
void loopForward()
{
    forward();
}
