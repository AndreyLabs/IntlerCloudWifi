#include "IntlerCloudWifi.h"

Cloud::Cloud()
{
  login = password = device = "";
  
  interval = DEFAULT_INTERVAL;

  hour = -1;
  minute = -1;
  second = -1;
}

Cloud::Cloud(String login, String password, String device)
{
  this->login = login;
  this->password = password;
  this->device = device;

  interval = DEFAULT_INTERVAL;
  receivedOrders = NULL;

  hour = -1;
  minute = -1;
  second = -1;
}

void Cloud::setLogin(String login) {
  this->login = login;
}

void Cloud::setPassword(String password) {
  this->password = password;
}

void Cloud::setDevice(String device) {
  this->device = device;
}

int Cloud::getHour() {
  return hour;
}
int Cloud::getMinute() {
  return minute;
}
int Cloud::getSecond() {
  return second;
}

void Cloud::parseCurrentTime(String data) {
  if(data == "")
    return;

  String s = data.substring(data.length()-2, data.length());
  String m = data.substring(data.length()-5, data.length()-3);
  String h = data.substring(data.length()-8, data.length()-6);
  
  hour = h.toInt();
  minute = m.toInt();
  second = s.toInt();
}

void Cloud::sendValue(String name, double value) {

  if (sensorsList == NULL) {
    SensorValue* newSensorValue = new SensorValue;
    newSensorValue->name = name;
    newSensorValue->value = value;
    newSensorValue->next = NULL;

    sensorsList = newSensorValue; 
    return;
  }

  SensorValue* iter = sensorsList;
  while (iter->next != NULL) {
    if ((iter->name).equals(name)) {
      iter->value = value;
      return;
    }
    iter = iter->next;
  }

  if (name.equals(iter->name))
    iter->value = value;
  else {
    SensorValue* newSensorValue = new SensorValue;
    newSensorValue->name = name;
    newSensorValue->value = value;
    newSensorValue->next = NULL;
    (iter->next) = newSensorValue;
  }
}

void Cloud::addSensorValue(SensorValue* value) {
  if (sensorsList == NULL) {
    sensorsList = value; 
    return;
  }

  SensorValue* iter = sensorsList;
  while (iter->next != NULL) {
    if ((iter->name).equals(value->name)) {
      iter->value = value->value;
      return;
    }
    iter = iter->next;
  }

  (iter->next) = value;
}

String Cloud::getRequestBody() {
  String requestBody = "{\"login\":\"" + login + "\",";
  requestBody += "\"password\":\"" + password + "\",";
  requestBody += "\"deviceName\":\"" + device + "\",";
  requestBody += "\"deviceType\":\"Arduino\",\"sensorsValue\":{";

  SensorValue* iter = sensorsList;
  if (iter == NULL) 
    requestBody += "}";
  else {
    while (iter->next != NULL) {
      requestBody += "\"" + String(iter->name) + "\":" + String(iter->value) + "," ;
      iter = iter->next;
    }
    requestBody += "\"" + String(iter->name) + "\":" + String(iter->value) + "}";
  }
  requestBody += ", \"ordersAccepted\":"+ getOrdersString() +"}";
  
  return requestBody;
}

void Cloud::sendData(String url, String data) {
  if (client.connect(SERVER, 80)) {
    client.print("POST ");
    client.print(url);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(SERVER);
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.print(data);
    client.println();
  }
  else {
    Serial.println("cannot connect to Intler cloud");
  }
}

String Cloud::getCloudInput() {
  String cloudInput = "";
  while (client.available()) {
    char c = client.read();
    cloudInput.concat(c);
  }

  return cloudInput;
}

void Cloud::sendRequest() {
  if (!waitResponce) {
    String data = getRequestBody();
    Serial.print("Sending to cloud: ");
    Serial.println(data);
    sendData(URL, data);
    
    clearOrders();
    clearSensorsValues();

    requestTiming = millis();
    waitResponce = true;
  }
  
  if (millis() - requestTiming > interval) {
    waitResponce = false;

    String cloudInput = getCloudInput();

    parseCurrentTime(cloudInput.substring(cloudInput.indexOf("Date"),cloudInput.indexOf("GMT") - 1));

    parseHttpResponce(cloudInput.substring(cloudInput.indexOf("{"),cloudInput.indexOf("}") + 1));
    //delete(bufClient);
  }
}

