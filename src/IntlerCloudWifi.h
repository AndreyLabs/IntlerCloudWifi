#ifndef IntlerCloudWifi_h
#define IntlerCloudWifi_h

#include <WiFi.h>
#define DEFAULT_INTERVAL 10000
#define URL "/send-device-data"
#define INITURL "/init-data"
#define SERVER "kvm1.9038221382.mg7dm.vps.myjino.ru"

struct Command {
  String name;
  void (*procedure)(double);
  Command* next;
};

struct Order {
  String name;
  Order* next;
};

struct SensorValue {
  String name;
  double value;
  SensorValue *next;
};

class Cloud
{
  public:
    Cloud();
    Cloud(String login, String password, String device);
    
    void setLogin(String login);
    void setPassword(String password);
    void setDevice(String device);

    void connect(const char* ssid, const char* password);
    void run();
    void setInterval(int interval);
    
    void sendValue(String name, double value);
    void addCommand(String name, void (*orderFunction)(double));

    int getHour();
    int getMinute();
    int getSecond();
	
  private:
    String login;
    String password;
    String device;

    int hour;
    int minute;
    int second;

    Order* receivedOrders;
    Command* commands;
    SensorValue* sensorsList;

  	unsigned long beginMicros, endMicros;

    long requestTiming;
    int interval;

    void addSensorValue(SensorValue* value);
    void clearSensorsValues();

    bool waitResponce;
    WiFiClient client;
    void parseCurrentTime(String data);
    void sendRequest();
    void sendData(String url, String data);
    String getCloudInput();
    String getRequestBody();
    String getOrdersString();
    void parseHttpResponce(String responce);

    void executeOrder(String orderStr);
    void addNewOrder(String orderName);
    double stringToDouble(String doubleValue);
    void clearOrders();
};

#endif
