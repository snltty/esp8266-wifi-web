/*内网穿透*/
#include <vector>

//服务器
const char* forwardServerHost = "192.168.1.2";
const int forwardServerPort = 12346;

//穿透端口
const int forwardRemotePort = 8888;
//本地服务
const char* forwardLocalHost = "192.168.1.2";
const int forwardLocalPort = 80;

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
struct ForwardSession {
  WiFiClient source;
  WiFiClient target;
};
//交换数据列表
std::vector<ForwardSession> forwardSessions;

void forward(){
    //连接服务器
    if(!tunnel.connected() && WiFi.status() == WL_CONNECTED)
    {
        tunnel.connect(forwardServerHost, forwardServerPort);
        //开启一个穿透
        tunnel.print(forwardOpenFlag+delimiterString+forwardRemotePort+delimiterString+forwardLocalHost+delimiterString+forwardLocalPort);
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
        if (source.connect(serverIP,serverPort) && target.connect(targetIP, targetPort)){
            //回复穿透
            source.print(forwardReplyFlag+delimiterString+serverId);
            forwardSessions.push_back({source, target});
        }
    }

    //处理所有连接
    for(auto& session : forwardSessions){
        while (session.source.available()) {
          size_t bytesAvailable = session.source.available();
          uint8_t buf[bytesAvailable];
          session.source.read(buf, bytesAvailable);
          session.target.write(buf, bytesAvailable);
        }
        while (session.target.available()) {
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

void setupForward(){
}
void loopForward(){
    forward();
}
