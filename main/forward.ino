#include <vector>
//转发服务
WiFiServer forwardServer(1802);
struct ForwardClientSession {
  WiFiClient client;
  WiFiClient target;
};
std::vector<ForwardClientSession> forwardSessions;

const char* forwardTargetHost = "192.168.1.2";
const int forwardTargetPort = 80;

void setupForward(){
    forwardServer.begin();
}
void loopForward(){

    WiFiClient client = forwardServer.available();
    //有新连接
    if (client){
      WiFiClient target;
      if (target.connect(forwardTargetHost, forwardTargetPort)){
          forwardSessions.push_back({client, target});
      }
    }

    //处理所有连接
    for(auto& session : forwardSessions){
        while (session.client.available()) {
          size_t bytesAvailable = session.client.available();
          uint8_t buf[bytesAvailable];
          session.client.read(buf, bytesAvailable);
          session.target.write(buf, bytesAvailable);
        }
        while (session.target.available()) {
          size_t bytesAvailable = session.target.available();
          uint8_t buf[bytesAvailable];
          session.target.read(buf, bytesAvailable);
          session.client.write(buf, bytesAvailable);
        }
    }
    //清理无效连接
    forwardSessions.erase(
    std::remove_if(forwardSessions.begin(), forwardSessions.end(), 
      [](ForwardClientSession& s) { return !s.client.connected() || !s.target.connected(); }),
    forwardSessions.end());
}
