#include <IntlerCloudWifi.h>
void printTemperature(double val) {
  Serial.println("Print temperature " + String(val));
}

void printLight(double val) {
  Serial.println("Print light " + String(val));
}

int luminosity = 32, temperature = 64;
Cloud cloud("user", "pass", "myArduino");
void setup() {
  Serial.begin(115200);
 
  cloud.connect("ssid", "password");
  cloud.setCurrentTime();
  cloud.setInterval(8000);
 
  cloud.addCommand("temperature", printTemperature);
  cloud.addCommand("light", printLight);
}
void loop()
{
  cloud.run();
  cloud.sendValue("temperature", temperature);
  cloud.sendValue("light", luminosity);
}