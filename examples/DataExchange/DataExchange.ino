#include <IntlerCloudWifi.h>

int luminosity = 32, temperature = 64;
Cloud cloud("user", "pass", "myArduino");

void printTemperature(double val) {
  Serial.print("Print temperature " + String(val));
}

void printLight(double val) {
  Serial.println("Print light " + String(val));
}

void setup() {
  Serial.begin(115200);
 
  cloud.connect("ssid", "password");  
  cloud.setInterval(5000);
 
  cloud.addCommand("temperature", printTemperature);
  cloud.addCommand("light", printLight);
}
void loop()
{
  cloud.run();
  cloud.sendValue("temperature", temperature);
  cloud.sendValue("light", luminosity);

  if (cloud.isAnswerRecived()) {
    // Ваш код, выполняющийся когда данные от облака получены, 
    // а новые данные с платы еще не отправлены
  }
}