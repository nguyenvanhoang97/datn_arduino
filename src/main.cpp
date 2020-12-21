#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ArduinoJson.h>

WebSocketsClient webSocket;

const int SOIL = A0;
const int DHTTYPE = DHT11;
const int DHTPIN = 4;
const char *ssid = "VietTel";
const char *password = "66668888";
const char *ip_host = "192.168.1.247";
const uint16_t port = 3000;
int checkAuto = 1;

DHT dht(DHTPIN, DHTTYPE);

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.printf("[WSc] Disconnected!\n");
    break;
  case WStype_CONNECTED:
  {
    Serial.printf("[WSc] Connected to url: %s\n", payload);
  }
  break;
  case WStype_TEXT:
    Serial.printf("[WSc] get text: %s\n", payload);
    if (strcmp((char *)payload, "MOTOR_ON") == 0)
    {
      digitalWrite(LED_BUILTIN, LOW);
    }
    else if (strcmp((char *)payload, "MOTOR_OFF") == 0)
    {
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else if (strcmp((char *)payload, "AUTO_ON") == 0)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      checkAuto = 0;
    }
    else if (strcmp((char *)payload, "AUTO_OFF") == 0)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      checkAuto = 1;
    }
    break;
  case WStype_BIN:
    Serial.printf("[WSc] get binary length: %u\n", length);
    break;
  }
}
void setup()
{
  // pinMode(MOTOR, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SOIL, INPUT);
  Serial.begin(9600);
  digitalWrite(LED_BUILTIN, HIGH);
  dht.begin();
  Serial.println("ESP8266 Websocket Client");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  webSocket.begin(ip_host, port);
  webSocket.onEvent(webSocketEvent);
}
void loop()
{
  webSocket.loop();
  
  String dataSend;
  float humi = dht.readHumidity();
  float temp = dht.readTemperature();
  float soil = analogRead(SOIL);

  int percent = map(soil, 0, 1023, 0, 100);

  StaticJsonDocument<256> data;
  data["temperature"] = temp;
  data["humidity"] = humi;
  data["soil_moisture"] = percent;
  
  serializeJson(data, dataSend);

  if (checkAuto == 0) {
    if (soil == 100) {
      digitalWrite(LED_BUILTIN, LOW);
    } else {
      digitalWrite(LED_BUILTIN, HIGH);
    }
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }

  webSocket.sendTXT(dataSend);

  delay(500);
}