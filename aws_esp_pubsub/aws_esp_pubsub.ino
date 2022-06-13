#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#error Platform not supported
#endif
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson (use v6.xx)
#include <time.h>
#define emptyString String()
#include "DHT.h"

//Follow instructions from https://github.com/debsahu/ESP-MQTT-AWS-IoT-Core/blob/master/doc/README.md
//Enter values in secrets.h ▼
#include "secrets.h"

#if !(ARDUINOJSON_VERSION_MAJOR == 6 and ARDUINOJSON_VERSION_MINOR >= 7)
#error "Install ArduinoJson v6.7.0-beta or higher"
#endif

const int MQTT_PORT = 8883;
const char* MQTT_SUB_TOPIC =  "$aws/things/" THINGNAME "/shadow/update/delta";
const char* MQTT_SUB_TOPIC1 =  "$aws/things/" THINGNAME "/shadow/get/accepted";
const char* MQTT_PUB_TOPIC = "$aws/things/" THINGNAME "/shadow/update";

#ifdef USE_SUMMER_TIME_DST
uint8_t DST = 1;
#else
uint8_t DST = 0;
#endif

WiFiClientSecure net;

#ifdef ESP8266
BearSSL::X509List cert(cacert);
BearSSL::X509List client_crt(client_cert);
BearSSL::PrivateKey key(privkey);
#endif

PubSubClient client(net);

time_t now;
time_t nowish = 1510592825;
#define DHTTYPE DHT11   // DHT 11

const int DHTPin = 5;   //D1
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

const int red = D2;
const int green = D3;

void NTPConnect(void)
{
  Serial.print("Setting time using SNTP");
  configTime(TIME_ZONE * 3600, DST * 3600, "pool.ntp.org", "time.nist.gov");
  now = time(nullptr);
  while (now < nowish)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("done!");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void messageReceived(char *topic, byte *payload, unsigned int length)
{
  String msgIN = "";
  for (int i = 0; i < length; i++)
  {
    msgIN += (char)payload[i];
  }
  String msgString = msgIN;
  Serial.println("Recieved [" + String(topic) + "]: " + msgString);

  StaticJsonDocument<200> doc;
  StaticJsonDocument<64> filter;

  DeserializationError error = deserializeJson(doc, msgString, DeserializationOption::Filter(filter));

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  if(String(topic) == "$aws/things/qwerty/shadow/update/delta"){
    String redstr = doc["state"]["red"];
    String greenstr = doc["state"]["green"];
    Serial.println("Red: "+redstr);
    Serial.println("Green: "+greenstr);
    if(redstr && greenstr){
      if (redstr == "1") {
        digitalWrite(red, HIGH);              
      } else {
        digitalWrite(red, LOW);
      }
      if (greenstr == "1") {
        digitalWrite(green, HIGH);              
      } else {
        digitalWrite(green, LOW);
      }
    }
  }
  if(String(topic) == "$aws/things/qwerty/shadow/get/accepted"){
      String redstr = doc["state"]["desired"]["red"];
      String greenstr = doc["state"]["desired"]["green"];
      Serial.println("Get Red: "+redstr);
      
      if (redstr == "1") {
        digitalWrite(red, HIGH);              
      } else {
        digitalWrite(red, LOW);
      }

      if (greenstr == "1") {
        digitalWrite(green, HIGH);              
      } else {
        digitalWrite(green, LOW);
      }
    String str = "{ \"state\": { \"reported\": { \"red\":"+redstr+", \"green\":"+greenstr+"} } }";
    client.publish("$aws/things/qwerty/shadow/update",str.c_str ());
  }
  Serial.println();
}

void pubSubErr(int8_t MQTTErr)
{
  if (MQTTErr == MQTT_CONNECTION_TIMEOUT)
    Serial.print("Connection tiemout");
  else if (MQTTErr == MQTT_CONNECTION_LOST)
    Serial.print("Connection lost");
  else if (MQTTErr == MQTT_CONNECT_FAILED)
    Serial.print("Connect failed");
  else if (MQTTErr == MQTT_DISCONNECTED)
    Serial.print("Disconnected");
  else if (MQTTErr == MQTT_CONNECTED)
    Serial.print("Connected");
  else if (MQTTErr == MQTT_CONNECT_BAD_PROTOCOL)
    Serial.print("Connect bad protocol");
  else if (MQTTErr == MQTT_CONNECT_BAD_CLIENT_ID)
    Serial.print("Connect bad Client-ID");
  else if (MQTTErr == MQTT_CONNECT_UNAVAILABLE)
    Serial.print("Connect unavailable");
  else if (MQTTErr == MQTT_CONNECT_BAD_CREDENTIALS)
    Serial.print("Connect bad credentials");
  else if (MQTTErr == MQTT_CONNECT_UNAUTHORIZED)
    Serial.print("Connect unauthorized");
}

void connectToMqtt(bool nonBlocking = false)
{
  Serial.print("MQTT connecting ");
  while (!client.connected())
  {
    if (client.connect(THINGNAME))
    {
      Serial.println("connected!");
      if (!client.subscribe(MQTT_SUB_TOPIC))
        pubSubErr(client.state());
     if (!client.subscribe(MQTT_SUB_TOPIC1))
        pubSubErr(client.state());
           
      client.publish("$aws/things/qwerty/shadow/get","");  //For getting last saved state
    }
    else
    {
      Serial.print("failed, reason -> ");
      pubSubErr(client.state());
      if (!nonBlocking)
      {
        Serial.println(" < try again in 5 seconds");
        delay(5000);
      }
      else
      {
        Serial.println(" <");
      }
    }
    if (nonBlocking)
      break;
  }
}

void connectToWiFi(String init_str)
{
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void checkWiFiThenMQTT(void)
{
  connectToWiFi("Checking WiFi");
  connectToMqtt();
}

void setup()
{
  Serial.begin(115200);

  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  dht.begin();

  delay(5000);
  Serial.println();
  
  WiFi.begin(ssid, pass);
  connectToWiFi(String("Attempting to connect to SSID: ") + String(ssid));

  NTPConnect();

#ifdef ESP32
  net.setCACert(cacert);
  net.setCertificate(client_cert);
  net.setPrivateKey(privkey);
#else
  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);
#endif

  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(messageReceived);

  connectToMqtt();
}

void loop()
{
  now = time(nullptr);
  if (!client.connected())
  {
    checkWiFiThenMQTT();
  }
  else
  {
    client.loop();
    float h = dht.readHumidity();
    float temp = dht.readTemperature();
    if (isnan(h) || isnan(temp)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    Serial.println("Temp: " + String(temp) + " , Humidity: " + String(h));
    String str= "{\"temp\" : "+String(temp)+" , \"humidity\": "+String(h)+"}";
    //client.publish(MQTT_PUB_TOPIC, str.c_str ());
    delay(5000);
  }
}