void Cloud::connect(const char* ssid, const char* password) {
    // delay(1000);

    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void Cloud::run() {
  sendRequest();
}

void Cloud::parseHttpResponce(String responce) {
  // String responce = responce.substring(responce.indexOf("{"),responce.indexOf("}") + 1); //TODO
  int bodyPos = responce.indexOf("{");

  String curOrderStr = responce.substring(1); 
  while(true) {
    String order;
    int commaPos = curOrderStr.indexOf(",");
    if (commaPos == -1) {
      order = curOrderStr.substring(0, curOrderStr.indexOf("}"));
      if (curOrderStr.indexOf("}") < 2) {
        //Serial.println("No command");
        return;
        }
      executeOrder(order);
      break;
    } else {
      order = curOrderStr.substring(0, curOrderStr.indexOf(","));
      executeOrder(order);
      curOrderStr = curOrderStr.substring(curOrderStr.indexOf(",") + 1);
    }
  }
}

void Cloud::executeOrder(String str) {
  int nameEndPos = str.substring(1).indexOf("\"");
  String orderName = str.substring(1, nameEndPos + 1);
  orderName.trim();
  String val = str.substring(str.indexOf(":") + 1);
  val.trim();
  
  addNewOrder(orderName);
  double doubleVal = stringToDouble(val);

  Command* iter = commands;
  if (iter == NULL)
    return;
  do {
    if (orderName.equals(iter->name))
      (iter->procedure)(doubleVal); 
    iter = iter->next;
  } while ((iter != NULL));
}

void Cloud::addNewOrder(String orderName) {
  Order* newOrder = new Order;
  newOrder->name = orderName;
  newOrder->next = NULL;
  
  Order* iter = receivedOrders;
  if (iter == NULL)
    receivedOrders = newOrder; 
  else {
    while (iter->next != NULL) 
      iter = iter->next;
  
    iter->next = newOrder;
  }
}

double Cloud::stringToDouble(String str) {
  char floatbufVar[32];
  str.toCharArray(floatbufVar,sizeof(floatbufVar));
  
  return atof(floatbufVar);
}

void Cloud::addCommand(String name, void (*procedure)(double)) {
  Command* newCommand = new Command;
  newCommand->name = name;
  newCommand->procedure = procedure;
  newCommand->next = NULL;

  Command* iter = commands;
  if (iter == NULL)
    commands = newCommand; 
  else {
    while (iter->next != NULL) 
      iter = iter->next;
  
    iter->next = newCommand;
  }
}

String Cloud::getOrdersString() {
  String result = "[";
  Order* iter = receivedOrders;
  if (iter != NULL) {
    result += "\"" + iter->name + "\"";
    while (iter->next != NULL) {
      iter = iter->next;
      result += ",\"" + iter->name + "\"";
    }
  }
  result.concat("]");
  
  return result;
}

void Cloud::clearOrders() {
  Order* iter = receivedOrders;
  Order* deletedOrder;
  while (iter != NULL) {
    deletedOrder = iter;
    iter = iter->next;
    delete(deletedOrder);
  };
  receivedOrders = NULL;
}

void Cloud::clearSensorsValues() {
  SensorValue* iter = sensorsList;
  SensorValue* deletedValue;
  while (iter != NULL) {
    deletedValue = iter;
    iter = iter->next;
    delete(deletedValue);
  };
  sensorsList = NULL;
}

void Cloud::setInterval(int interval) {
  if (interval < 5000)
    this->interval = 5000;
  else
    this->interval = interval;
}

bool Cloud::isAnswerRecived() {
  return !waitResponce;
}

void Cloud::printLastResponseTime() {
  Serial.println();
  Serial.print("Answer recived at: ");
  Serial.print(getHour());
  Serial.print(" : ");
  Serial.print(getMinute());
  Serial.print(" : ");
  Serial.print(getSecond());
  Serial.println();
}