#include <Arduino.h>

/*启动主入口*/
void setup()
{
  Serial.begin(115200);
  Serial.println();
  setupWifi();
  setupWeb();
  setupForward();
}
/*循环主入口*/
void loop()
{
  loopWifi();
  loopWeb();
  loopForward();
}
