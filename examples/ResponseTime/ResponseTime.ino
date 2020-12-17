#include <IntlerCloudWifi.h>

int luminosity = 32, temperature = 64;
Cloud cloud("user", "pass", "myArduino");

void printTemperature(double val) {
  // печатаем полученное значение
  Serial.print("Print temperature " + String(val));
  
  // печатаем время ответа. можно сделать и с помощью 
  // cloud.printLastResponseTime();
  Serial.print(", time: ");
  Serial.print(cloud.getHour());
  Serial.print(" : ");
  Serial.print(cloud.getMinute());
  Serial.print(" : ");
  Serial.println(cloud.getSecond());
}

void printLight(double val) {
  Serial.println("Print light " + String(val));
}

void setup() {
  Serial.begin(115200);
 
  cloud.connect("ssid", "password");  
 
  cloud.addCommand("temperature", printTemperature);
  cloud.addCommand("light", printLight);

  cloud.setInterval(5000); // не обязательно. значение по умолчанию: 10000
}

// меняем значения переменных местами
void swap(int *a, int *b) {
  int t = (*a);
  (*a) = (*b);
  (*b) = t;
}

void loop()
{
  cloud.run();
  cloud.sendValue("temperature", temperature);
  cloud.sendValue("light", luminosity);

  if(cloud.isAnswerRecived()) {
   swap(&luminosity, &temperature); // меняем значения переменных местами

   cloud.printLastResponseTime(); // печатаем в порт время ответа
  }
}